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
  ms_malloc_clbk allocate;
  ms_free_clbk deallocate;
  ms_realloc_clbk reallocate;
  void * user; // User data pointer
} ms_allocator;

MSUSERET void* MSAPI ms_malloc(ms_allocator const * const allocator, size_t const size, size_t const alignment);
MSUSERET void* MSAPI ms_realloc(ms_allocator const * const allocator, void * const ptr, size_t const new_size);
void MSAPI ms_free(ms_allocator const * const allocator, void * const ptr);

typedef struct {
  uint64_t size; // Total size (including metadata and padding)
  uint32_t padding; // Padding added before the header to ensure alignment
  uint32_t alignment;
} ms_header;

typedef struct ms_free_list_node ms_free_list_node; // Chunk of contiguous free memory
typedef struct ms_free_list ms_free_list;

typedef void (*ms_free_list_on_before_node_create_clbk)(
  ms_free_list * const list,
  void * const ptr, // Pointer to the memory that will host the node
  size_t const size, // Size of the memory hosted by the node
  void * const user
); // Invoked before a node is created

typedef void (*ms_free_list_on_before_alloc_from_node_clbk)(
  ms_free_list * const list,
  ms_free_list_node * const node,
  size_t const size,
  void * const user
); // Invoked before memory is allocate from a node

struct ms_free_list {
  ms_free_list_node *first;
  void *user;
  ms_free_list_on_before_node_create_clbk on_before_node_create;
  ms_free_list_on_before_alloc_from_node_clbk on_before_alloc_from_node;
};

struct ms_free_list_node {
  ms_free_list_node *next, *prev; // NULL = no next/prev
  uint64_t size;
};

void * MSAPI ms_free_list_malloc(
  ms_free_list *const list,
  size_t const count, // Bytes to allocate
  uint32_t const alignment,
  size_t *const out_allocated_size // Total allocated size
);

void MSAPI ms_free_list_free(
  ms_free_list *const list,
  void * const ptr,
  size_t const size
);

void MSAPI ms_free_list_create_node(
  ms_free_list* const list,
  ms_free_list_node *const node,
  ms_free_list_node *const prev,
  ms_free_list_node *const next,
  uint64_t const size
);

typedef struct {
  void *base;
  uint64_t size;
  uint64_t commit_page_size; // Size of the memory commit unit
  uint64_t committed_size;
  ms_free_list free_list;
} ms_heap;

MSAPI ms_result ms_heap_construct(
  ms_heap *const heap,
  uint64_t const size, // Must be multiple of the page size
  uint64_t const page_size // Must be a power of two
);

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
  uint64_t base_size; // Initial arena size
  ms_allocator allocator; // Allocator the arena draws memory from
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
