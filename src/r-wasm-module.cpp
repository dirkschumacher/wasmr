#include "r-wasm-module.h"
#include "module.h"
#include "helpers.h"
#include <string.h>
#include <stdio.h>

namespace wasmr {

void RcppWasmModule::finalize() {
  for (const finalize_import_func_log_entry& entry : finalize_import_func_log) {
    if (entry.fun_call_context) {
      delete entry.fun_call_context;
    }
    if (entry.func) {
      wasmer_import_func_destroy(entry.func);
    }
    if (entry.trampoline_buffer) {
      wasmer_trampoline_buffer_destroy(entry.trampoline_buffer);
    }
    if (entry.module_name) {
      delete[] entry.module_name;
    }
    if (entry.function_name) {
      delete[] entry.function_name;
    }
  }
  instance.destroy();
};

void RcppWasmModule::instantiate(Rcpp::RawVector bytes, const Rcpp::List imports) {
  if (bytes.size() <= 0) {
    Rcpp::stop("You need at least one byte");
  }
  wasmr::Module module;
  module.compile(&bytes[0], bytes.size());
  instance.set_module(std::move(module));
  instance.instantiate(prepare_imports(imports));
};

inline bool is_wasmer_numeric_value(wasmer_value_tag tag) {
  return tag == wasmer_value_tag::WASM_F64 || tag == wasmer_value_tag::WASM_F32;
}

inline bool is_wasmer_integer_value(wasmer_value_tag tag) {
  return tag == wasmer_value_tag::WASM_I64 || tag == wasmer_value_tag::WASM_I32;
}

SEXP r_wasm_import_function_compute(r_fun_call_context* local_context, uint64_t* data) {
  Rcpp::Function fun = local_context->fun;
  Rcpp::Language lang(fun);
  std::vector<wasmer_value_tag> params_sig = local_context->params_sig;
  int n_params = params_sig.size();
  for (int i = 0; i < n_params; i++) {
    wasmer_value_tag val = params_sig[i];
    if (is_wasmer_numeric_value(val)) {
      Rcpp::stop("Double/Float parameter types not supported yet");
    }
    if (is_wasmer_integer_value(val)) {
      int x = data[i + 1];
      lang.push_back(Rcpp::IntegerVector::create(x));
    }
  }
  return lang.eval();
}

template<class T>
T r_wasm_import_function(r_fun_call_context* local_context, uint64_t* data) {
  return Rcpp::as<T>(r_wasm_import_function_compute(local_context, data));
}

double execute_r_fun_in_wasmer_double(r_fun_call_context* local_context, uint64_t* data) {
  return r_wasm_import_function<double>(local_context, data);
}

int execute_r_fun_in_wasmer_int(r_fun_call_context* local_context, uint64_t* data) {
  return r_wasm_import_function<int>(local_context, data);
}

void execute_r_fun_in_wasmer_void(r_fun_call_context* local_context, uint64_t* data) {
  r_wasm_import_function_compute(local_context, data);
}

wasmer_value_tag wasmer_value_from_str(const std::string& str) {
  if (str == "I32") return wasmer_value_tag::WASM_I32;
  if (str == "I64") return wasmer_value_tag::WASM_I64;
  if (str == "F32") return wasmer_value_tag::WASM_F32;
  if (str == "F64") return wasmer_value_tag::WASM_F64;
  Rcpp::stop("Invalid wasm parameter type.");
}

std::vector<wasmer_value_tag> RcppWasmModule::to_wasmer_value_tags(const Rcpp::CharacterVector vec) {
  std::vector<wasmer_value_tag> ret;
  int len = vec.size();
  for (int i = 0; i < len; i++) {
    std::string type = Rcpp::as<std::string>(vec[i]);
    ret.push_back(wasmer_value_from_str(type));
  }
  return ret;
}

#define WASMR_BUILD_FUNC_CALLBACK(CALLBACK_NAME)                               \
  wasmer_trampoline_buffer_builder_add_callinfo_trampoline(                    \
    tbb,                                                                       \
    (wasmer_trampoline_callable_t *) CALLBACK_NAME,                            \
    (void *) local_context,                                                    \
    params_sig_vec.size() + 1                                                  \
  )

std::vector<wasmer_import_t> RcppWasmModule::prepare_imports(const Rcpp::List imports) {
  // TODO: can I also query the import definitions in the wasm file?
  std::vector<wasmer_import_t> ret;
  SEXP module_names_sexp = Rcpp::wrap(imports.names());
  if (Rf_isNull(module_names_sexp)) {
    return ret;
  }
  auto module_names = Rcpp::as<std::vector<std::string>>(module_names_sexp);
  for (int i = 0; i < module_names.size(); i++) {
    std::string module_name = module_names[i];
    Rcpp::List functions = imports[module_name];
    SEXP function_names_sexp = functions.names();
    if (Rf_isNull(function_names_sexp)) {
      Rcpp::stop("All import functions need to be named.");
    }
    auto function_names = Rcpp::as<std::vector<std::string>>(function_names_sexp);
    for (int j = 0; j < function_names.size(); j++) {
      std::string function_name = function_names[j];
      Rcpp::List typed_fun_list = functions[function_name];
      Rcpp::Function fun = typed_fun_list["fun"];
      wasmer_import_t import;
      wasmer_byte_array module_name_bytes;
      wasmer_byte_array import_name_bytes;

      const std::string::size_type module_name_size = module_name.size();
      char* module_name_char = new char[module_name_size + 1];
      memcpy(module_name_char, module_name.c_str(), module_name_size + 1);
      module_name_bytes.bytes = (const uint8_t*)module_name_char;
      module_name_bytes.bytes_len = strlen(module_name_char);

      const std::string::size_type function_name_size = function_name.size();
      char* function_name_char = new char[function_name_size + 1];
      memcpy(function_name_char, function_name.c_str(), function_name_size + 1);
      import_name_bytes.bytes = (const uint8_t *) function_name_char;
      import_name_bytes.bytes_len = strlen(function_name_char);

      import.module_name = module_name_bytes;
      import.import_name = import_name_bytes;

      import.tag = wasmer_import_export_kind::WASM_FUNCTION;

      auto params_sig_vec = to_wasmer_value_tags(typed_fun_list["param_types"]);
      auto return_sig_vec = to_wasmer_value_tags(typed_fun_list["return_type"]);
      r_fun_call_context* local_context = new r_fun_call_context(fun, params_sig_vec);
      wasmer_trampoline_buffer_builder_t* tbb = wasmer_trampoline_buffer_builder_new();

      unsigned long exec_r_fun_idx;
      bool is_void_return = return_sig_vec.size() == 0;
      bool is_float_return = return_sig_vec.size() == 1 && is_wasmer_numeric_value(return_sig_vec[0]);
      bool is_integer_return = return_sig_vec.size() == 1 && is_wasmer_integer_value(return_sig_vec[0]);
      if (is_void_return) {
        exec_r_fun_idx = WASMR_BUILD_FUNC_CALLBACK(execute_r_fun_in_wasmer_void);
      } else if (is_float_return) {
        exec_r_fun_idx = WASMR_BUILD_FUNC_CALLBACK(execute_r_fun_in_wasmer_double);
      } else if (is_integer_return)  {
        exec_r_fun_idx = WASMR_BUILD_FUNC_CALLBACK(execute_r_fun_in_wasmer_int);
      } else {
        Rcpp::stop("Your input function has a wrong return type");
      }
      wasmer_trampoline_buffer_t* tb = wasmer_trampoline_buffer_builder_build(tbb);
      const wasmer_trampoline_callable_t* exec_r_fun_callable = wasmer_trampoline_buffer_get_trampoline(tb, exec_r_fun_idx);

      wasmer_import_func_t* func = wasmer_import_func_new(
        (void (*)(void*)) exec_r_fun_callable,
        params_sig_vec.data(), params_sig_vec.size(),
        return_sig_vec.data(), return_sig_vec.size()
      );
      import.value.func = func;
      ret.push_back(import);

      // register pointers for finalization at a later point in time
      finalize_import_func_log_entry log_entry;
      log_entry.fun_call_context = local_context;
      log_entry.func = func;
      log_entry.trampoline_buffer = tb;
      log_entry.module_name = module_name_char;
      log_entry.function_name = function_name_char;
      finalize_import_func_log.push_back(log_entry);

    }
  }
  return ret;
};

#define WASMR_SET_PARAM_VALUE(TYPE, WASMER_TYPE, VAL)          \
case wasmer_value_tag::WASMER_TYPE:                            \
  param.value.TYPE = VAL;                                      \
  break;

Rcpp::List RcppWasmModule::call_exported_function(const std::string fun_name, const Rcpp::List arguments) {
  const wasmr::InstanceExportFunction& fun = instance.get_exported_function(fun_name);
  if (arguments.length() != fun.params_arity) {
    Rcpp::stop("The number of arguments is not correct");
  }
  std::vector<wasmer_value_t> params;
  for (int i = 0; i < fun.params_arity; i++) {
    wasmer_value_t param;
    const auto& val = arguments[i];
    param.tag = fun.params_data_types[i];
    switch(param.tag) {
    WASMR_SET_PARAM_VALUE(F32, WASM_F32, val);
    WASMR_SET_PARAM_VALUE(F64, WASM_F64, val);
    WASMR_SET_PARAM_VALUE(I32, WASM_I32, val);
    WASMR_SET_PARAM_VALUE(I64, WASM_I64, val);
    default:
      Rcpp::stop("Unsupported type");
    }
    params.push_back(param);
  }

  auto results = instance.call_exported_function(fun_name, params);

  Rcpp::List ret;
  for (int i = 0; i < fun.returns_arity; i++) {
    const auto& res = results[i];
    const auto& res_type = fun.returns_data_types[i];
    switch(res_type) {
    case wasmer_value_tag::WASM_F32:
      ret.push_back(Rcpp::NumericVector::create(res.value.F32));
      break;
    case wasmer_value_tag::WASM_F64:
      ret.push_back(Rcpp::NumericVector::create(res.value.F64));
      break;
    case wasmer_value_tag::WASM_I32:
      ret.push_back(Rcpp::IntegerVector::create(res.value.I32));
      break;
    case wasmer_value_tag::WASM_I64:
      ret.push_back(Rcpp::IntegerVector::create(res.value.I64));
      break;
    default:
      Rcpp::stop("Unsupported type");
    }
  }
  return ret;
};

constexpr auto WASMER_PAGE_SIZE = 65 * 1000;

Rcpp::RawVector RcppWasmModule::get_memory_as_raw_vector(const uint32_t offset = 0) const {
  auto len = instance.get_memory_length() * WASMER_PAGE_SIZE;
  uint8_t* memory_data = wasmer_memory_data(instance.get_wasmer_memory());
  memory_data = memory_data + offset;
  Rcpp::RawVector ret(len - offset);
  std::copy(memory_data, memory_data + (len - offset), ret.begin());
  return ret;
};


void RcppWasmModule::set_memory(const uint32_t offset, const Rcpp::IntegerVector indexes, const Rcpp::RawVector values) {
  Rcpp::IntegerVector indexes_starting_at_0 = indexes - 1;
  instance.set_memory(
    offset,
    Rcpp::as<std::vector<uint32_t>>(indexes_starting_at_0),
    Rcpp::as<std::vector<uint8_t>>(values)
  );
}

Rcpp::RawVector RcppWasmModule::get_memory(const uint32_t offset, const Rcpp::IntegerVector indexes) {
  auto n = indexes.size();
  uint8_t* memory_data = wasmer_memory_data(instance.get_wasmer_memory());
  memory_data = memory_data + offset;
  Rcpp::RawVector ret(n);
  auto len = instance.get_memory_length() * WASMER_PAGE_SIZE;
  for (auto i = 0; i < n; i++) {
    auto index = indexes[i];
    if (index > len) {
      Rcpp::stop("Index out of bounds");
    }
    ret[i] = memory_data[index - 1];
  }
  return ret;
}

uint32_t RcppWasmModule::get_memory_length() const {
  return instance.get_memory_length();
};

void RcppWasmModule::grow_memory(const uint32_t delta) {
  wasmer_result_t res = wasmer_memory_grow(instance.get_wasmer_memory(), delta);
  if (res != wasmer_result_t::WASMER_OK) {
    Rcpp::stop(wasmr::helpers::last_error());
  }
};

Rcpp::List RcppWasmModule::get_exported_functions() const {
  std::vector<Rcpp::List> ret;
  auto as_int = [](const wasmer_value_tag& x) { return (int) x; };
  for (const auto& fun : instance.get_exported_functions()) {
    auto params_data_types = std::vector<int>(fun.params_arity);
    std::transform(fun.params_data_types.begin(), fun.params_data_types.end(), params_data_types.begin(), as_int);
    auto returns_data_types = std::vector<int>(fun.returns_arity);
    std::transform(fun.returns_data_types.begin(), fun.returns_data_types.end(), returns_data_types.begin(), as_int);
    ret.push_back(
      Rcpp::List::create(
        Rcpp::Named("name") = fun.name,
        Rcpp::Named("index") = fun.idx,
        Rcpp::Named("params_arity") = fun.params_arity,
        Rcpp::Named("params_data_types") = params_data_types,
        Rcpp::Named("returns_arity") = fun.returns_arity,
        Rcpp::Named("returns_data_types") = returns_data_types
      )
    );
  }
  return Rcpp::wrap(ret);
};

}
