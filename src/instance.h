#ifndef WASMR_INSTANCE_H_
#define WASMR_INSTANCE_H_

#include "module.h"
#include "wasmer.h"
#include <string>
#include <vector>
#include <unordered_map>

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
  void instantiate(const std::vector<wasmer_import_t> imports);
  void set_module(const Module module);
  std::vector<InstanceExportFunction> get_exported_functions() const;
  const InstanceExportFunction& get_exported_function(const std::string name);
  uint32_t get_memory_length() const;
  std::vector<wasmer_value_t> call_exported_function(const std::string fun_name, std::vector<wasmer_value_t> params);
  wasmer_memory_t* get_wasmer_memory() const;
  wasmer_exports_t* get_wasmer_exports() const;
  void set_memory(const uint32_t offset, const std::vector<uint32_t>& indexes, const std::vector<uint8_t>& values);
  void destroy();

private:
  std::vector<wasmer_import_t> imports_vec;
  wasmer_exports_t* exports = NULL;
  wasmer_instance_t* instance = NULL;
  wasmer_memory_t* memory = NULL;
  Module module;
  std::unordered_map<std::string, InstanceExportFunction> exported_functions;
  void init_exports_and_memory();
};

}
#endif // WASMR_INSTANCE_H_
