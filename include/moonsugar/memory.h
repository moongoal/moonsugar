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
   * Usable allocation size. This can be greater than the requested
   * allocation size.
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
  ms_free_list *restrict const list,
  size_t const count,
  uint32_t const alignment,
  size_t *restrict const out_allocated_size
);

/**
 * Free memory from a node.
 *
 * @param list The free list the node was allocated from.
 * @param ptr The pointer to the memory allocated via `ms_free_list_malloc()`.
 * @param size The total amount of memory being deallocated, in bytes.
 */
void MSAPI ms_free_list_free(
  ms_free_list *restrict const list,
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
  ms_free_list_node *restrict const node,
  ms_free_list_node *restrict const prev,
  ms_free_list_node *restrict const next,
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

/**
 * Construct a new heap, requesting memory from the OS.
 *
 * @param heap The heap to construct.
 * @param size The heap size.
 * @param page_size The allocation page size.
 *
 * @return The new heap.
 */
MSAPI void ms_heap_construct(ms_heap *const heap, uint64_t const size, uint64_t const page_size);

/**
 * Destroy an existing heap.
 *
 * @param heap The heap to destroy.
 */
MSAPI void ms_heap_destroy(ms_heap *const heap);
/**
 * Allocate memory from the heap.
 *
 * @param heap The heap.
 * @param count The number of bytes to allocate.
 * @param alignment Alignment constraint.
 *
 * @return A pointer to the allocated memory or NULL
 *  on failure.
 */
MSAPI MSUSERET void * ms_heap_malloc(ms_heap *const heap, size_t const count, size_t const alignment);

/**
 * Re-allocate memory from the heap.
 *
 * @param heap The heap.
 * @param new_count The new number of bytes to allocate.
 *
 * @return A pointer to the re-allocated memory or NULL
 *  on failure. The returned pointer may be the same as
 *  `ptr` if no memory relocation is necessary, `ptr` is returned;
 *  if memory relocation is necessary, the existing data is copied.
 */
MSAPI MSUSERET void * ms_heap_realloc(ms_heap *const heap, void *const ptr, size_t const new_count);

/**
 * De-allocate previously allocaetd memory.
 *
 * @param heap The heap.
 * @param ptr The pointer to the memory to free,
 *  as returned from `allocate()`.
 */
MSAPI void ms_heap_free(ms_heap *const heap, void *const ptr);

/**
 * Get the allocation header for a given pointer.
 *
 * @param ptr The pointer to get the allocation header for.
 *  The pointer must be as returned from one of the heap
 *  allocation functions. This argument is assumed to never
 *  be NULL.
 *
 * @return A pointer to the header of the allocation.
 */
MSUSERET MSAPI ms_header *ms_heap_get_header(void *const ptr);

/**
 * Test ownership of a pointer.
 *
 * @param heap The heap.
 * @param ptr The pointer to test for ownership.
 *
 * @return True if the memory referenced by this pointer
 *  was allocated via this heap, false if not.
 */
MSAPI MSUSERET bool ms_heap_owns(ms_heap *const heap, void *const ptr);

typedef struct {
  /**
   * Base stack memory address.
   */
  void *base;

  /**
   * Stack memory size.
   */
  size_t size;

  /**
   * Top stack pointer.
   */
  MS_ATOMIC(uint8_t*) top;

  /**
   * Top committed memory pointer.
   */
  MS_ATOMIC(uint8_t*) committed_top;
} ms_stack;

/**
 * Construct a stack allocator.
 *
 * @param stack The allocator.
 * @param max_size Maximum amount of memory allocatable.
 */
MSAPI void ms_stack_construct(ms_stack * const restrict stack, uint64_t const max_size);

/**
 * Destroy a stack allocator.
 *
 * @param stack The allocator.
 */
MSAPI void ms_stack_destroy(ms_stack * const restrict stack);

/**
 * Allocate memory from a stack allocator.
 *
 * @param stack The allocator.
 * @param size The size of the allocator, in bytes.
 * @param alignment The alignment boundary
 *
 * @return A pointer to the allocated memory.
 */
MSAPI void* ms_stack_malloc(
  ms_stack * const restrict stack,
  size_t size,
  size_t const alignment
);

/**
 * Clear a stack allocator.
 *
 * @param stack The allocator.
 */
MSAPI void ms_stack_clear(ms_stack * const restrict stack);

typedef struct ms_arena ms_arena;

struct ms_arena {
  ms_free_list free_list;
  void *base;
  uint64_t size;
  ms_arena *next;
  ms_allocator *allocator;
};

/**
 * Construct a new arena, requesting memory from the OS.
 *
 * @param arena The arena to construct.
 * @param size The arena size.
 * @param base_ptr The base memory pointer.
 *
 * @return The new arena.
 */
MSAPI void ms_arena_construct(ms_arena *const arena, uint64_t const size, void * const base_ptr);

/**
 * Destroy an existing arena.
 *
 * @param arena The arena to destroy.
 */
MSAPI void ms_arena_destroy(ms_arena *const arena);

/**
 * Allocate memory from the arena.
 *
 * @param arena The arena.
 * @param count The number of bytes to allocate.
 * @param alignment Alignment constraint.
 *
 * @return A pointer to the allocated memory or NULL
 *  on failure.
 */
MSAPI MSUSERET void * ms_arena_malloc(ms_arena *const arena, size_t const count, size_t const alignment);

/**
 * Re-allocate memory from the arena.
 *
 * @param arena The arena.
 * @param new_count The new number of bytes to allocate.
 *
 * @return A pointer to the re-allocated memory or NULL
 *  on failure. The returned pointer may be the same as
 *  `ptr` if no memory relocation is necessary, `ptr` is returned;
 *  if memory relocation is necessary, the existing data is copied.
 */
MSAPI MSUSERET void * ms_arena_realloc(ms_arena *const arena, void *const ptr, size_t const new_count);

/**
 * De-allocate previously allocaetd memory.
 *
 * @param arena The arena.
 * @param ptr The pointer to the memory to free,
 *  as returned from `allocate()`.
 */
MSAPI void ms_arena_free(ms_arena *const arena, void *const ptr);

#endif // MS_MEMORY_H
