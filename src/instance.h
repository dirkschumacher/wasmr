#ifndef WASMR_INSTANCE_H_
#define WASMR_INSTANCE_H_

#include "module.h"
#include "wasmer.h"
#include <string>
#include <vector>

namespace wasmr {

struct InstanceExportFunction {
  std::string name;
  int idx;

  uint32_t params_arity;
  std::vector<wasmer_value_tag> params_data_types;

  uint32_t returns_arity;
  std::vector<wasmer_value_tag> returns_data_types;
};

class Instance {
public:
  void instantiate();
  void set_module(Module module);
  const std::vector<InstanceExportFunction>& get_exported_functions() const;
  wasmer_memory_t* get_wasmer_memory() const;
  wasmer_exports_t* get_wasmer_exports() const;
  void destroy();

private:
  wasmer_import_t imports[0] = {};
  wasmer_exports_t* exports = NULL;
  wasmer_instance_t* instance = NULL;
  wasmer_memory_t* memory = NULL;
  Module module;
  std::vector<InstanceExportFunction> exported_functions;

  void init_exports_and_memory();
};

}
#endif // WASMR_INSTANCE_H_
