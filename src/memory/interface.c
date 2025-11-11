#include <moonsugar/memory.h>

void* ms_malloc(ms_allocator const * const allocator, size_t const size) {
  return allocator->allocate(allocator->user, size);
}

void* ms_malloca(ms_allocator const * const allocator, size_t const size, size_t const alignment) {
  return allocator->allocate_aligned(allocator->user, size, alignment);
}

void* ms_realloc(ms_allocator const * const allocator, void * const ptr, size_t const new_size) {
  return allocator->reallocate(allocator->user, ptr, new_size);
}

void ms_free(ms_allocator const * const allocator, void * const ptr) {
  allocator->deallocate(allocator->user, ptr);
}

