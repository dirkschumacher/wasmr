#include "module.h"
#include "helpers.h"
#include <stdexcept>
namespace wasmr {

void Module::compile(uint8_t* wasm_bytes, const uint32_t wasm_bytes_len) {
  bool is_valid = wasmer_validate(wasm_bytes, wasm_bytes_len);
  if (!is_valid) {
    throw std::runtime_error("wasm module is not valid.");
  }
  auto result = wasmer_compile(&module, wasm_bytes, wasm_bytes_len);
  if (result != wasmer_result_t::WASMER_OK) {
    throw std::runtime_error(helpers::last_error());
  }
};

void Module::destroy() {
  if (module) {
    wasmer_module_destroy(module);
  }
}

wasmer_module_t* Module::get_wasmer_module() const {
  return module;
}

}
