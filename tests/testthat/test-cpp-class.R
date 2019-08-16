test_that("calling functions work", {
  path <- "../../inst/examples/sum.wasm"
  f <- file(path, "rb")
  bytes <- readBin(f, what = "raw", n = file.size(path))
  close(f)
  mod <- new(WasmModule)
  mod$instantiate(bytes)
  funs <- mod$get_exported_functions()
  expect_equal(length(funs), 1)
  fun <- funs[[1]]
  expect_equal(fun[["name"]], "sum")
  fun_result <- mod$call_exported_function("sum", list(1, 5))
  expect_equal(fun_result[[1]], 6)
})

test_that("we can access the memory", {
  path <- "../../inst/examples/hello.wasm"
  f <- file(path, "rb")
  bytes <- readBin(f, what = "raw", n = file.size(path))
  close(f)
  mod <- new(WasmModule)
  mod$instantiate(bytes)
  funs <- mod$get_exported_functions()
  pointer <- mod$call_exported_function("hello", list())
  memory <- mod$get_memory_view(pointer[[1]])
  expect_equal(rawToChar(memory), "Hello world")
})
