/**
 *
 * @file
 *
 * Memory interface.
 */
#ifndef MS_MEMORY_H
#define MS_MEMORY_H

#include <moonsugar/api.h>

#define MS_DEFAULT_ALIGNMENT (8ULL)
#define MS_HEAP_DEALLOC_THR (4194304ull)

#define MS_ALLOCATOR_INIT(prefix, user) (ms_allocator) { \
  prefix ## malloc, \
  prefix ## free, \
  prefix ## realloc, \
  (user) \
}

#define MS_ALLOCATOR_DEF_HEAP(heap) (ms_allocator) { \
  (ms_malloc_clbk)ms_heap_malloc, \
  (ms_free_clbk)ms_heap_free, \
  (ms_realloc_clbk)ms_heap_realloc, \
  &(heap) \
}

#define MS_ALLOCATOR_DEF_ARENA(arena) (ms_allocator) { \
  (ms_malloc_clbk)ms_arena_malloc, \
  (ms_free_clbk)ms_arena_free, \
  (ms_realloc_clbk)ms_arena_realloc, \
  &(arena) \
}

/**
 * Aligned memory allocation callback.
 * 
 * @param user The user pointer as stored in the allocator.
 * @param count The number of bytes to allocate. Can be 0.
 * @param alignment The alignment boundary, in bytes. This value must
 *  always be a power of two.
 *
 * @return A pointer to the allocated memory or NULL on failure.
 *  NULL is also returned if `count == 0`.
 */
typedef void* (*ms_malloc_clbk)(void * const user, size_t const count, size_t const alignment);

/**
 * Memory deallocation callback.
 * 
 * @param user The user pointer as stored in the allocator.
 * @param ptr A pointer to the base of the memory block to free.
 */
typedef void (*ms_free_clbk)(void * const user, void * const ptr);

/**
 * Memory deallocation callback.
 * 
 * @param user The user pointer as stored in the allocator.
 * @param ptr A pointer to the base of the memory block to reallocate.
 * @param new_count The number of bytes to reallocate. Can be 0.
 *
 * @return A pointer to the reallocated memory. The pointer will be `ptr`
 *  if no new memory was allocated. The result will be NULL on failure or
 *  if `new_count == 0`. The returned memory block will have the same
 *  alignment constraint as the original block.
 */
typedef void* (*ms_realloc_clbk)(void * const user, void * const ptr, size_t const new_count);

typedef struct {
  /**
   * Aligned memory allocator.
   */
  ms_malloc_clbk allocate;

  /**
   * Memory deallocator.
   */
  ms_free_clbk deallocate;

  /**
   * Memory reallocator.
   */
  ms_realloc_clbk reallocate;

  /**
   * User data pointer. Set to NULL if unused.
   */
  void * user;
} ms_allocator;

/**
 * Allocate aligned memory.
 *
 * @see ms_malloca_clbk
 */
MSUSERET void* MSAPI ms_malloc(ms_allocator const * const allocator, size_t const size, size_t const alignment);

/**
 * Reallocate memory.
 *
 * @see ms_realloc_clbk
 */
MSUSERET void* MSAPI ms_realloc(ms_allocator const * const allocator, void * const ptr, size_t const new_size);

/**
 * Deallocate memory.
 *
 * @see ms_free_clbk
 */
void MSAPI ms_free(ms_allocator const * const allocator, void * const ptr);

typedef struct {
  /**
   * Total allocation size (including any metadata and padding). This
   * can be greater than the requested allocation size.
   */
  uint64_t size;

  /**
   * Padding, in bytes, added before the header to ensure allocation
   * alignment.
   */
  uint32_t padding;

  /**
   * Alignment constraint as provided during allocation.
   */
  uint32_t alignment;
} ms_header;

/**
 * A chunk of contiguous memory allocated from the heap.
 */
typedef struct ms_free_list_node ms_free_list_node;
typedef struct ms_free_list ms_free_list;

/**
 * Callback invoked before a node is created.
 *
 * @param list The free list.
 * @param ptr A pointer to the memory where the node will be created.
 * @param size The size in bytes of the node being created.
 * @param user The user value.
 */
typedef void (*ms_free_list_on_before_node_create_clbk)(
  ms_free_list * const list,
  void * const ptr,
  size_t const size,
  void * const user
);

/**
 * Callback invoked before memory is allocated from one node.
 *
 * @param list The free list.
 * @param node The node memory will be allocated from.
 * @param size The size in bytes of the memory being allocated.
 * @param user The user value.
 */
typedef void (*ms_free_list_on_before_alloc_from_node_clbk)(
  ms_free_list * const list,
  ms_free_list_node * const node,
  size_t const size,
  void * const user
);

struct ms_free_list {
  ms_free_list_node *first;
  void *user;
  ms_free_list_on_before_node_create_clbk on_before_node_create;
  ms_free_list_on_before_alloc_from_node_clbk on_before_alloc_from_node;
};

struct ms_free_list_node {
  /**
   * Next and previous chunks.
   *
   * When `next == NULL` this is the last chunk.
   * When `prev == NULL` this is the first chunk.
   */
  ms_free_list_node *next, *prev;

  /**
   * Size in bytes of this chunk.
   */
  uint64_t size;
};

/**
 * Allocate memory from a free list.
 *
 * @param list The list to allocate from.
 * @param count The number of bytes to allocate.
 * @param alignment The alignment boundary of the allocation.
 * @param out_allocated_size Output total allocated size.
 *
 * @return The pointer to the allocated memory or NULL on failure.
 */
void * MSAPI ms_free_list_malloc(
  ms_free_list *const list,
  size_t const count,
  uint32_t const alignment,
  size_t *const out_allocated_size
);

/**
 * Free memory from a node.
 *
 * @param list The free list the node was allocated from.
 * @param ptr The pointer to the memory allocated via `ms_free_list_malloc()`.
 * @param size The total amount of memory being deallocated, in bytes.
 */
void MSAPI ms_free_list_free(
  ms_free_list *const list,
  void * const ptr,
  size_t const size
);

/**
 * Create a new free list node.
 *
 * @param list The free list.
 * @param node The node being created.
 * @param prev The previous node.
 * @param next The next node.
 * @param size The size of the node, in bytes.
 */
void MSAPI ms_free_list_create_node(
  ms_free_list* const list,
  ms_free_list_node *const node,
  ms_free_list_node *const prev,
  ms_free_list_node *const next,
  uint64_t const size
);

/**
 * A heap of global memory, shared across the entire process.
 * Heaps are paged and every allocation occupies the minimum amount
 * of pages required to satisfy it. The heap size must always be a
 * multiple of the number of pages.
 *
 * Memory contained by this heap will be reserved but uncommitted
 * when not allocated, and committed only during allocation.
 *
 * De-allocated memory will be de-committed.
 */
typedef struct {
  /**
   * Base address of heap.
   */
  void *base;

  /**
   * Size of heap, in bytes.
   */
  uint64_t size;

  /**
   * Commit page size.
   */
  uint64_t commit_page_size;

  /**
   * Size of committed memory, in bytes.
   *
   * A chunk may be larger than its committed size and grow/shrink
   * whenever necessary to keep the balance between allocation performance
   * and resource usage.
   */
  uint64_t committed_size;

  /**
   * List of free blocks of memory.
   */
  ms_free_list free_list;
} ms_heap;

MSAPI ms_result ms_heap_construct(ms_heap *const heap, uint64_t const size, uint64_t const page_size);
MSAPI void ms_heap_destroy(ms_heap *const heap);
MSAPI MSUSERET void * ms_heap_malloc(ms_heap *const heap, size_t const count, size_t const alignment); // Returns NULL on failure
MSAPI MSUSERET void * ms_heap_realloc(ms_heap *const heap, void *const ptr, size_t const new_count);
MSAPI void ms_heap_free(ms_heap *const heap, void *const ptr);
MSUSERET MSAPI ms_header *ms_heap_get_header(void *const ptr);
MSAPI MSUSERET bool ms_heap_owns(ms_heap *const heap, void *const ptr);

/*
 * Stack
 */

typedef struct {
  void *base; // Base stack memory address
  size_t size; // Stack memory size
  MS_ATOMIC(uint8_t*) top; // Top stack pointer
  MS_ATOMIC(uint8_t*) committed_top; // Top of committed memory
} ms_stack;

MSAPI ms_result ms_stack_construct(ms_stack * const stack, uint64_t const max_size);
MSAPI void ms_stack_destroy(ms_stack * const stack);
MSAPI void* ms_stack_malloc(ms_stack * const stack, size_t size, size_t const alignment);
MSAPI void ms_stack_clear(ms_stack * const stack); // Reset the stack to empty state

/*
 * Arena
 *
 * An arena is a pre-allocated, disposable area of memory
 * intended to centralise and speed-up the sub-allocation
 * of related resources.
 *
 * Arenas are of fixed size but can be chained. Chained
 * arenas will automatically be deallocated when they
 * become empty. Each chained arena size is aligned
 * to a multiple of the primary arena.
 */

typedef struct ms_arena ms_arena;
typedef struct ms_arena_node ms_arena_node;

struct ms_arena_node {
  ms_free_list free_list;
  uint64_t total_size;
  uint64_t allocated_size;
  ms_arena_node *next;
  uint8_t base[];
};

typedef enum {
  MS_ARENA_STICKY_BIT = 1 // Do not release empty nodes on deallocation
} ms_arena_flag_bits;

typedef uint32_t ms_arena_flags;

struct ms_arena {
  uint64_t base_size;
  ms_arena_node *first;
  ms_allocator allocator;
  ms_arena_flags flags;
};

typedef struct {
  uint64_t base_size;
  ms_allocator allocator;
  ms_arena_flags flags;
} ms_arena_description;

MSAPI void ms_arena_construct(ms_arena *const arena, ms_arena_description const * const description);
MSAPI void ms_arena_destroy(ms_arena *const arena);
MSAPI MSUSERET void * ms_arena_malloc(ms_arena *const arena, size_t const count, size_t const alignment); // Returns the pointer; NULL on failure or when count is 0
MSAPI MSUSERET void * ms_arena_realloc(ms_arena *const arena, void *const ptr, size_t const new_count); // Returns the new pointer; NULL on failure of if new_count is 0
MSAPI void ms_arena_free(ms_arena *const arena, void *const ptr);
ms_header * MSAPI ms_arena_get_header(void *const ptr); // Get allocation header
MSAPI void ms_arena_clear(ms_arena *const arena); // Reset arena to empty, invalidating all previous allocations

/*
 * OS memory interface.
 */

MSAPI void ms_release(void * const ptr, const size_t count); // Release reserved memory to the OS
MSUSERET MSAPI MSMALLOC void* ms_reserve(const size_t count); // Reserve memory - Returns the pointer to the base of the reserved block or NULL on failure
MSAPI bool ms_commit(void * const ptr, const size_t count); // Commit reserved memory - Returns false on failure
MSAPI void ms_decommit(void * const ptr, const size_t count); // Decommit committed memory

#endif // MS_MEMORY_H
