#' Instantiate a WebAssembly module
#'
#' @param path the path to the binary wasm file
#'
#' @export
#' @importFrom methods new
#' @importFrom stats setNames
instantiate <- function(path) {
  stopifnot(file.exists(path))
  f <- file(path, "rb")
  bytes <- readBin(f, what = "raw", n = file.size(path))
  close(f)
  mod <- new(WasmModule)
  mod$instantiate(bytes)
  funs <- mod$get_exported_functions()
  fun_names <- vapply(funs, function(x) x$name, character(1L))
  exports <- lapply(funs, function(x) {
    fun <- x
    fun_parameters <- paste0("x_", seq_len(fun$params_arity))
    if (fun$params_arity == 0L) {
      fun_parameters <- character(0L)
    }
    ret_fun <- function() {
      args <- do.call("list", lapply(fun_parameters, as.name))
      res <- mod$call_exported_function(fun$name, args)
      if (fun$returns_arity == 1L) {
        res[[1L]]
      } else {
        res
      }
    }
    formals(ret_fun) <- as.pairlist(setNames(lapply(fun_parameters, function(x) 0), fun_parameters))
    ret_fun
  })
  exports <- setNames(exports, fun_names)
  memory <- list(
    get_memory_view = function(pointer) {
      stopifnot(is.numeric(pointer))
      pointer <- as.integer(pointer)
      stopifnot(pointer > 0)
      mod$get_memory_view(pointer)
    }
  )
  structure(
    list(
      exports = exports,
      memory = memory
    ), class = "wasmerrr_instance"
  )
}
