
<!-- README.md is generated from README.Rmd. Please edit that file -->

# WebAssembly in R using wasmer

<!-- badges: start -->

[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://www.tidyverse.org/lifecycle/#experimental)
[![Travis build
status](https://travis-ci.org/dirkschumacher/wasmrrr.svg?branch=master)](https://travis-ci.org/dirkschumacher/wasmrrr)
<!-- badges: end -->

The goal of `wasmr` is to run
[WebAssembly](https://developer.mozilla.org/en-US/docs/WebAssembly/Concepts)
code from R using the [wasmer](https://wasmer.io/) runtime.

This is a early version with probably bugs, missing features and not the
best code quality. But it already works and I am happy to hear feedback.
The goal is that this package evolves into something stable.

The package is mainly written in C++ using the C API of the `wasmer`
runtime which is written in `rust`. You therefore need a rust compiler
to install the package.

## Installation

``` r
remotes::install_github("dirkschumacher/wasmr")
```

Also make sure to have a very recent rust compiler. The best way to
install the rust toolchain is to use [rustup](https://rustup.rs/).

## Example

``` r
library(wasmr)
```

``` r
f <- system.file("examples/sum.wasm", package = "wasmr")
instance <- instantiate(f)
instance$exports$sum(10, 20)
#> [1] 30
```

``` r
f <- system.file("examples/hello.wasm", package = "wasmr")
instance <- instantiate(f)
memory_pointer <- instance$exports$hello()
hi <- instance$memory$get_memory_view(memory_pointer)
rawToChar(hi[1:11])
#> [1] "Hello world"
```

``` r
f <- system.file("examples/fib.wasm", package = "wasmr")
instance <- instantiate(f)
instance$exports$fib(20)
#> [1] 6765

fib <- function(n) {
  if (n < 2) return(n)
  fib(n - 1) + fib(n - 2)
}

microbenchmark::microbenchmark(
  instance$exports$fib(20),
  fib(20)
)
#> Unit: microseconds
#>                      expr      min       lq       mean   median         uq
#>  instance$exports$fib(20)   70.714   77.635   126.6736  132.941   143.6045
#>                   fib(20) 7905.294 8338.638 10593.8455 9070.479 11339.4980
#>        max neval
#>    365.488   100
#>  39368.216   100
```

## Memory

``` r
f <- system.file("examples/fib.wasm", package = "wasmr")
instance <- instantiate(f)
instance$memory$get_memory_length()
#> [1] 2

# grow the number of pages
instance$memory$grow(1)
instance$memory$get_memory_length()
#> [1] 3
```

The value returned from `get_memory_view` is a lazy raw vector that just
stores a pointer to the `wasmer` memory. If you use the accessor methods
`[i]` or request a continuous region `[1:n]` then only the necessary
values from the linear memory will be returned as a `raw vector`.

The data structure does not support setting memory … yet.

``` r
f <- system.file("examples/hello.wasm", package = "wasmr")
instance <- instantiate(f)
memory_pointer <- instance$exports$hello()
memory <- instance$memory$get_memory_view(memory_pointer)
.Internal(inspect(memory))
#> @7f99f274d2c8 24 RAWSXP g0c0 [NAM(7)] wasm memory (len=130000, offset=1024)
memory[1:11]
#>  [1] 48 65 6c 6c 6f 20 77 6f 72 6c 64
rawToChar(memory[1:5])
#> [1] "Hello"
```

### Imports

``` r
set.seed(42)
imports <- list(
  env = list(
    add = typed_function(
      function(a, b) {
        # use R's RNG to add a random number to the result
        a + b * as.integer(runif(1) * 100)
      },
      param_types = c("I32", "I32"),
      return_type = "I32"
    )
  )
)
f <- system.file("examples/sum_import.wasm", package = "wasmr")
instance <- instantiate(f, imports)
# sum: add(a, b) + 42
instance$exports$sum(1, 5)
#> [1] 498
```

## Caveats and limitations

  - It is not feature complete and does not support everything that
    `wasm` supports
  - Imported functions can only have integer parameters
  - No globals
  - There is hardly any documentation except for the examples
  - `I32/I64` are mapped to `IntegerVector` and `F32/F64` to
    `NumericVector`. Currently no way to differentiate.
  - I am still learning about `wasm` 🙈
  - WIP

## Inspiration and References

  - [wasmer Python](https://github.com/wasmerio/python-ext-wasm) -
    haven’t tried it but looked at the API/examples
  - [wasmer](https://github.com/wasmerio/wasmer) - especially the C api
    and the tests give some good examples.
  - @jeroen ’s [rust template](https://github.com/r-rust/hellorust)
  - @romainfrancois ’s
    [altrepisode](https://github.com/romainfrancois/altrepisode) and
    @jimhester ’s [vroom](https://github.com/r-lib/vroom) on how to use
    ALTREP and C++.

## Contribute

While this is work in progress, the best way to contribute is to test
the package and write/comment on issues. Before sending a PR it would
great to post an issue first to discuss.

## License

MIT just like wasmer.
