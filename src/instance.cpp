#include "instance.h"
#include "module.h"
#include "helpers.h"

namespace wasmr {

  void Instance::set_module(Module mod) {
    module = std::move(mod);
  }

  void Instance::destroy() {
    if (instance) {
      wasmer_instance_destroy(instance);
      module.destroy();
    }
  }

  wasmer_memory_t* Instance::get_wasmer_memory() const {
    return memory;
  }

  wasmer_exports_t* Instance::get_wasmer_exports() const {
    return exports;
  }

  void Instance::instantiate() {
    wasmer_module_t* wasmer_mod = module.get_wasmer_module();
    auto compile_result = wasmer_module_instantiate(wasmer_mod, &instance, imports, 0);
    if (compile_result != wasmer_result_t::WASMER_OK) {
      throw std::logic_error(helpers::last_error());
    }
    Instance::init_exports_and_memory();
  };

  void Instance::init_exports_and_memory() {
    wasmer_instance_exports(instance, &exports);
    int exports_len = wasmer_exports_len(exports);
    for (int i = 0; i < exports_len; i++) {
      wasmer_export_t* exp = wasmer_exports_get(exports, i);
      wasmer_import_export_kind kind = wasmer_export_kind(exp);
      if (kind == wasmer_import_export_kind::WASM_FUNCTION) {
        InstanceExportFunction fun;
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
          throw std::logic_error(helpers::last_error());
        }
      }
    }
  }

  const std::vector<InstanceExportFunction>& Instance::get_exported_functions() const {
    return exported_functions;
  }
}
