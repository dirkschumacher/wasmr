#' Instantiate a WebAssembly module
#'
#' @param path the path to the binary wasm file
#'
#' @export
#' @importFrom methods new
#' @importFrom stats setNames
instantiate <- function(path) {
  Instance$new(path)
}

Instance <- R6::R6Class(
  "Instance",
  portable = FALSE,
  public = list(
    initialize = function(path) {
      stopifnot(file.exists(path))
      f <- file(path, "rb")
      bytes <- readBin(f, what = "raw", n = file.size(path))
      close(f)
      private$wasm_instance_module <- new(WasmModule)
      private$wasm_instance_module$instantiate(bytes)

      private$exports_list <- private$build_exports()
      private$memory_list <- private$build_memory()
    },
    finalize = function() {
      # TODO: might need to do some deallocation here
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
    build_exports = function() {
      funs <- private$wasm_instance_module$get_exported_functions()
      fun_names <- vapply(funs, function(x) x$name, character(1L))
      exports <- lapply(funs, function(x) {
        fun <- x
        fun_parameters <- paste0("x_", seq_len(fun$params_arity))
        if (fun$params_arity == 0L) {
          fun_parameters <- character(0L)
        }
        ret_fun <- function() {
          args <- do.call("list", lapply(fun_parameters, as.name))
          res <- private$wasm_instance_module$call_exported_function(fun$name, args)
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
          stopifnot(pointer > 0)
          private$wasm_instance_module$get_memory_view(pointer)
        },
        grow = function(delta) {
          stopifnot(is.numeric(delta))
          private$wasm_instance_module$grow_memory(as.integer(delta))
        },
        get_memory_length = function() {
          private$wasm_instance_module$get_memory_length()
        }
      )
    }
  )
)
