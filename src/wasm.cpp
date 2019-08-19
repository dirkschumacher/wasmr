#include <Rcpp.h>
#include "wasmer.h"
#include "instance.h"
#include "module.h"
#include "helpers.h"

// [[Rcpp::plugins(cpp11)]]

class RcppWasmModule {
public:
  RcppWasmModule() {}

 // TODO: figure out to release everything without the segfault
  void finalize() {
    instance.destroy();
  }

  void instantiate(Rcpp::RawVector bytes) {
    if (bytes.size() <= 0) {
      Rcpp::stop("You need at least one byte");
    }
    wasmr::Module module;
    module.compile(&bytes[0], bytes.size());
    instance.set_module(std::move(module));
    instance.instantiate();
  }

  Rcpp::List call_exported_function(std::string fun_name, Rcpp::List arguments) {
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

    auto results = instance.call_exported_function(fun_name, params);

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
    uint8_t* memory_data = wasmer_memory_data(instance.get_wasmer_memory());
    uint8_t* return_val = memory_data + pointer;
    int len = strlen((char*)return_val);
    Rcpp::RawVector ret(len);
    std::copy(return_val, return_val + len, ret.begin());
    return ret;
  }

  uint32_t get_memory_length() {
    return instance.get_memory_length();
  }

  void grow_memory(uint32_t delta) {
    wasmer_result_t res = wasmer_memory_grow(instance.get_wasmer_memory(), delta);
    if (res != wasmer_result_t::WASMER_OK) {
      Rcpp::stop(wasmr::helpers::last_error());
    }
  }

  Rcpp::List get_exported_functions() {
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
  }

private:
  wasmr::Instance instance;
};

void wasm_finalize(RcppWasmModule* mod) {
  if (mod) {
    mod->finalize();
  }
}

RCPP_MODULE(wasm_module) {
  Rcpp::class_<RcppWasmModule>("WasmModule")
  .constructor()
  .method("instantiate", &RcppWasmModule::instantiate)
  .method("call_exported_function", &RcppWasmModule::call_exported_function)
  .method("get_exported_functions", &RcppWasmModule::get_exported_functions)
  .method("get_memory_length", &RcppWasmModule::get_memory_length)
  .method("get_memory_view", &RcppWasmModule::get_memory_view)
  .method("grow_memory", &RcppWasmModule::grow_memory)
  .finalizer(&wasm_finalize)
  ;
}
