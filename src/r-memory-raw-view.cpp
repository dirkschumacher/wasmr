#include "r-memory-raw-view.h"

namespace wasmr {

SEXP r_wasmer_memory_raw_view::make(Instance* wasm_instance, int32_t offset_shift) {
  SEXP xp = PROTECT(R_MakeExternalPtr(wasm_instance, R_NilValue, R_NilValue));
  SEXP res = R_new_altrep(class_t, xp, Rf_ScalarInteger(offset_shift));
  R_RegisterCFinalizerEx(xp, finalize, FALSE);
  UNPROTECT(1);
  return res;
}

void r_wasmer_memory_raw_view::finalize(SEXP ptr) {
  if (R_ExternalPtrAddr(ptr)) {
    R_ClearExternalPtr(ptr);
  }
}

Instance* r_wasmer_memory_raw_view::ptr(SEXP vec) {
  return static_cast<Instance*>(R_ExternalPtrAddr(R_altrep_data1(vec)));
}

Instance& r_wasmer_memory_raw_view::get(SEXP vec) {
  return *r_wasmer_memory_raw_view::ptr(vec);
}

uint32_t r_wasmer_memory_raw_view::offset(SEXP vec) {
  return INTEGER(R_altrep_data2(vec))[0];
}

R_xlen_t r_wasmer_memory_raw_view::r_length(SEXP vec) {
  constexpr auto WASMER_PAGE_SIZE = 65 * 1000;
  return get(vec).get_memory_length() * WASMER_PAGE_SIZE;
}

Rboolean r_wasmer_memory_raw_view::inspect(SEXP vec, int pre, int deep, int pvec, void (*inspect_subtree)(SEXP, int, int, int)) {
  Rprintf("wasm memory (len=%d, offset=%d)\n", r_length(vec), offset(vec));
  return TRUE;
}

const void* r_wasmer_memory_raw_view::dataptr_or_null(SEXP vec) {
  return ptr(vec) + offset(vec);
}

void* r_wasmer_memory_raw_view::dataptr(SEXP vec, Rboolean writeable) {
  return ptr(vec) + offset(vec);
}

Rbyte r_wasmer_memory_raw_view::raw_elt(SEXP vec, R_xlen_t i) {
  auto shift = offset(vec);
  uint8_t* memory_data = wasmer_memory_data(get(vec).get_wasmer_memory()) + shift;
  if (shift + i < r_length(vec)) {
    return memory_data[i];
  }
  throw std::out_of_range("Element access for memory is out of range");
}

R_xlen_t r_wasmer_memory_raw_view::get_region(SEXP vec, R_xlen_t start, R_xlen_t size, Rbyte* out) {
  uint8_t* memory_data = wasmer_memory_data(get(vec).get_wasmer_memory());
  auto start_offset = start + offset(vec);
  out = memory_data + start_offset;
  R_xlen_t len = r_length(vec) - start_offset;
  return len > size ? len : size;
}

void r_wasmer_memory_raw_view::init(DllInfo* dll) {
  class_t = R_make_altraw_class("r_wasmer_memory_raw_view", "wasmr", dll);

  R_set_altrep_Length_method(class_t, r_length);
  R_set_altrep_Inspect_method(class_t, inspect);

  R_set_altvec_Dataptr_method(class_t, dataptr);
  R_set_altvec_Dataptr_or_null_method(class_t, dataptr_or_null);

  R_set_altraw_Elt_method(class_t, raw_elt);
  R_set_altraw_Get_region_method(class_t, get_region);
}

}

R_altrep_class_t wasmr::r_wasmer_memory_raw_view::class_t;

void init_wasmr_memory_raw_view(DllInfo* dll) {
  wasmr::r_wasmer_memory_raw_view::init(dll);
};
