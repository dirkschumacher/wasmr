
<!-- README.md is generated from README.Rmd. Please edit that file -->

# WebAssembly in R using wasmer

<!-- badges: start -->

[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://www.tidyverse.org/lifecycle/#experimental)
<!-- badges: end -->

The goal of wasmrrr is to run
[WebAssembly](https://developer.mozilla.org/en-US/docs/WebAssembly/Concepts)
code from R using the [wasmer](https://wasmer.io/) runtime.

This is a super early version with bugs and missing features. It is a
first prototype for me to learn more about WebAssembly, but will evolve
hopefully into something stable. It has the appropriate code quality of
a prototype :).

## Installation

``` r
remotes::install_github("dirkschumacher/wasmerrr")
```

## Example

``` r
library(wasmrrr)
```

``` r
f <- system.file("examples/sum.wasm", package = "wasmrrr")
instance <- instantiate(f)
instance$exports$sum(10, 20)
#> [1] 30
```

``` r
f <- system.file("examples/hello.wasm", package = "wasmrrr")
instance <- instantiate(f)
memory_pointer <- instance$exports$hello()
(hi <- instance$memory$get_memory_view(memory_pointer))
#>  [1] 48 65 6c 6c 6f 20 77 6f 72 6c 64
rawToChar(hi)
#> [1] "Hello world"
```

``` r
f <- system.file("examples/fib.wasm", package = "wasmrrr")
instance <- instantiate(f)
instance$exports$fib(20)
#> [1] 6765

fib <- function(n) {
  if (n < 2) return(n);
  fib(n - 1) + fib(n - 2);
}

microbenchmark::microbenchmark(
  instance$exports$fib(20),
  fib(20)
)
#> Unit: microseconds
#>                      expr      min        lq      mean   median         uq
#>  instance$exports$fib(20)   76.423   79.9405  134.6831   91.756   167.7595
#>                   fib(20) 7904.662 8149.7310 9696.9050 8741.592 10578.1010
#>        max neval
#>    461.343   100
#>  21856.367   100
```

## Inspiration and References

  - [wasmer Python](https://github.com/wasmerio/python-ext-wasm) -
    haven’t tried it but looked at the API/examples
  - [wasmer](https://github.com/wasmerio/wasmer) - especially the C api
    and the tests give some good examples.
  - @jeroen’s [rust template](https://github.com/r-rust/hellorust)
  - [Rcpp’s
    modules](http://dirk.eddelbuettel.com/code/rcpp/Rcpp-modules.pdf)

## Todo

  - No import support yet - any wasm file with imports does not work
  - No Table
  - bug fixes
  - Read more about the design of wasm 🙈

## License

MIT just like wasmer.