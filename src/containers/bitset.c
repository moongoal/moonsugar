#include <memory.h>
#include <moonsugar/assert.h>
#include <moonsugar/util.h>
#include <moonsugar/containers.h>

#define DIV64(x) ((x) >> 6)
#define MOD64(x) ((x) & 0x3f)
#define MUL64(x) ((x) << 6)

ms_result ms_bitset_construct(
  ms_bitset * const restrict bitset,
  ms_bitset_description const * const restrict description
) {
  MS_ASSERT(bitset);
  MS_ASSERT(description);

  uint32_t const initial_capacity = ms_align_sz(description->initial_capacity, 64);

  *bitset = (ms_bitset) {
    description->allocator,
    NULL,
    0,
    description->resizable
  };

  if(initial_capacity > 0) {
    MS_CKRET(ms_bitset_resize(bitset, initial_capacity));
  }

  return MS_RESULT_SUCCESS;
}

void ms_bitset_destroy(ms_bitset * const restrict bitset) {
  MS_ASSERT(bitset);

  ms_free(&bitset->allocator, bitset->data);
  bitset->data = NULL;
}

ms_result ms_bitset_set(ms_bitset * const restrict bitset, uint32_t const index) {
  MS_ASSERT(bitset);

  uint32_t const cluster_index = DIV64(index);
  uint32_t const cluster_offset = MOD64(index);

  if(cluster_index >= bitset->capacity) {
    if(bitset->resizable) {
      MS_CKRET(ms_bitset_resize(bitset, MUL64(cluster_index + 1)));
    } else {
      return MS_RESULT_INVALID_ARGUMENT;
    }
  }

  bitset->data[cluster_index] = ms_set(bitset->data[cluster_index], 1U << cluster_offset);

  return MS_RESULT_SUCCESS;
}

ms_result ms_bitset_clear(ms_bitset * const restrict bitset, uint32_t const index) {
  MS_ASSERT(bitset);

  uint32_t const cluster_index = DIV64(index);
  uint32_t const cluster_offset = MOD64(index);

  if(cluster_index >= bitset->capacity) {
    if(bitset->resizable) {
      MS_CKRET(ms_bitset_resize(bitset, MUL64(cluster_index + 1)));
    } else {
      return MS_RESULT_INVALID_ARGUMENT;
    }
  }

  bitset->data[cluster_index] = ms_clear(bitset->data[cluster_index], 1U << cluster_offset);

  return MS_RESULT_SUCCESS;
}

void ms_bitset_clear_all(ms_bitset * const restrict bitset) {
  MS_ASSERT(bitset);
  
  for(uint32_t i = 0; i < bitset->capacity; ++i) {
    bitset->data[i] = 0;
  }
}

ms_result ms_bitset_resize(ms_bitset * const restrict bitset, uint32_t const new_capacity_bits) {
  MS_ASSERT(bitset);

  uint32_t const new_capacity = DIV64(new_capacity_bits);
  uint32_t const old_capacity = bitset->capacity;
  int64_t const capacity_delta = new_capacity - old_capacity;

  bitset->data = ms_realloc(&bitset->allocator, bitset->data, sizeof(uint64_t) * new_capacity);

  if(bitset->data == NULL && new_capacity > 0) {
    return MS_RESULT_MEMORY;
  }

  if(capacity_delta > 0) {
    memset(bitset->data + old_capacity, 0, sizeof(uint64_t) * capacity_delta);
  }

  bitset->capacity = new_capacity;

  return MS_RESULT_SUCCESS;
}

bool ms_bitset_get(ms_bitset * const restrict bitset, uint32_t const index) {
  MS_ASSERT(bitset);

  uint32_t const cluster_index = DIV64(index);
  uint32_t const cluster_offset = MOD64(index);

  return ms_test(bitset->data[cluster_index], 1U << cluster_offset);
}

