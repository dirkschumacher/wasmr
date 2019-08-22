test_that("call functions", {
  instance <- instantiate("../../inst/examples/sum.wasm")
  res <- instance$exports$sum(1, 5)
  expect_equal(res, 6)
})

test_that("get the memory", {
  instance <- instantiate("../../inst/examples/hello.wasm")
  pointer <- instance$exports$hello()
  memory <- instance$memory$get_memory_view(pointer)
  expect_equal(rawToChar(memory), "Hello world")
})

test_that("complex loop", {
  instance <- instantiate("../../inst/examples/fib.wasm")
  res <- instance$exports$fib(20)
  expect_equal(res, 6765)
})

test_that("grow memory", {
  instance <- instantiate("../../inst/examples/hello.wasm")
  len <- instance$memory$get_memory_length()
  instance$memory$grow(1)
  len2 <- instance$memory$get_memory_length()
  expect_true(len < len2)
})

test_that("import functions",{
  imports <- list(
    env = list(
      add = typed_function(
        function(a, b) {
          a + b
        },
        param_types = c("F64", "F64"),
        return_type = c("F64")
      )
    )
  )
  instance <- instantiate("../../inst/examples/sum_import.wasm", imports)
  res <- instance$exports$sum(1, 5)
  expect_equal(res, 6 + 42)
})
