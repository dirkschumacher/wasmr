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