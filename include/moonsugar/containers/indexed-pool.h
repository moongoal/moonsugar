/**
 * @file
 *
 * A pool that manages indices.
 *
 * Differently from the pool component, an indexed pool (ipool) doesn't
 * own items, only metadata. Acquiring an item in an ipool results in obtaining
 * its index. The user is responsible for managing the lifetime of any associated
 * memory. This component is primarily intended to generate handles and assist
 * in managing their associated memory.
 */
#ifndef MS_CONTAINERS_POOL_H
#define MS_CONTAINERS_POOL_H

#include <moonsugar/api.h>
#include <moonsugar/util.h>
#include <moonsugar/memory.h>

#define MS_IPOOL_STATE_FULL (UINT64_MAX)

typedef uint64_t ms_ipool_state_block;

typedef struct {
  ms_ipool_state_block *state; // Item state bitmap
  uint32_t item_count; // Number of item slots
  uint_fast32_t last_state; // Last released item slot index
  ms_allocator allocator;
} ms_ipool;

typedef struct {
  ms_allocator allocator;
  uint32_t capacity; // Capacity of the pool, must be a multiple of 64
} ms_ipool_description;

MSAPI ms_result ms_ipool_construct(ms_ipool *const this, ms_ipool_description const *const desc);
MSAPI void ms_ipool_destroy(ms_ipool *const this);
MSAPI MSUSERET uint32_t ms_ipool_acquire(ms_ipool *const this); // Returns UINT32_MAX when full
MSAPI ms_result ms_ipool_release(ms_ipool *const this, uint32_t const item); // MS_RESULT_INVALID_ARGUMENT on double-release
MSAPI ms_result ms_ipool_resize(ms_ipool *const this, uint32_t const new_count); // Count must be a multiple of 64

#endif // MS_CONTAINERS_POOL_H

