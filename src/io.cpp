#include "zstd/lib/zstd.h"

#include <memory>
#include <array>
#include <Rcpp.h>


//' @param data decompressed data
//' @param level level of compression
//' @export
// [[Rcpp::export]]
SEXP zstdCompress(Rcpp::RObject data, int level) {
    if (level < 1) {
        Rcpp::warning("compression level can't be less than 1, assuming it is 1");
        level = 1;
    } else if (level > ZSTD_maxCLevel()) {
        std::array<char, 128> msg;
        snprintf(msg.data(), msg.size() - 1, "compression level (=%i) is to high, assuming maximum allowed %i", level, ZSTD_maxCLevel());
        Rcpp::warning(msg.data());
        level = ZSTD_maxCLevel();
    }

    auto input = Rcpp::as<Rcpp::RawVector>(data);
    auto sz = ZSTD_compressBound(input.size());
    auto packed = Rcpp::RawVector::create();

    std::unique_ptr<uint8_t[]> temp(new uint8_t[sz]());

    auto rc = ZSTD_compress(&temp[0], sz, input.begin(), input.size(), level);
    if (!ZSTD_isError(rc)) {
        packed.assign(&temp[0], &temp[rc]);
    } else {
        Rcpp::stop(ZSTD_getErrorName(rc));
    }

    return Rcpp::wrap(packed);
}



//' @param data decompressed data
//' @export
// [[Rcpp::export]]
Rcpp::RawVector zstdDecompress(Rcpp::RObject data) {
    auto input = Rcpp::as<Rcpp::RawVector>(data);
    auto data_size = ZSTD_getDecompressedSize(input.begin(), input.size());

    if (data_size > 0) {
        auto result = Rcpp::RawVector(data_size);
        //std::unique_ptr<uint8_t[]> temp(new uint8_t[sz]());
        auto rc = ZSTD_decompress(&result[0], data_size, input.begin(), input.size());
        if (ZSTD_isError(rc)) {
            Rcpp::stop(ZSTD_getErrorName(rc));
        }
        //result.assign(&temp[0], &temp[rc]);
        return(result);
    } else {
      Rcpp::stop("Couldn't get size of an object. Data corrupted?");
    }

    return Rcpp::RawVector();
}
