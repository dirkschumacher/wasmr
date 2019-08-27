#' Instantiate a WebAssembly module
#'
#' @param path the path to the binary wasm file
#' @param imports a named list of import function
#'
#' @export
#' @importFrom methods new
#' @importFrom stats setNames
#' @include RcppExports.R
instantiate <- function(path, imports = list()) {
  Instance$new(path, imports)
}

Instance <- R6::R6Class(
  "Instance",
  portable = FALSE,
  public = list(
    initialize = function(path, imports = list()) {
      stopifnot(file.exists(path))
      stopifnot(is.list(imports))
      f <- file(path, "rb")
      bytes <- readBin(f, what = "raw", n = file.size(path))
      close(f)
      private$wasm_instance_module <- wasm_init_module()
      wasm_instantiate(private$wasm_instance_module, bytes, imports)
      private$exports_list <- private$build_exports()
      private$memory_list <- private$build_memory()
    }
  ),
  active = list(
    exports = function(value) {
      if (missing(value)) {
        private$exports_list
      } else {
        stop("you cannot assign anything to export")
      }
    },
    memory = function(value) {
      if (missing(value)) {
        private$memory_list
      } else {
        stop("you cannot assign anything to memory")
      }
    }
  ),
  private = list(
    wasm_instance_module = NULL,
    exports_list = list(),
    memory_list = list(),
    finalize = function() {
      wasm_finalize_module(private$wasm_instance_module)
    },
    build_exports = function() {
      funs <- wasm_get_exported_functions(private$wasm_instance_module)
      fun_names <- vapply(funs, function(x) x$name, character(1L))
      exports <- lapply(funs, function(x) {
        fun <- x
        fun_parameters <- paste0("x_", seq_len(fun$params_arity))
        if (fun$params_arity == 0L) {
          fun_parameters <- character(0L)
        }
        ret_fun <- function() {
          args <- do.call("list", lapply(fun_parameters, as.name))
          res <- wasm_call_exported_function(private$wasm_instance_module, fun$name, args)
          if (fun$returns_arity == 1L) {
            res[[1L]]
          } else {
            res
          }
        }
        formals(ret_fun) <- as.pairlist(
          setNames(lapply(fun_parameters, function(x) 0), fun_parameters)
        )
        ret_fun
      })
      exports <- setNames(exports, fun_names)
      exports
    },
    build_memory = function() {
      list(
        get_memory_view = function(pointer) {
          stopifnot(is.numeric(pointer))
          pointer <- as.integer(pointer)
          stopifnot(pointer >= 0)
          wasm_get_memory_view(private$wasm_instance_module, pointer)
        },
        grow = function(delta) {
          stopifnot(is.numeric(delta))
          wasm_grow_memory(private$wasm_instance_module, as.integer(delta))
        },
        get_memory_length = function() {
          wasm_get_memory_length(private$wasm_instance_module)
        }
      )
    }
  )
)
