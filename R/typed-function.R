#' Use this to define a typed function
#'
#' @param fun the function
#' @param param_types a character vector of wasm types with the length of the
#'   numbers of parameters
#' @param return_type a character vector of length 0 or 1
#'
#' @export
typed_function <- function(fun, param_types, return_type = character()) {
  stopifnot(length(return_type) <= 1L)
  allowed_types <- c("F32", "F64", "I32", "I64")
  stopifnot(all(return_type %in% c(allowed_types, "")))
  stopifnot(all(param_types %in% c("I32", "I64")))
  stopifnot(is.function(fun))
  stopifnot(length(formals(fun)) == length(param_types))
  list(
    fun = function(...) {
      args <- list(...)
      stopifnot(all(vapply(args, length, integer(1L)) == 1L))
      ret <- fun(...)
      stopifnot(length(ret) == 1L, is.numeric(ret))
      ret
    },
    param_types = param_types,
    return_type = return_type
  )
}
