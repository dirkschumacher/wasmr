
<!-- README.md is generated from README.Rmd. Please edit that file -->

# WebAssembly in R using wasmer

<!-- badges: start -->

[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://www.tidyverse.org/lifecycle/#experimental)
[![Travis build
status](https://travis-ci.org/dirkschumacher/wasmrrr.svg?branch=master)](https://travis-ci.org/dirkschumacher/wasmrrr)
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
(hi <- instance$memory$get_memory_view(memory_pointer))
#>  [1] 48 65 6c 6c 6f 20 77 6f 72 6c 64
rawToChar(hi)
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
#>                      expr      min        lq       mean   median
#>  instance$exports$fib(20)   79.486   93.3555   164.3562  163.860
#>                   fib(20) 7967.592 8342.5465 10052.8656 9480.994
#>          uq      max neval
#>    188.2435   471.73   100
#>  11050.3475 17489.67   100
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

## Contribute

While this is work in progress, the best way to contribute is to test
the package and write/comment on issues. Before sending a PR it would
great to post an issue first to discuss.

## License

MIT just like wasmer.
