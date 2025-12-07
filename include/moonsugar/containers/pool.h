/**
 * @file
 *
 * Data pool.
 */
#ifndef MS_CONTAINERS_POOL_H
#define MS_CONTAINERS_POOL_H

#include <moonsugar/api.h>
#include <moonsugar/util.h>
#include <moonsugar/memory.h>

typedef struct ms_pool_node ms_pool_node;

struct ms_pool_node {
  /**
   * Pointer to the next available node.
   */
  ms_pool_node *next;

  /**
   * Item data.
   */
  uint8_t data[];
};

typedef struct {
  /**
   * Memory allocator.
   */
  ms_allocator allocator;

  /**
   * Data items.
   */
  ms_pool_node *items;

  /**
   * Array of pages containing the nodes of the pool.
   * Nodes are allocated in pages, each of `nodes_per_page`
   * items.
   */
  void **pages;

  /**
   * Item slot count.
   */
  uint32_t item_count;

  /**
   * Size of one item acquirable via the pool.
   */
  uint32_t item_size;

  /**
   * Number of items allocated per page.
   */
  uint32_t items_per_page;
} ms_pool;

typedef struct {
  /**
   * The allocator.
   */
  ms_allocator allocator;

  /**
   * Size of one item acquirable via the pool.
   */
  uint32_t item_size;

  /**
   * Number of items allocated per page.
   */
  uint32_t items_per_page;
} ms_pool_description;

/**
 * Construct a pool.
 *
 * @param this The pool.
 * @param desc The pool description.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_INVALID_ARGUMENT if any count or size value is 0
 *  - MS_RESULT_HOST_MEMORY if memory allocation failed.
 */
MSAPI ms_result ms_pool_construct(ms_pool *const this, ms_pool_description const *const desc);

/**
 * Destroy a pool.
 *
 * @param this The pool.
 */
MSAPI void ms_pool_destroy(ms_pool *const this);

/**
 * Acquire an item from a pool.
 *
 * @param this The pool.
 *
 * @return A pointer to the acquired item.
 */
MSAPI MSUSERET void* ms_pool_acquire(ms_pool *const this);

/**
 * Release a previously acquired item to a pool.
 *
 * @param this The pool the item was acquired from.
 * @param item A pointer to the item to release.
 */
MSAPI void ms_pool_release(ms_pool *const this, void * const item);

#endif // MS_CONTAINERS_POOL_H


