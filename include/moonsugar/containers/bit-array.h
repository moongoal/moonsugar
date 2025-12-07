/**
 * @file
 *
 * Array of bits packed into 64 bit integers.
 */
#ifndef MS_CONTAINERS_BIT_ARRAY_H
#define MS_CONTAINERS_BIT_ARRAY_H

#include <moonsugar/api.h>
#include <moonsugar/memory.h>

typedef struct {
  ms_allocator allocator;

  /**
   * The bit map.
   *
   * Each integer uses all 64 bits, with the LSB
   * being the one with the lowest index.
   */
  uint64_t * data;

  /**
   * The number of integers contained in `data`.
   */
  uint32_t capacity;

  /**
   * True if the barray automatically resizes to acommodate
   * out of bounds indices.
   */
  bool resizable;
} ms_barray;

typedef struct {
  /**
   * The allocator.
   */
  ms_allocator allocator;

  /**
   * The initial number of bits to allocate capacity for.
   */
  uint32_t initial_capacity;

  /**
   * True if the barray automatically resizes to acommodate
   * out of bounds indices.
   */
  bool resizable;
} ms_barray_description;

/**
 * Construct a new bit array.
 *
 * @param barray The barray to construct.
 * @param description The description.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY on memory allocation failure
 *  - MS_RESULT_INVALID_ARGUMENT if heap is NULL
 */
ms_result MSAPI ms_barray_construct(
  ms_barray * const barray,
  ms_barray_description const * const description
);

/**
 * Destroy a bit array.
 *
 * @param barray The barray.
 */
void MSAPI ms_barray_destroy(ms_barray * const barray);

/**
 * Set a bit.
 *
 * @param barray The barray.
 * @param index The index of the bit to set.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if reallocation fails on growth
 *  - MS_RESULT_INVALID_ARGUMENT if growth is 0 and the index is out of bounds
 */
ms_result MSAPI ms_barray_set(ms_barray * const barray, uint32_t const index);

/**
 * Clear a bit.
 *
 * @param barray The barray.
 * @param index The index of the bit to clear.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if reallocation fails on growth
 *  - MS_RESULT_INVALID_ARGUMENT if growth is 0 and the index is out of bounds
 */
ms_result MSAPI ms_barray_clear(ms_barray * const barray, uint32_t const index);

/**
 * Clear all bits.
 *
 * @param barray The barray to clear.
 */
void MSAPI ms_barray_clear_all(ms_barray * const barray);

/**
 * Resize a barray.
 *
 * @param barray The barray.
 * @param new_capacity The new number of bits to allocate capacity for.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if reallocation fails on growth
 */
ms_result MSAPI ms_barray_resize(ms_barray * const barray, uint32_t const new_capacity);

/**
 * Get the value of a bit. This function assumes the bit index is within bounds.
 *
 * @param barray The barray.
 * @param index The index of the bit to get.
 *
 * @return True if the bit is set, false if not.
 */
bool MSAPI ms_barray_get(ms_barray * const barray, uint32_t const index);

#endif // MS_CONTAINERS_BIT_ARRAY_H
