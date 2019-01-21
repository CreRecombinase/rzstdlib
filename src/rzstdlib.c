/* do not remove */

#include <Rdefines.h>
#include <R_ext/Error.h>

#ifdef _WIN32
    #include "hdf5/hdf5.h"
#else
    #include "zstd/lib/zstd.h"
#endif

SEXP rzstdlib_zstd_libversion(void)
{
  unsigned majnum = ZSTD_VERSION_MAJOR;
  unsigned minnum = ZSTD_VERSION_MINOR;
  unsigned relnum = ZSTD_VERSION_RELEASE;
  //    herr_t herr = H5get_libversion( &majnum, &minnum, &relnum );

    SEXP Rval;
    PROTECT(Rval = allocVector(INTSXP, 3));
    INTEGER(Rval)[0] = majnum;
    INTEGER(Rval)[1] = minnum;
    INTEGER(Rval)[2] = relnum;

    SEXP names = PROTECT(allocVector(STRSXP,3));
    SET_STRING_ELT(names, 0, mkChar("majnum"));
    SET_STRING_ELT(names, 1, mkChar("minnum"));
    SET_STRING_ELT(names, 2, mkChar("relnum"));
    SET_NAMES(Rval, names);
    UNPROTECT(1);

    UNPROTECT(1);
    return Rval;
}

#include <R_ext/Rdynload.h>

R_CallMethodDef callMethods[] = {
  {"rzstdlib_hdf5_libversion", (DL_FUNC) &rzstdlib_zstd_libversion, 0},
  {NULL, NULL, 0}
};

void R_init_rzstdlib(DllInfo *info)
{
  R_registerRoutines(info, NULL, callMethods, NULL, NULL);
}
