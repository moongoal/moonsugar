/*
 * A dynamically resizable array.
 */
#ifndef MS_CONTAINERS_AUTO_ARRAY_H
#define MS_CONTAINERS_AUTO_ARRAY_H

#include <moonsugar/api.h>
#include <moonsugar/memory.h>

typedef struct {
  ms_allocator allocator;
  void *base;
  uint32_t item_size;
  uint32_t capacity;
  uint32_t count;
} ms_autoarray;

typedef struct {
  ms_allocator allocator;
  uint32_t item_size;
  uint32_t initial_capacity;
} ms_autoarray_description;

ms_result MSAPI ms_autoarray_construct(ms_autoarray * const arr, ms_autoarray_description const * const description);
void MSAPI ms_autoarray_destroy(ms_autoarray * const arr);
ms_result MSAPI ms_autoarray_resize(ms_autoarray * const array, uint32_t const new_count); // Change count
ms_result MSAPI ms_autoarray_reserve(ms_autoarray * const array, uint32_t new_capacity); // Change capacity - new capacity will be rounded to the next multiple of 8
void* MSAPI MSUSERET ms_autoarray_get(ms_autoarray * const arr, uint32_t const index); // Get pointer to item
void* MSAPI MSUSERET ms_autoarray_append(ms_autoarray * const arr); // Increase count by 1 and get pointer to last item

#endif // MS_CONTAINERS_AUTO_ARRAY_H
