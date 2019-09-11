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
  void instantiate(Rcpp::RawVector bytes, const Rcpp::List imports);
  Rcpp::List call_exported_function(const std::string fun_name, const Rcpp::List arguments);
  Rcpp::RawVector get_memory_as_raw_vector(const uint32_t offset) const;
  void set_memory(const uint32_t offset, const Rcpp::IntegerVector indexes, const Rcpp::RawVector values);
  Rcpp::RawVector get_memory(const uint32_t offset, const Rcpp::IntegerVector indexes);
  uint32_t get_memory_length() const;
  void grow_memory(const uint32_t delta);
  Rcpp::List get_exported_functions() const;
private:
  Instance instance;
  std::vector<finalize_import_func_log_entry> finalize_import_func_log;

  std::vector<wasmer_import_t> prepare_imports(const Rcpp::List imports);
  std::vector<wasmer_value_tag> to_wasmer_value_tags(const Rcpp::CharacterVector);
};

}
#endif
