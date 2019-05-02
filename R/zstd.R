
zstd_path <-function(){
  retpath <- fs::path(fs::path_package("rzstdlib"),"bin","zstd")
  stopifnot(fs::file_exists(retpath))
  return(retpath)
}


#' Silly little function that adds a suffix to a path if it's already there, and removes it if isn't
#'
#' @param file
#' @param suffix
#'
#' @return
#' @export
#'
#' @examples
swap_suffix <- function(file,suffix="zstd"){
  ext <- fs::path_ext(file)
  if(ext==suffix){
    return(fs::path_ext_remove(file))
  }
  return(fs::path_ext_set(file,suffix))
}


#' Compress/decompress a file using the zstd protocol
#'
#' @param file A character(1) path to an existing file. This file will be compressed using the ZSTD algorithm if it is uncompressed, and it will be decompressed if it is already compressed
#' @param dest A character(1) path to the destination file. If dest exists, then it is only over-written when overwrite=TRUE.
#' @param level an integer(1) argument in the closed set 1-19 specifying compression level. A higher compression level may result in a smaller file size, but will take longer to compute.  This is ignored in the case of decompression
#' @param overwrite a logical(1) indicating whether `dest` should be over-written if it already exists.
#' @param dictionary If specified, a character(1) path to an existing dictionary for dictionary-based compression.
#' @param remove_source A logical(1) indicating whether to remove `file` after compression/decompression is successful
#' @param decompression a logical(1) indicating whether to compress or decompress the file.  By default, this will be inferred based on the file extension
#'
#' @return The full path to `dest`
#' @export
#'
#' @examples
#'
#' # get some text
#' txt <- readLines(file.path(R.home("doc"), "COPYING"))
#' #generate a temporary file
#' tmpf <- fs::file_temp()
#' #write the text to the file
#' write(txt,tmpf)
#' #compress the result
#' dest <- zstd(tmpf)
zstd <- function(file,dest=NULL,level=3L,overwrite = FALSE,dictionary=NULL,remove_source=FALSE,decompression=NA){

  if(is.na(decompression)){
    if(is.null(dest)){
      dest <-swap_suffix(file)
    }
    decompression <- fs::path_ext(dest)=="zstd"
  }


# fs::path_ext
  stopifnot(fs::file_exists(file))
  if(overwrite){
    stopifnot(!fs::file_exists(dest))
  }
  #level <- as.integer(level)
  stopifnot(!is.na(level),is.integer(level))
  zstd_cmd <- as.character(zstd_path())
  dict_cmd <-character(1)
  if(!is.null(dictionary)){
    stopifnot(!file.exists(dictionary))
    dict_cmd <- paste0(" -D ",fs::path_expand(dictionary)," ")
  }
  overwrite_cmd <- ifelse(overwrite," -f ","")
  remove_src_cmd <- ifelse(remove_source," --rm ","")
  zstd_args <-paste0(dict_cmd,overwrite_cmd,remove_src_cmd," ",file," -o ",fs::path_expand(dest))
  result <- processx::run(zstd_cmd,stringr::str_trim(zstd_args),error_on_status = F)
  return(dest)
}
