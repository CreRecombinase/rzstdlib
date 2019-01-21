#' Compiler arguments for using Rzstdlib
#'
#' This function returns values for
#' \code{PKG_CC_FLAGS} variable for use in Makevars files.
#'
#' @param opt A scalar character from the list of available options;
#' default is \code{PKG_C_LIBS}.
#' @return \code{NULL}; prints the corresponding value to stdout.
#' @examples
#' pkgconfig("PKG_C_LIBS")
#' @export
pkgconfig <- function(opt = c("PKG_C_LIBS","PKG_RPATH")) {

    path <- Sys.getenv(
        x = "RZSTDLIB_RPATH",
        unset = system.file("libs", package="rzstdlib", mustWork=TRUE)
    )
    if(opt=="PKG_RPATH"){
      cat(paste0("-L",path," -Wl,-rpath,",path," -lzstd"))
    }
    else{

    if (nzchar(.Platform$r_arch)) {
        arch <- sprintf("/%s", .Platform$r_arch)
    } else {
        arch <- ""
    }
    patharch <- paste0(path, arch)

    result <- switch(match.arg(opt),
                     # PKG_CPPFLAGS = {
                     #     sprintf('-I"%s"', system.file("include", package="Rhdf5lib"))
                     # },
                     PKG_C_LIBS = {
                       switch(Sys.info()['sysname'],
                              Windows = {
                                patharch <- gsub(x = shortPathName(patharch),
                                                 pattern = "\\",
                                                 replacement = "/",
                                                 fixed = TRUE)
                                sprintf('-L%s -lzstd',
                                        patharch)
                              }, {
                                sprintf('-L%s -lzstd',
                                        patharch)
                                #                                    sprintf('%s/libhdf5.a %s/libsz.a -lz',
                                #                                           patharch, patharch)
                              }
                       )
                     },
                     PKG_RPATH = {
                       switch(Sys.info()['sysname'],
                              Windows = {
                               stop("Can't set rpath on windows...")
                              }, {
                                sprintf('-rpath,%s/libzstd.so',
                                        patharch)
                                #                                    sprintf('%s/libhdf5.a %s/libsz.a -lz',
                                #                                           patharch, patharch)
                              }
                       )
                     }
    )

    cat(result)
    }
}


