#include <memory.h>
#include <moonsugar/assert.h>
#include <moonsugar/util.h>
#include <moonsugar/containers/bit-array.h>

#define DIV64(x) ((x) >> 6)
#define MOD64(x) ((x) & 0x3f)
#define MUL64(x) ((x) << 6)

ms_result ms_barray_construct(
  ms_barray * const restrict barray,
  ms_barray_description const * const restrict description
) {
  MS_ASSERT(barray);
  MS_ASSERT(description);

  uint32_t const initial_capacity = ms_align_sz(description->initial_capacity, 64);

  *barray = (ms_barray) {
    description->allocator,
    NULL,
    0,
    description->resizable
  };

  if(initial_capacity > 0) {
    MS_CKRET(ms_barray_resize(barray, initial_capacity));
  }

  return MS_RESULT_SUCCESS;
}

void ms_barray_destroy(ms_barray * const restrict barray) {
  MS_ASSERT(barray);

  ms_free(&barray->allocator, barray->data);
  barray->data = NULL;
}

ms_result ms_barray_set(ms_barray * const restrict barray, uint32_t const index) {
  MS_ASSERT(barray);

  uint32_t const cluster_index = DIV64(index);
  uint32_t const cluster_offset = MOD64(index);

  if(cluster_index >= barray->capacity) {
    if(barray->resizable) {
      MS_CKRET(ms_barray_resize(barray, MUL64(cluster_index + 1)));
    } else {
      return MS_RESULT_INVALID_ARGUMENT;
    }
  }

  barray->data[cluster_index] = ms_set(barray->data[cluster_index], 1U << cluster_offset);

  return MS_RESULT_SUCCESS;
}

ms_result ms_barray_clear(ms_barray * const restrict barray, uint32_t const index) {
  MS_ASSERT(barray);

  uint32_t const cluster_index = DIV64(index);
  uint32_t const cluster_offset = MOD64(index);

  if(cluster_index >= barray->capacity) {
    if(barray->resizable) {
      MS_CKRET(ms_barray_resize(barray, MUL64(cluster_index + 1)));
    } else {
      return MS_RESULT_INVALID_ARGUMENT;
    }
  }

  barray->data[cluster_index] = ms_clear(barray->data[cluster_index], 1U << cluster_offset);

  return MS_RESULT_SUCCESS;
}

void ms_barray_clear_all(ms_barray * const restrict barray) {
  MS_ASSERT(barray);
  
  for(uint32_t i = 0; i < barray->capacity; ++i) {
    barray->data[i] = 0;
  }
}

ms_result ms_barray_resize(ms_barray * const restrict barray, uint32_t const new_capacity_bits) {
  MS_ASSERT(barray);

  uint32_t const new_capacity = DIV64(new_capacity_bits);
  uint32_t const old_capacity = barray->capacity;
  int64_t const capacity_delta = new_capacity - old_capacity;

  barray->data = ms_realloc(&barray->allocator, barray->data, sizeof(uint64_t) * new_capacity);

  if(barray->data == NULL && new_capacity > 0) {
    return MS_RESULT_MEMORY;
  }

  if(capacity_delta > 0) {
    memset(barray->data + old_capacity, 0, sizeof(uint64_t) * capacity_delta);
  }

  barray->capacity = new_capacity;

  return MS_RESULT_SUCCESS;
}

bool ms_barray_get(ms_barray * const restrict barray, uint32_t const index) {
  MS_ASSERT(barray);

  uint32_t const cluster_index = DIV64(index);
  uint32_t const cluster_offset = MOD64(index);

  return ms_test(barray->data[cluster_index], 1U << cluster_offset);
}

