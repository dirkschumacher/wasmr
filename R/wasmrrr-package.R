## usethis namespace: start
#' @useDynLib wasmrrr, .registration = TRUE
#' @import Rcpp
## usethis namespace: end
NULL

loadModule("wasm_module", TRUE)
