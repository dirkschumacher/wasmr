extern crate wasmer_runtime;
extern crate wasmer_runtime_c_api;
extern crate libc;

use wasmer_runtime::Memory;

#[repr(C)]
pub struct wasmer_memory_t;

#[no_mangle]
pub extern "C" fn wasmer_memory_write_u8(
  mem: *const wasmer_memory_t,
  offset: u32,
  indexes: *const u32,
  indexes_length: usize,
  data: *const u8,
  data_length: usize) -> ()  {
  let memory = unsafe { &*(mem as *const Memory) };
  let slice_data = unsafe { std::slice::from_raw_parts(data, data_length) };
  let slice_indexes = unsafe { std::slice::from_raw_parts(indexes, indexes_length) };
  let view = memory.view::<u8>();
  let offset = offset as usize;
  for (index, value) in slice_indexes.iter().zip(slice_data.iter()) {
    let index = *index as usize;
    let value = *value;
    view[offset + index].set(value);
  }
}
