#ifndef WASMR_RCPPWASMMODULE_INSTANCE_H_
#define WASMR_RCPPWASMMODULE_INSTANCE_H_
#include "wasmer.h"
#include "instance.h"
#include <Rcpp.h>
namespace wasmr {

struct r_fun_call_context {
  r_fun_call_context(Rcpp::Function fun, std::vector<wasmer_value_tag> params_sig) : fun(fun), params_sig(params_sig) {};
  Rcpp::Function fun;
  std::vector<wasmer_value_tag> params_sig;
};

struct finalize_import_func_log_entry {
  r_fun_call_context* fun_call_context;
  wasmer_import_func_t* func;
  wasmer_trampoline_buffer_t* trampoline_buffer;
  char* module_name;
  char* function_name;
};

// this functions as the interface towards R
class RcppWasmModule {
public:
  void finalize();
  void instantiate(Rcpp::RawVector bytes, Rcpp::List imports);
  Rcpp::List call_exported_function(std::string fun_name, Rcpp::List arguments);
  SEXP get_memory_view(uint32_t offset);
  void set_memory(uint32_t offset, Rcpp::IntegerVector indexes, Rcpp::RawVector values);
  uint32_t get_memory_length();
  void grow_memory(uint32_t delta);
  Rcpp::List get_exported_functions();
private:
  Instance instance;
  std::vector<finalize_import_func_log_entry> finalize_import_func_log;

  std::vector<wasmer_import_t> prepare_imports(Rcpp::List imports);
  std::vector<wasmer_value_tag> to_wasmer_value_tags(Rcpp::CharacterVector);
};

}
#endif
