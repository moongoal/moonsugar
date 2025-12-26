#include <memory.h>
#include <moonsugar/assert.h>
#include <moonsugar/log.h>
#include <moonsugar/util.h>
#include <moonsugar/memory.h>
#include <moonsugar/sys.h>

// End pointer of mallocd memory
#define ARENA_END(arena) ((void *)((uint8_t *)(arena->base) + (arena)->size))

// Test whether a pointer belongs to the given arena
#define DOES_PTR_BELONG(arena, ptr) \
  ((uint8_t*)ptr > arena->base && (uint8_t *)ptr < (arena->base + arena->total_size))

ms_header *ms_arena_get_header(void *const ptr) { return (ms_header *)(ptr)-1; }

static void on_before_node_create(
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

static void on_before_alloc_from_node(
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

void ms_arena_construct(ms_arena *const arena, ms_arena_description const * const description) {
  ms_sys_info const *restrict const si = ms_get_sys_info();

  if(si->alloc_granularity < MS_DEFAULT_ALIGNMENT) {
    ms_errorf(
        "Reported OS allocation granularity %llu cannot satisfy minimum engine allocation requirement %llu",
        (uint64_t)si->alloc_granularity,
        (uint64_t)MS_DEFAULT_ALIGNMENT
    );
  }

  *arena = (ms_arena){
      description->base_size,
      NULL,
      description->allocator,
      description->flags
  };
}

void ms_arena_destroy(ms_arena *const arena) {
  // Destroy internally allocated arenas
  for(ms_arena_node * a = arena->first, *b; a != NULL; a = b) {
    b = a->next;
    ms_free(&arena->allocator, a);
  }

  arena->first = NULL;
}

static ms_arena_node* create_arena_node(ms_arena * const arena, uint64_t const size) {
  ms_arena_node * const next = ms_malloc(&arena->allocator, size + sizeof(ms_arena_node), MS_DEFAULT_ALIGNMENT);

  *next = (ms_arena_node) {
    { NULL, NULL, on_before_node_create, on_before_alloc_from_node },
    size,
    0,
    NULL
  };

  // Create first chunk
  ms_free_list_create_node(&next->free_list, (ms_free_list_node*)next->base, NULL, NULL, size);

  return next;
}

static void* allocate_from_node(ms_arena_node * const node, size_t const count, size_t const alignment) {
  // Add header
  size_t chunk_size;
  uint8_t *const unaligned_ptr = ms_free_list_malloc(&node->free_list, count, alignment, &chunk_size);

  // Not enough continguous memory
  if(unaligned_ptr == NULL) {
    return NULL;
  }
  uint8_t *const aligned_min_ptr = unaligned_ptr + sizeof(ms_header); // Assumes 0 padding
  uint8_t *const aligned_ptr = ms_align_ptr(aligned_min_ptr, alignment);
  uint32_t const padding = aligned_ptr - aligned_min_ptr;
  ms_header *restrict const hdr = (ms_header *)aligned_ptr - 1;

  hdr->size = chunk_size;
  hdr->alignment = alignment;
  hdr->padding = padding;

  node->allocated_size += chunk_size;

  return aligned_ptr;
}

void * ms_arena_malloc(ms_arena *const arena, size_t const count, size_t alignment) {
  // Minimum alignment requirement
  alignment = ms_max(alignment, MS_DEFAULT_ALIGNMENT);

  if(count > 0) {
    ms_arena_node *prev = NULL;

    for(ms_arena_node * node = arena->first; node != NULL; prev = node, node = node->next) {
      uint64_t node_free_size = node->total_size - node->allocated_size;

      // Preliminary size check
      if(node_free_size < count) {
        continue;
      }

      void * const ptr = allocate_from_node(node, count, alignment);

      if(ptr != NULL) {
        return ptr;
      }
    }

    // Unable to allocate to any of the available nodes
    // When allocating a new node we want to either store
    // a minimum of 16 items if they are too large or
    // increase the capacity to twice the amount of memory
    // of the last available node.
    uint64_t const new_node_size = 2 * ms_align_sz(
      ms_max(count * 8, prev ? prev->total_size : arena->base_size),
      arena->base_size
    );

    ms_arena_node * const new_node = create_arena_node(arena, new_node_size);

    if(prev) {
      prev->next = new_node;
    } else {
      arena->first = new_node;
    }

    return allocate_from_node(new_node, count, alignment);
  }

  return NULL;
}

void ms_arena_free(ms_arena *const arena, void *const ptr) {
  ms_arena_node *prev = NULL;

  if(ptr == NULL) {
    return;
  }

  for(ms_arena_node * node = arena->first; node != NULL; prev = node, node = node->next) {
    if(DOES_PTR_BELONG(node, ptr)) {
      ms_header *restrict const head = ms_arena_get_header(ptr);
      ms_free_list_node * const chunk = (ms_free_list_node *)((uint8_t *)head - head->padding);
      size_t const chunk_size = chunk->size;

      ms_free_list_free(&node->free_list, chunk, head->size);

      MS_ASSERT(node->allocated_size >= chunk_size);
      node->allocated_size -= chunk_size;

      if(node->allocated_size == 0 && !ms_test(arena->flags, MS_ARENA_STICKY_BIT)) {
        ms_free(&arena->allocator, node);

        if(prev) {
          prev->next = NULL;
        } else {
          arena->first = NULL;
        }
      }

      return;
    }
  }

  ms_error("Attempting to free pointer not mallocd via this arena.");
}

void *ms_arena_realloc(ms_arena *const arena, void *const ptr, size_t const new_count) {
  if(ptr) {
    if(new_count > 0) {
      ms_header * const hdr = ms_arena_get_header(ptr);

      if(new_count <= hdr->size) {
        return ptr;
      }

      void * const new_ptr = ms_arena_malloc(arena, new_count, hdr->alignment);

      memcpy(new_ptr, ptr, hdr->size);
      ms_arena_free(arena, ptr);

      return new_ptr;
    } else {
      ms_arena_free(arena, ptr);
      return NULL;
    }
  }

  return ms_arena_malloc(arena, new_count, MS_DEFAULT_ALIGNMENT);
}

void ms_arena_clear(ms_arena *const arena) {
  for(
    ms_arena_node * node = arena->first, *prev = NULL;
    node != NULL;
    prev = node, node = node->next
  ) {
    node->allocated_size = 0;

    if(!ms_test(arena->flags, MS_ARENA_STICKY_BIT)) {
      ms_free(&arena->allocator, node);

      if(prev) {
        prev->next = NULL;
      } else {
        arena->first = NULL;
      }
    } else {
      ms_free_list_create_node(&node->free_list, (ms_free_list_node*)node->base, NULL, NULL, node->total_size);
    }
  }
}
