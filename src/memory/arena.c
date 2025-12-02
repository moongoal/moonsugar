#include <memory.h>
#include <moonsugar/assert.h>
#include <moonsugar/log.h>
#include <moonsugar/util.h>
#include <moonsugar/platform.h>
#include <moonsugar/memory.h>

// End pointer of mallocd memory
#define ARENA_END(arena) ((void *)((uint8_t *)(arena->base) + (arena)->size))

// Test whether a pointer belongs to the given arena
#define DOES_PTR_BELONG(arena, ptr) \
  (ptr > arena->base && (uint8_t *)ptr < ((uint8_t *)arena->base + arena->size))

static ms_header *ms_arena_get_header(void *const ptr) { return (ms_header *)(ptr)-1; }

void on_before_node_create(
  ms_free_list * const list,
  void * const ptr,
  size_t const size,
  void * const user
) {
  ((void)list);
  ((void)ptr);
  ((void)size);
  ((void)user);
}

void on_before_alloc_from_node(
  ms_free_list * const list,
  ms_free_list_node * const node,
  size_t const size,
  void * const user
) {
  ((void)list);
  ((void)node);
  ((void)size);
  ((void)user);
}

void ms_arena_construct(ms_arena *const arena, uint64_t const size, void * const base_ptr) {
  ms_sys_info const *restrict const si = ms_get_sys_info();

  if(si->alloc_granularity < MS_DEFAULT_ALIGNMENT) {
    ms_errorf(
        "Reported OS allocation granularity %llu cannot satisfy minimum engine allocation requirement %llu",
        (uint64_t)si->alloc_granularity,
        (uint64_t)MS_DEFAULT_ALIGNMENT
    );
  }

  *arena = (ms_arena){
      (ms_free_list) {
        NULL,
        arena,
        on_before_node_create,
        on_before_alloc_from_node
      }, // free_list
      base_ptr, // base
      size, // size
      NULL, // next
      NULL, // allocator
  };

  // Create first chunk
  ms_free_list_node *const chunk = base_ptr;
  ms_free_list_create_node(&arena->free_list, chunk, NULL, NULL, size);
}

void ms_arena_destroy(ms_arena *const arena) {
  // Destroy internally allocated arenas
  for(ms_arena * a = arena->next, *b; a != NULL; a = b) {
    b = a->next;
    ms_free(a->allocator, a);
  }

  arena->free_list.first = NULL;
  arena->size = 0;
  arena->base = NULL;
}

static ms_arena* create_next_arena(ms_arena * const arena) {
  uint64_t const size = (arena->size * 2) + sizeof(ms_arena);
  ms_arena * const next = ms_malloc(arena->allocator, size);
  void * const base = next + 1;

  ms_arena_construct(next, size, base);

  return next;
}

void * ms_arena_malloca(ms_arena *const arena, size_t const count, uint32_t alignment) {
  // Minimum alignment requirement
  alignment = ms_max(alignment, MS_DEFAULT_ALIGNMENT);

  if(count > 0) {
    // Add header
    size_t chunk_size;
    uint8_t *const unaligned_ptr = ms_free_list_malloc(&arena->free_list, count, alignment, &chunk_size);

    if(unaligned_ptr == NULL) {
      if(arena->next == NULL) {
        arena->next = create_next_arena(arena);
      }

      return ms_arena_malloca(arena->next, count, alignment);
    }

    uint8_t *const aligned_min_ptr = unaligned_ptr + sizeof(ms_header); // Assumes 0 padding
    uint8_t *const aligned_ptr = ms_align_ptr(aligned_min_ptr, alignment);
    uint32_t const padding = aligned_ptr - aligned_min_ptr;
    ms_header *restrict const hdr = (ms_header *)aligned_ptr - 1;

    hdr->size = chunk_size;
    hdr->alignment = alignment;
    hdr->padding = padding;

    return aligned_ptr;
  }

  return NULL;
}

void *ms_arena_malloc(ms_arena *const arena, size_t const count) {
  return ms_arena_malloca(arena, count, MS_DEFAULT_ALIGNMENT);
}

void ms_arena_free(ms_arena *const arena, void *const ptr) {
  if(DOES_PTR_BELONG(arena, ptr)) {
    ms_header *restrict const head = ms_arena_get_header(ptr);
    ms_free_list_node *chunk = (ms_free_list_node *)((uint8_t *)head - head->padding);

    ms_free_list_free(&arena->free_list, chunk, head->size);
  } else {
    if(ptr != NULL) {
      if(arena->next != NULL) {
        ms_arena_free(arena->next, ptr);

        // Remove empty arena
        // The first arena is never removed as we don't know how
        // it's been allocated.
        if(
          arena->next->free_list.first != NULL
          && arena->next->free_list.first->size == arena->next->size
        ) {
          ms_arena * const following = arena->next->next;

          // Arenas internally allocated are all obtained via an
          // individual allocation.
          ms_free(arena->next->allocator, arena->next);
          arena->next = following;
        }
      } else {
        ms_error("Attempting to free pointer not mallocd via this arena.");
      }
    }
  }
}

static void * realloc_from_free_list(ms_arena *const arena, void *restrict const ptr, size_t const new_count) {
  ms_header *const hdr = ms_arena_get_header(ptr);
  size_t const available_size = hdr->size - hdr->padding - sizeof(ms_header);

  if(new_count > available_size) { // Not enough room for expansion
    void *restrict const new_ptr = ms_arena_malloca(arena, new_count, hdr->alignment);

    // Copy the old data and free the existing allocation
    if(new_ptr) {
      ms_free_list_node *chunk = (ms_free_list_node *)((uint8_t *)hdr - hdr->padding);

      memcpy_s(new_ptr, new_count, ptr, available_size);
      ms_free_list_free(&arena->free_list, chunk, available_size);

      return new_ptr;
    } else {
      ms_fatal("Unable to relocate memory.");
    }
  } else {
    MS_ASSERT(new_count > 0);
  }

  return ptr;
}

void *ms_arena_realloc(ms_arena *const arena, void *const ptr, size_t const new_count) {
  if(ptr) {
    if(new_count > 0) {
      if(DOES_PTR_BELONG(arena, ptr)) {
        return realloc_from_free_list(arena, ptr, new_count);
      } else {
        ms_error("Attempting to realloc a pointer not mallocd via this arena.");
      }
    } else {
      ms_arena_free(arena, ptr);
    }
  }

  return ms_arena_malloc(arena, new_count);
}

