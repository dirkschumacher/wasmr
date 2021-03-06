test_that("call functions", {
  file <- system.file("examples", "sum.wasm", package = "wasmr")
  instance <- instantiate(file)
  res <- instance$exports$sum(1, 5)
  expect_equal(res, 6)
})

test_that("get the memory", {
  file <- system.file("examples", "hello.wasm", package = "wasmr")
  instance <- instantiate(file)
  pointer <- instance$exports$hello()
  memory <- instance$memory$get_memory_view(pointer)
  expect_equal(rawToChar(memory$get(1:11)), "Hello world")
})

test_that("complex loop", {
  file <- system.file("examples", "fib.wasm", package = "wasmr")
  instance <- instantiate(file)
  res <- instance$exports$fib(20)
  expect_equal(res, 6765)
})

test_that("grow memory", {
  file <- system.file("examples", "hello.wasm", package = "wasmr")
  instance <- instantiate(file)
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
          (a + b) * 2
        },
        param_types = c("I32", "I32"),
        return_type = c("I32")
      )
    )
  )
  file <- system.file("examples", "sum_import.wasm", package = "wasmr")
  instance <- instantiate(file, imports)
  res <- instance$exports$sum(1, 5)
  expect_equal(res, 6 * 2 + 42)
})


test_that("import functions 2",{
  imports <- list(
    env = list(
      add = typed_function(
        function(a, b) {
          (a + b)
        },
        param_types = c("I32", "I32"),
        return_type = c("I32")
      ),
      add2 = typed_function(
        function(a, b) {
          (a + b)
        },
        param_types = c("I32", "I32"),
        return_type = c("I32")
      )
    )
  )
  file <- system.file("examples", "two-imports.wasm", package = "wasmr")
  instance <- instantiate(file, imports)
  res <- instance$exports$sum(1, 5)
  expect_equal(res, (1 + 5) * 2)
})


test_that("write to memory", {
  file <- system.file("examples", "greet.wasm", package = "wasmr")
  instance <- instantiate(file)
  subject <- charToRaw("everyone")
  length_of_subject <- length(subject)
  input_pointer <- instance$exports$allocate(length_of_subject)
  memory <- instance$memory$get_memory_view(input_pointer)
  memory$set(1:length_of_subject, subject)
  memory$set(length_of_subject + 1, as.raw(0)) # C-string terminates by NULL.
  output_pointer <- instance$exports$greet(input_pointer)
  memory <- instance$memory$get_memory_view(output_pointer)
  expected_output <- "Hello, everyone!"
  expect_equal(rawToChar(memory$get(1:16)), expected_output)
  instance$exports$deallocate(input_pointer, length_of_subject)
  instance$exports$deallocate(output_pointer, 16)
})
