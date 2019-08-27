#ifndef R_WASMER_MEMORY_RAW_VIEW_H
#define R_WASMER_MEMORY_RAW_VIEW_H

#include <R.h>
#include <Rinternals.h>
#include <R_ext/Altrep.h>
#include "instance.h"
#include "wasmer.h"

namespace wasmr {
struct r_wasmer_memory_raw_view {

public:

  static R_altrep_class_t class_t;

  static SEXP make(Instance* wasm_instance, int32_t offset_shift);

  static void finalize(SEXP ptr);

  static Instance* ptr(SEXP x);

  static Instance& get(SEXP vec);

  static uint32_t offset(SEXP vec);

  // ALTREP methods
  static R_xlen_t r_length(SEXP vec);
  static Rboolean inspect(SEXP x, int pre, int deep, int pvec, void (*inspect_subtree)(SEXP, int, int, int));

  // ALTVEC methods
  static const void* dataptr_or_null(SEXP vec);
  static void* dataptr(SEXP vec, Rboolean writeable);

  // ALTRAW methods
  static Rbyte raw_elt(SEXP vec, R_xlen_t i);
  static R_xlen_t get_region(SEXP vec, R_xlen_t start, R_xlen_t size, Rbyte* out);

  static void init(DllInfo* dll);

};

}

// [[Rcpp::init]]
void init_wasmr_memory_raw_view(DllInfo* dll);
#endif
