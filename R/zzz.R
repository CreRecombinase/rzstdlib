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
pkgconfig <- function(opt = c("PKG_C_LIBS", "PKG_RPATH")) {

    path <- Sys.getenv(
        x = "RZSTDLIB_RPATH",
        unset = system.file("lib", package = "rzstdlib", mustWork = TRUE)
    )

    if (nzchar(.Platform$r_arch)) {
        arch <- sprintf("/%s", .Platform$r_arch)
    } else {
        arch <- ""
    }
    patharch <- paste0(path, arch)

    result <- switch(match.arg(opt),
                     PKG_C_LIBS = {
                         switch(Sys.info()["sysname"],
                                Windows = {
                                    patharch <- gsub(x = shortPathName(patharch),
                                                     pattern = "\\",
                                                     replacement = "/",
                                                     fixed = TRUE)
                                    sprintf("-L%s -lzstd",
                                            patharch)
                                }, {
                                    sprintf("%s/libzstd.a ",
                                            patharch, patharch)

                                }
                                )
                     }
                     )

    cat(result)
}
