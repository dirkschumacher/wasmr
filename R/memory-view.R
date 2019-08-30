#' Create a new memory view
#'
#' @param wasm_instance_module a pointer to the rcpp wasm module
#' @param instance an object of the Instance class
#' @param offset a positive integer or 0
#'
#' @export
#' @include RcppExports.R
new_memory_view <- function(wasm_instance_module, instance, offset) {
  MemoryView$new(wasm_instance_module, instance, offset)
}

MemoryView <- R6::R6Class(
  "MemoryView",
  portable = FALSE,
  public = list(
    initialize = function(wasm_instance_module, instance, offset) {
      stopifnot(is.integer(offset), offset >= 0L)
      private$instance <- instance
      private$offset <- offset
      private$wasm_instance_module <- wasm_instance_module
    },
    get = function(indexes) {
      stopifnot(all(indexes >= 1))
      indexes <- as.integer(indexes)
      wasm_get_memory(private$wasm_instance_module, private$offset, indexes)
    },
    set = function(indexes, values) {
      stopifnot(is.raw(values), is.numeric(indexes), length(indexes) == length(values))
      indexes <- as.integer(indexes)
      stopifnot(all(indexes >= 1))
      wasm_set_memory(private$wasm_instance_module, private$offset, indexes, values)
    },
    as_raw_vec = function() {
      private$instance$memory$get_memory_raw_vec(private$offset)
    }
  ),
  private = list(
    instance = NULL,
    wasm_instance_module = NULL,
    offset = NULL
  )
)
