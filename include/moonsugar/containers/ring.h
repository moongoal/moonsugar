/**
 * @file
 *
 * Ring buffer.
 */
#ifndef MS_CONTAINERS_RING_H
#define MS_CONTAINERS_RING_H

#include <moonsugar/api.h>
#include <moonsugar/memory.h>

/**
 * A ring buffer.
 */
typedef struct {
  /**
   * The count of items currently stored in the buffer.
   */
  uint32_t count;

  /**
   * The write index.
   */
  uint32_t windex;

  /**
   * Maximum number of items.
   */
  uint32_t capacity;

  /**
   * Individual item size.
   */
  uint32_t item_size;

  /**
   * Values store.
   */
  void *values;

  /**
   * Memory allocator.
   */
  ms_allocator allocator;
} ms_ring;

typedef struct {
  /**
   * Maximum number of items.
   */
  uint32_t capacity;

  /**
   * Size of one item, in bytes.
   */
  uint32_t item_size;

  /**
   * Memory allocator.
   */
  ms_allocator allocator;
} ms_ring_description;

/**
 * Construct a ring.
 *
 * @param this The ring.
 * @param capacity The maximum number of items that
 *  can be stored in the ring data buffer.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if buffer allocation failed
 *  - MS_RESULT_INVALID_ARGUMENT if capacity is 0 or not a power of 2
 *    or if the item size is 0
 */
MSAPI ms_result
ms_ring_construct(ms_ring *const this, ms_ring_description const *const description);

/**
 * Destroy a ring.
 *
 * @param this The ring.
 */
MSAPI void ms_ring_destroy(ms_ring *const this);

/**
 * Enqueue an item.
 *
 * @param this The ring.
 *
 * @return The pointer to the value in the ring or NULL if the ring is full.
 */
MSAPI MSUSERET void *ms_ring_enqueue(ms_ring *const this);

/**
 * Dequeue an item.
 *
 * @param this The ring.
 *
 * @return The pointer to the value in the ring or NULL if the ring is empty.
 */
MSAPI void *ms_ring_dequeue(ms_ring *const this);

#endif // MS_CONTAINERS_RING_H


