#include <string.h>
#include <moonsugar/assert.h>
#include <moonsugar/util.h>
#include <moonsugar/containers/auto-array.h>

ms_result ms_autoarray_construct(ms_autoarray * const arr, ms_autoarray_description const * const description) {
  MS_ASSERT(arr);
  MS_ASSERT(description);

  uint32_t const capacity = ms_align_sz(description->initial_capacity, 8);

  *arr = (ms_autoarray) {
    description->allocator,
    ms_malloc(
      &description->allocator,
      description->item_size * capacity,
      MS_DEFAULT_ALIGNMENT
    ),
    description->item_size,
    capacity,
    0
  };

  return MS_RESULT_SUCCESS;
}

void ms_autoarray_destroy(ms_autoarray * const arr) {
  MS_ASSERT(arr);

  ms_free(&arr->allocator, arr->base);
  memset(arr, 0, sizeof(ms_autoarray));
}

ms_result ms_autoarray_resize(ms_autoarray * const array, uint32_t const new_count) {
  MS_CKRET(ms_autoarray_reserve(array, new_count));

  array->count = new_count;

  return MS_RESULT_SUCCESS;
}

ms_result ms_autoarray_reserve(ms_autoarray * const array, uint32_t new_capacity) {
  MS_ASSERT(array);

  new_capacity = ms_align_sz(new_capacity, 8);

  if(new_capacity > array->capacity) {
    array->base = ms_realloc(
      &array->allocator,
      array->base,
      array->item_size * new_capacity
    );

    array->capacity = new_capacity;

    return ms_choose(MS_RESULT_SUCCESS, MS_RESULT_MEMORY, array->base != NULL && new_capacity > 0);
  }

  return MS_RESULT_SUCCESS;
}

void* ms_autoarray_get(ms_autoarray * const arr, uint32_t const index) {
  return (uint8_t*)arr->base + arr->item_size * index;
}

void* ms_autoarray_append(ms_autoarray * const arr) {
  if(arr->count == arr->capacity) {
    ms_autoarray_reserve(arr, arr->capacity * 2);
  }

  arr->count += 1;

  return ms_autoarray_get(arr, arr->count - 1);
}
