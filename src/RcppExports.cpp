// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

// zstdCompress
SEXP zstdCompress(Rcpp::RObject data, int level);
RcppExport SEXP _rzstdlib_zstdCompress(SEXP dataSEXP, SEXP levelSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< Rcpp::RObject >::type data(dataSEXP);
    Rcpp::traits::input_parameter< int >::type level(levelSEXP);
    rcpp_result_gen = Rcpp::wrap(zstdCompress(data, level));
    return rcpp_result_gen;
END_RCPP
}
// zstdDecompress
Rcpp::RawVector zstdDecompress(Rcpp::RObject data);
RcppExport SEXP _rzstdlib_zstdDecompress(SEXP dataSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< Rcpp::RObject >::type data(dataSEXP);
    rcpp_result_gen = Rcpp::wrap(zstdDecompress(data));
    return rcpp_result_gen;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_rzstdlib_zstdCompress", (DL_FUNC) &_rzstdlib_zstdCompress, 2},
    {"_rzstdlib_zstdDecompress", (DL_FUNC) &_rzstdlib_zstdDecompress, 1},
    {NULL, NULL, 0}
};

RcppExport void R_init_rzstdlib(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
