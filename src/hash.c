#include <string.h>
#include <xxh3.h>
#include <moonsugar/assert.h>
#include <moonsugar/hash.h>

uint64_t ms_hash(void const *const data, uint64_t const size) {
  MS_ASSERT(data);
  return XXH3_64bits(data, size);
}

uint64_t ms_hash_u64(void const * const value) {
  return *(uint64_t*)value;
}

uint64_t ms_hash_u32(void const * const value) {
  return *(uint32_t*)value;
}

uint64_t ms_hash_cstr(void const * const s_ptr) {
  char const * const s = *(char const**)s_ptr;
  uint32_t const len = strlen(s);

  return ms_hash(s, len * sizeof(char));
}

