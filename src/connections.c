#include <R_ext/Connections.h>
#include <Rinternals.h>

#define R_EOF	-1
#include "gzio.h"


# define BUFSIZE 10000
int dummy_vfprintf(Rconnection con, const char *format, va_list ap)
{
    R_CheckStack2(BUFSIZE); // prudence
    char buf[BUFSIZE], *b = buf;
    int res;
    const void *vmax = NULL; /* -Wall*/
    int usedVasprintf = FALSE;
    va_list aq;

    va_copy(aq, ap);
    res = vsnprintf(buf, BUFSIZE, format, aq);
    va_end(aq);
#ifdef HAVE_VASPRINTF
    if(res >= BUFSIZE || res < 0) {
	res = vasprintf(&b, format, ap);
	if (res < 0) {
	    b = buf;
	    buf[BUFSIZE-1] = '\0';
	    warning(_("printing of extremely long output is truncated"));
	} else usedVasprintf = TRUE;
    }
#else
    if(res >= BUFSIZE) { /* res is the desired output length */
	vmax = vmaxget();
	/* apparently some implementations count short,
	   <http://unixpapa.com/incnote/stdio.html>
	   so add some margin here */
	b = R_alloc(res + 101, sizeof(char));
	vsnprintf(b, res + 100, format, ap);
    } else if(res < 0) { /* just a failure indication */
	vmax = vmaxget();
	b = R_alloc(10*BUFSIZE, sizeof(char));
	res = vsnprintf(b, 10*BUFSIZE, format, ap);
	if (res < 0) {
	    b[10*BUFSIZE - 1] = '\0';
	    warning(_("printing of extremely long output is truncated"));
	    res = 10*BUFSIZE;
	}
    }
#endif /* HAVE_VASPRINTF */
    if(con->outconv) { /* translate the buffer */
	char outbuf[BUFSIZE+1], *ob;
	const char *ib = b;
	size_t inb = res, onb, ires;
	Rboolean again = FALSE;
	size_t ninit = strlen(con->init_out);
	do {
	    onb = BUFSIZE; /* leave space for nul */
	    ob = outbuf;
	    if(ninit) {
		strcpy(ob, con->init_out);
		ob += ninit; onb -= ninit; ninit = 0;
	    }
	    errno = 0;
	    ires = Riconv(con->outconv, &ib, &inb, &ob, &onb);
	    again = (ires == (size_t)(-1) && errno == E2BIG);
	    if(ires == (size_t)(-1) && errno != E2BIG)
		/* is this safe? */
		warning(_("invalid char string in output conversion"));
	    *ob = '\0';
	    con->write(outbuf, 1, ob - outbuf, con);
	} while(again && inb > 0);  /* it seems some iconv signal -1 on
				       zero-length input */
    } else
	con->write(b, 1, res, con);
    if(vmax) vmaxset(vmax);
    if(usedVasprintf) free(b);
    return res;
}

int dummy_fgetc(Rconnection con)
{
    int c;
    Rboolean checkBOM = FALSE, checkBOM8 = FALSE;

    if(con->inconv) {
	if(con->navail <= 0) {
	    unsigned int i, inew = 0;
	    char *p, *ob;
	    const char *ib;
	    size_t inb, onb, res;

	    if(con->EOF_signalled) return R_EOF;
	    if(con->inavail == -2) {
		con->inavail = 0;
		checkBOM = TRUE;
	    }
	    if(con->inavail == -3) {
		con->inavail = 0;
		checkBOM8 = TRUE;
	    }
	    p = con->iconvbuff + con->inavail;
	    for(i = con->inavail; i < 25; i++) {
		if (con->buff)
		    c = buff_fgetc(con);
		else
		    c = con->fgetc_internal(con);
		if(c == R_EOF){ con->EOF_signalled = TRUE; break; }
		*p++ = (char) c;
		con->inavail++;
		inew++;
	    }
	    if(inew == 0) return R_EOF;
	    if(checkBOM && con->inavail >= 2 &&
	       ((int)con->iconvbuff[0] & 0xff) == 255 &&
	       ((int)con->iconvbuff[1] & 0xff) == 254) {
		con->inavail -= (short) 2;
		memmove(con->iconvbuff, con->iconvbuff+2, con->inavail);
	    }
	    if(inew == 0) return R_EOF;
	    if(checkBOM8 && con->inavail >= 3 &&
	       !memcmp(con->iconvbuff, "\xef\xbb\xbf", 3)) {
		con->inavail -= (short) 3;
		memmove(con->iconvbuff, con->iconvbuff+3, con->inavail);
	    }
	    ib = con->iconvbuff; inb = con->inavail;
	    ob = con->oconvbuff; onb = 50;
	    errno = 0;
	    res = Riconv(con->inconv, &ib, &inb, &ob, &onb);
	    con->inavail = (short) inb;
	    if(res == (size_t)-1) { /* an error condition */
		if(errno == EINVAL || errno == E2BIG) {
		    /* incomplete input char or no space in output buffer */
		    memmove(con->iconvbuff, ib, inb);
		} else {/*  EILSEQ invalid input */
		    warning(_("invalid input found on input connection '%s'"),
			    con->description);
		    con->inavail = 0;
		    con->EOF_signalled = TRUE;
		}
	    }
	    con->next = con->oconvbuff;
	    con->navail = (short)(50 - onb);
	}
	con->navail--;
	/* the cast prevents sign extension of 0xFF to -1 (R_EOF) */
	return (unsigned char)*con->next++;
    } else if (con->buff)
	return buff_fgetc(con);
    else
	return con->fgetc_internal(con);
}






/* needs to be declared before con_close1 */
typedef struct gzconn {
  Rconnection con;
  int cp; /* compression level */
z_stream s;
int z_err, z_eof;
uLong crc;
Byte buffer[Z_BUFSIZE];
int nsaved;
char saved[2];
Rboolean allow;
} *Rgzconn;


typedef struct gzfileconn {
  void *fp;
  int compress;
} *Rgzfileconn;

static Rboolean gzfile_open(Rconnection con)
{
  gzFile fp;
  char mode[6];
  Rgzfileconn gzcon = con->private;
  const char *name;

  strcpy(mode, con->mode);
  /* Must open as binary */
  if(strchr(con->mode, 'w')) snprintf(mode, 6, "wb%1d", gzcon->compress);
  else if (con->mode[0] == 'a') snprintf(mode, 6, "ab%1d", gzcon->compress);
  else strcpy(mode, "rb");
  errno = 0; /* precaution */
  name = R_ExpandFileName(con->description);
  /* We cannot use isDir, because we cannot get the fd from gzFile
   (it would be possible with gzdopen, if supported) */
  if (isDirPath(name)) {
    warning(_("cannot open file '%s': it is a directory"), name);
    return FALSE;
  }
  fp = R_gzopen(name, mode);
  if(!fp) {
    warning(_("cannot open compressed file '%s', probable reason '%s'"),
            name, strerror(errno));
    return FALSE;
  }
  ((Rgzfileconn)(con->private))->fp = fp;
  con->isopen = TRUE;
  con->canwrite = (con->mode[0] == 'w' || con->mode[0] == 'a');
  con->canread = !con->canwrite;
  con->text = strchr(con->mode, 'b') ? FALSE : TRUE;
  set_buffer(con);
  set_iconv(con);
  con->save = -1000;
  return TRUE;
}

static void gzfile_close(Rconnection con)
{
  R_gzclose(((Rgzfileconn)(con->private))->fp);
  con->isopen = FALSE;
}

static int gzfile_fgetc_internal(Rconnection con)
{
  gzFile fp = ((Rgzfileconn)(con->private))->fp;
  unsigned char c;

  return R_gzread(fp, &c, 1) == 1 ? c : R_EOF;
}

/* This can only seek forwards when writing (when it writes nul bytes).
 When reading, it either seeks forwards of rewinds and reads again */
static double gzfile_seek(Rconnection con, double where, int origin, int rw)
{
  gzFile  fp = ((Rgzfileconn)(con->private))->fp;
  Rz_off_t pos = R_gztell(fp);
  int res, whence = SEEK_SET;

  if (ISNA(where)) return (double) pos;

  switch(origin) {
  case 2: whence = SEEK_CUR; break;
  case 3: error(_("whence = \"end\" is not implemented for gzfile connections"));
  default: whence = SEEK_SET;
  }
  res = R_gzseek(fp, (z_off_t) where, whence);
  if(res == -1)
    warning(_("seek on a gzfile connection returned an internal error"));
  return (double) pos;
}

static int gzfile_fflush(Rconnection con)
{
  return 0;
}

static size_t gzfile_read(void *ptr, size_t size, size_t nitems,
                          Rconnection con)
{
  gzFile fp = ((Rgzfileconn)(con->private))->fp;
  /* uses 'unsigned' for len */
  if ((double) size * (double) nitems > UINT_MAX)
    error(_("too large a block specified"));
  return R_gzread(fp, ptr, (unsigned int)(size*nitems))/size;
}

static size_t gzfile_write(const void *ptr, size_t size, size_t nitems,
                           Rconnection con)
{
  gzFile fp = ((Rgzfileconn)(con->private))->fp;
  /* uses 'unsigned' for len */
  if ((double) size * (double) nitems > UINT_MAX)
    error(_("too large a block specified"));
  return R_gzwrite(fp, (voidp)ptr, (unsigned int)(size*nitems))/size;
}

static Rconnection newgzfile(const char *description, const char *mode,
                             int compress)
{
  Rconnection new;
  new = (Rconnection) malloc(sizeof(struct Rconn));
  if(!new) error(_("allocation of gzfile connection failed"));
  new->class = (char *) malloc(strlen("gzfile") + 1);
  if(!new->class) {
    free(new);
    error(_("allocation of gzfile connection failed"));
    /* for Solaris 12.5 */ new = NULL;
  }
  strcpy(new->class, "gzfile");
  new->description = (char *) malloc(strlen(description) + 1);
  if(!new->description) {
    free(new->class); free(new);
    error(_("allocation of gzfile connection failed"));
    /* for Solaris 12.5 */ new = NULL;
  }
  init_con(new, description, CE_NATIVE, mode);

  new->canseek = TRUE;
  new->open = &gzfile_open;
  new->close = &gzfile_close;
  new->vfprintf = &dummy_vfprintf;
  new->fgetc_internal = &gzfile_fgetc_internal;
  new->fgetc = &dummy_fgetc;
  new->seek = &gzfile_seek;
  new->fflush = &gzfile_fflush;
  new->read = &gzfile_read;
  new->write = &gzfile_write;
  new->private = (void *) malloc(sizeof(struct gzfileconn));
    if(!new->private) {
	free(new->description); free(new->class); free(new);
	error(_("allocation of gzfile connection failed"));
	/* for Solaris 12.5 */ new = NULL;
    }
    ((Rgzfileconn)new->private)->compress = compress;
    return new;
}
