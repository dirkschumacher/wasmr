#include <Rcpp.h>
#include "r-wasm-module.h"

// [[Rcpp::plugins(cpp11)]]
void wasm_finalize(wasmr::RcppWasmModule* mod) {
  if (mod) {
    mod->finalize();
  }
}

// [[Rcpp::export]]
SEXP wasm_init_module() {
  Rcpp::XPtr<wasmr::RcppWasmModule> ret(new wasmr::RcppWasmModule(), true);
  return Rcpp::wrap(ret);
}

// [[Rcpp::export]]
void wasm_finalize_module(Rcpp::XPtr<wasmr::RcppWasmModule> module) {
  module->finalize();
}

// [[Rcpp::export]]
void wasm_instantiate(Rcpp::XPtr<wasmr::RcppWasmModule> module, Rcpp::RawVector bytes, Rcpp::List imports) {
  module->instantiate(bytes, imports);
}

// [[Rcpp::export]]
Rcpp::List wasm_call_exported_function(Rcpp::XPtr<wasmr::RcppWasmModule> module, std::string fun_name, Rcpp::List arguments) {
  return module->call_exported_function(fun_name, arguments);
}

// [[Rcpp::export]]
Rcpp::List wasm_get_exported_functions(Rcpp::XPtr<wasmr::RcppWasmModule> module) {
  return module->get_exported_functions();
}

// [[Rcpp::export]]
uint32_t wasm_get_memory_length(Rcpp::XPtr<wasmr::RcppWasmModule> module) {
  return module->get_memory_length();
}

// [[Rcpp::export]]
Rcpp::RawVector wasm_get_memory_as_raw_vec(Rcpp::XPtr<wasmr::RcppWasmModule> module, uint32_t offset) {
  return module->get_memory_as_raw_vector(offset);
}

// [[Rcpp::export]]
void wasm_grow_memory(Rcpp::XPtr<wasmr::RcppWasmModule> module, uint32_t delta) {
  return module->grow_memory(delta);
}

// [[Rcpp::export]]
void wasm_set_memory(Rcpp::XPtr<wasmr::RcppWasmModule> module, uint32_t offset, Rcpp::IntegerVector indexes, Rcpp::RawVector values) {
  return module->set_memory(offset, indexes, values);
}

// [[Rcpp::export]]
Rcpp::RawVector wasm_get_memory(Rcpp::XPtr<wasmr::RcppWasmModule> module, uint32_t offset, Rcpp::IntegerVector indexes) {
  return module->get_memory(offset, indexes);
}
