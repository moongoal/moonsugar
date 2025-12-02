/**
 * @file
 *
 * Data containers.
 */
#ifndef MS_CONTAINERS_H
#define MS_CONTAINERS_H

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
   * True if the bitset automatically resizes to acommodate
   * out of bounds indices.
   */
  bool resizable;
} ms_bitset;

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
   * True if the bitset automatically resizes to acommodate
   * out of bounds indices.
   */
  bool resizable;
} ms_bitset_description;

/**
 * Construct a new bit set.
 *
 * @param bitset The bitset to construct.
 * @param description The description.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY on memory allocation failure
 *  - MS_RESULT_INVALID_ARGUMENT if heap is NULL
 */
ms_result MSAPI ms_bitset_construct(
  ms_bitset * const restrict bitset,
  ms_bitset_description const * const restrict description
);

/**
 * Destroy a bit set.
 *
 * @param bitset The bitset.
 */
void MSAPI ms_bitset_destroy(ms_bitset * const restrict bitset);

/**
 * Set a bit.
 *
 * @param bitset The bitset.
 * @param index The index of the bit to set.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if reallocation fails on growth
 *  - MS_RESULT_INVALID_ARGUMENT if growth is 0 and the index is out of bounds
 */
ms_result MSAPI ms_bitset_set(ms_bitset * const restrict bitset, uint32_t const index);

/**
 * Clear a bit.
 *
 * @param bitset The bitset.
 * @param index The index of the bit to clear.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if reallocation fails on growth
 *  - MS_RESULT_INVALID_ARGUMENT if growth is 0 and the index is out of bounds
 */
ms_result MSAPI ms_bitset_clear(ms_bitset * const restrict bitset, uint32_t const index);

/**
 * Clear all bits.
 *
 * @param bitset The bitset to clear.
 */
void MSAPI ms_bitset_clear_all(ms_bitset * const restrict bitset);

/**
 * Resize a bitset.
 *
 * @param bitset The bitset.
 * @param new_capacity The new number of bits to allocate capacity for.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if reallocation fails on growth
 */
ms_result MSAPI ms_bitset_resize(ms_bitset * const restrict bitset, uint32_t const new_capacity);

/**
 * Get the value of a bit. This function assumes the bit index is within bounds.
 *
 * @param bitset The bitset.
 * @param index The index of the bit to get.
 *
 * @return True if the bit is set, false if not.
 */
bool MSAPI ms_bitset_get(ms_bitset * const restrict bitset, uint32_t const index);

#endif // MS_CONTAINERS_H
