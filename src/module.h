#ifndef WASMR_MODULE_H_
#define WASMR_MODULE_H_

#include "wasmer.h"

namespace wasmr {

class Module {
public:
  void compile(uint8_t *wasm_bytes, const uint32_t wasm_bytes_len);
  void destroy();
  wasmer_module_t* get_wasmer_module() const;

private:
  wasmer_module_t* module = NULL;
};

}
#endif // WASMR_MODULE_H_
