library(wasmr)
wasm_url <- "https://github.com/syrusakbary/fastdiff/blob/master/python/fastdiff/fastdiff.wasm?raw=true"
f <- tempfile()
download.file(wasm_url, f)
instance <- instantiate(f)

str1 <- charToRaw("hello\nwasm\n")
str2 <- charToRaw("hello\npython\n")

str1_len <- length(str1)
str2_len <- length(str2)

# first we need to allocate memory for the two strings
ptr1 <- instance$exports$allocate(str1_len)
ptr2 <- instance$exports$allocate(str2_len)

# then we write the strings to the shared memory
instance$memory$get_memory_view(ptr1)$set(seq_along(str1), str1)
instance$memory$get_memory_view(ptr2)$set(seq_along(str2), str2)

# the compare function now reads the data from the wasm memory
out_ptr <- instance$exports$compare(ptr1, ptr2)
result <- rawToChar(instance$memory$get_memory_view(out_ptr)$get(1:28))
cat(result)
#>   hello
#> - wasm
#> + python
#>

# now dealocate memory
deallocate <- instance$exports$deallocate
deallocate(ptr1, str1_len)
#> list()
deallocate(ptr2, str2_len)
#> list()
deallocate(out_ptr, 28)
#> list()
