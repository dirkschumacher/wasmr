#include <stdint.h>
#include "../wasmer.h"

#ifdef __cplusplus
extern "C" {
#endif

void wasmer_memory_write_u8(
    wasmer_memory_t* mem,
    uint32_t offset,
    const uint32_t* indexes,
    size_t indexes_length,
    const uint8_t* data,
    size_t data_length
);

#ifdef __cplusplus
}
#endif
