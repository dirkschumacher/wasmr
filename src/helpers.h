#ifndef WASMR_HELPERS_H_
#define WASMR_HELPERS_H_
#include <string>
#include "wasmer.h"

namespace wasmr {
namespace helpers {
  inline std::string last_error() {
    int error_len = wasmer_last_error_length();
    char* cstr = (char*)std::malloc(error_len);
    wasmer_last_error_message(cstr, error_len);
    std::string rstr(cstr);
    std::free(cstr);
    return rstr;
  }
}
}

#endif // WASMR_HELPERS_H_
