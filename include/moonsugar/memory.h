/**
 *
 * @file
 *
 * Memory interface.
 */
#ifndef MS_MEMORY_H
#define MS_MEMORY_H

#include <moonsugar/api.h>

/**
 * Memory allocation callback.
 * 
 * @param user The user pointer as stored in the allocator.
 * @param count The number of bytes to allocate. Can be 0.
 *
 * @return A pointer to the allocated memory or NULL on failure.
 *  NULL is also returned if `count == 0`.
 */
typedef void* (*ms_malloc_clbk)(void * const user, size_t const count);

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
typedef void* (*ms_malloca_clbk)(void * const user, size_t const count, size_t const alignment);

/**
 * Memory deallocation callback.
 * 
 * @param user The user pointer as stored in the allocator.
 * @param ptr A pointer to the base of the memory block to deallocate.
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
   * Memory allocator.
   */
  ms_malloc_clbk allocate;

  /**
   * Aligned memory allocator.
   */
  ms_malloca_clbk allocate_aligned;

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
 * Allocate memory.
 *
 * @see ms_malloc_clbk
 */
MSUSERET void* MSAPI ms_malloc(ms_allocator const * const allocator, size_t const size);

/**
 * Allocate aligned memory.
 *
 * @see ms_malloca_clbk
 */
MSUSERET void* MSAPI ms_malloca(ms_allocator const * const allocator, size_t const size, size_t const alignment);

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

#endif // MS_MEMORY_H
