#include <Rcpp.h>
#include "wasmer.hpp"

// [[Rcpp::plugins(cpp11)]]

// store all the functions we can find
struct WasmExportFunction {
  std::string name;
  int idx;

  uint32_t params_arity;
  std::vector<wasmer_value_tag> params_data_types;

  uint32_t returns_arity;
  std::vector<wasmer_value_tag> returns_data_types;
};
class WasmModule {
public:
  WasmModule() {}

 // TODO: figure out to release everything without the segfault
  void finalize() {
    if (instance) wasmer_instance_destroy(instance);
    // if (exports) wasmer_exports_destroy(exports);
    // if (memory) wasmer_memory_destroy(memory);
  }

  void instantiate(Rcpp::RawVector bytes) {
    if (bytes.size() <= 0) {
      Rcpp::stop("You need at least one byte");
    }
    wasmer_result_t compile_result = wasmer_instantiate(&instance, &bytes[0], bytes.size(), imports, 0);

    if (compile_result != wasmer_result_t::WASMER_OK) {
      throw std::logic_error(last_error());
    }
    parse_functions();
  }

  Rcpp::List call_exported_function(std::string fun_name, Rcpp::List arguments) {
    auto fun_it = std::find_if(
      exported_functions.begin(), exported_functions.end(),
      [&fun_name](const WasmExportFunction& x) { return x.name == fun_name; });
    if (fun_it == exported_functions.end()) {
      Rcpp::stop("Function not found");
    }
    const WasmExportFunction& fun = *fun_it;
    if (arguments.length() != fun.params_arity) {
      Rcpp::stop("The number of arguments is not correct");
    }
    std::vector<wasmer_value_t> params;
    for (int i = 0; i < fun.params_arity; i++) {
      wasmer_value_t param;
      const auto& val = arguments[i];
      param.tag = fun.params_data_types[i];
      switch(param.tag) {
        case wasmer_value_tag::WASM_F32:
          param.value.F32 = (float_t)val;
          break;
        case wasmer_value_tag::WASM_F64:
          param.value.F64 = (double)val;
          break;
        case wasmer_value_tag::WASM_I32:
          param.value.I32 = (int32_t)val;
          break;
        case wasmer_value_tag::WASM_I64:
          param.value.I64 = (int64_t)val;
          break;
        default:
          Rcpp::stop("Unsupported type");
      }
      params.push_back(param);
    }

    std::vector<wasmer_value_t> results(fun.returns_arity);
    wasmer_export_t* exp = wasmer_exports_get(exports, fun.idx);
    wasmer_import_export_kind kind = wasmer_export_kind(exp);
    if (kind != wasmer_import_export_kind::WASM_FUNCTION) {
      Rcpp::stop("The function you are calling is not an actual function");
    }
    const wasmer_export_func_t* wasm_func = wasmer_export_to_func(exp);
    wasmer_result_t call_result;
    if (fun.params_arity == 0) {
      // TODO: need to figure out that works with std::vector
      wasmer_value_t empty_params[] = {};
      call_result = wasmer_export_func_call(wasm_func,
                                            empty_params,
                                            0,
                                            results.data(),
                                            fun.returns_arity);
    } else {
      call_result = wasmer_export_func_call(wasm_func,
                                            params.data(),
                                            fun.params_arity,
                                            results.data(),
                                            fun.returns_arity);
    }
    if (call_result != wasmer_result_t::WASMER_OK) {
      Rcpp::stop(last_error());
    }
    Rcpp::List ret;
    for (int i = 0; i < fun.returns_arity; i++) {
      const auto& res = results[i];
      const auto& res_type = fun.returns_data_types[i];
      switch(res_type) {
      case wasmer_value_tag::WASM_F32:
        ret.push_back(Rcpp::NumericVector::create((float)res.value.F32));
        break;
      case wasmer_value_tag::WASM_F64:
        ret.push_back(Rcpp::NumericVector::create((double)res.value.F64));
        break;
      case wasmer_value_tag::WASM_I32:
        ret.push_back(Rcpp::IntegerVector::create((int32_t)res.value.I32));
        break;
      case wasmer_value_tag::WASM_I64:
        ret.push_back(Rcpp::IntegerVector::create((int64_t)res.value.I64));
        break;
      default:
        Rcpp::stop("Unsupported type");
      }
    }
    return ret;
  }

  Rcpp::RawVector get_memory_view(int32_t pointer) {
    // does that always work????
    uint8_t* memory_data = wasmer_memory_data(memory);
    uint8_t* return_val = memory_data + pointer;
    int len = strlen((char*)return_val);
    Rcpp::RawVector ret(len);
    std::copy(return_val, return_val + len, ret.begin());
    return ret;
  }

  uint32_t get_memory_length() {
    return wasmer_memory_length(memory);
  }

  Rcpp::List get_exported_functions() {
    std::vector<Rcpp::List> ret;
    auto as_int = [](const wasmer_value_tag& x) { return (int) x; };
    for (const auto& fun : exported_functions) {
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
  }

private:
  wasmer_import_t imports[0] = {};
  wasmer_instance_t* instance = NULL;
  wasmer_exports_t* exports = NULL;
  wasmer_memory_t* memory = NULL;

  std::vector<WasmExportFunction> exported_functions;

  std::string last_error() {
    int error_len = wasmer_last_error_length();
    char* cstr = (char*)std::malloc(error_len);
    wasmer_last_error_message(cstr, error_len);
    std::string rstr(cstr);
    std::free(cstr);
    return rstr;
  }

  void parse_functions() {
    wasmer_instance_exports(instance, &exports);
    int exports_len = wasmer_exports_len(exports);
    for (int i = 0; i < exports_len; i++) {
      wasmer_export_t* exp = wasmer_exports_get(exports, i);
      wasmer_import_export_kind kind = wasmer_export_kind(exp);
      if (kind == wasmer_import_export_kind::WASM_FUNCTION) {
        WasmExportFunction fun;
        wasmer_byte_array w_name = wasmer_export_name(exp);
        const wasmer_export_func_t* wasm_func = wasmer_export_to_func(exp);

        uint32_t params_arity;
        wasmer_export_func_params_arity(wasm_func, &params_arity);

        wasmer_value_tag* params_sig = (wasmer_value_tag*)std::malloc(sizeof(wasmer_value_tag) * params_arity);
        wasmer_export_func_params(wasm_func, params_sig, params_arity);
        std::vector<wasmer_value_tag> params_data_types(params_arity);
        for (int j = 0; j < params_arity; j++) {
          params_data_types[j] = params_sig[j];
        }
        std::free(params_sig);

        uint32_t returns_arity;
        wasmer_export_func_returns_arity(wasm_func, &returns_arity);

        wasmer_value_tag* returns_sig = (wasmer_value_tag*)std::malloc(sizeof(wasmer_value_tag) * returns_arity);
        wasmer_export_func_returns(wasm_func, returns_sig, returns_arity);
        std::vector<wasmer_value_tag> returns_data_types(returns_arity);
        for (int j = 0; j < returns_arity; j++) {
          returns_data_types[j] = returns_sig[j];
        }
        std::free(returns_sig);

        fun.name = std::string(reinterpret_cast<const char*>(w_name.bytes), w_name.bytes_len);
        fun.idx = i;
        fun.params_arity = params_arity;
        fun.params_data_types = params_data_types;
        fun.returns_arity = returns_arity;
        fun.returns_data_types = returns_data_types;

        exported_functions.push_back(fun);
      }
      if (kind == wasmer_import_export_kind::WASM_MEMORY) {
        wasmer_result_t export_to_memory_result = wasmer_export_to_memory(exp, &memory);
        if (export_to_memory_result != wasmer_result_t::WASMER_OK) {
          Rcpp::stop(last_error());
        }
      }
    }
  }
};

void wasm_finalize(WasmModule* mod) {
  if (mod) {
    mod->finalize();
  }
}

RCPP_MODULE(wasm_module) {
  Rcpp::class_<WasmModule>("WasmModule")
  .constructor()
  .method("instantiate", &WasmModule::instantiate)
  .method("call_exported_function", &WasmModule::call_exported_function)
  .method("get_exported_functions", &WasmModule::get_exported_functions)
  .method("get_memory_length", &WasmModule::get_memory_length)
  .method("get_memory_view", &WasmModule::get_memory_view)
  .finalizer(&wasm_finalize)
  ;
}
