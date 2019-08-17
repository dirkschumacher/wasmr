## usethis namespace: start
#' @useDynLib wasmr, .registration = TRUE
#' @import Rcpp
## usethis namespace: end
NULL

loadModule("wasm_module", TRUE)
