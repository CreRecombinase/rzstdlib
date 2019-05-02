context("compression/decompression")

test_that("compress,decompress", {
    tv <- 1:5
    z <- writeBin(tv, raw(), size = 2)
    rz <- readBin(z, what = integer(), size = 2)
    cmp <- zstdCompress(z, level = 2)
    dcmp <- zstdDecompress(cmp)
    expect_equal(tv, readBin(dcmp, what = integer(), size = 2, n = length(tv)))
})



test_that("compress,decompress",{
  tv <- runif(100)
  z <- writeBin(tv, raw())
  rz <- readBin(z,what = numeric(),n = length(tv))
  cmp <- zstdCompress(z, level = 2)
  dcmp <- zstdDecompress(cmp)
  expect_equal(tv, readBin(dcmp,what=numeric(),n=length(tv)))
})
