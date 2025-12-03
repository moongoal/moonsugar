#include <memory.h>
#include <moonsugar/assert.h>
#include <moonsugar/log.h>
#include <moonsugar/util.h>
#include <moonsugar/platform.h>
#include <moonsugar/memory.h>

// End pointer of mallocd memory
#define HEAP_COMMITTED_END(heap) ((void *)((uint8_t *)(heap->base) + (heap)->committed_size))

// Test whether a pointer belongs to the given heap
#define DOES_PTR_BELONG(heap, ptr) \
  (ptr > heap->base && (uint8_t *)ptr < ((uint8_t *)heap->base + heap->size))

static inline void commit(ms_heap *restrict const heap, void *const commit_start, size_t const size) {
  bool const committed = ms_commit(commit_start, size);

  if(committed) {
    heap->committed_size += size;
    return;
  }

  ms_fatal("Failed committing memory.");
}

static inline void decommit(ms_heap *restrict const heap, void *const decommit_start, size_t const size) {
  MS_ASSERT(heap->committed_size >= size);

  ms_decommit(decommit_start, size);
  heap->committed_size -= size;
}

ms_header *ms_heap_get_header(void *const ptr) { return (ms_header *)(ptr)-1; }

/**
 * Commit chunk memory if necessary.
 *
 * @param heap The heap.
 * @param chunk The chunk to commit memory for.
 * @param size_to_commit The amount of chunk memory to commit, in bytes.
 */
static void commit_free_list_node_memory(
  ms_heap *restrict const heap,
  ms_free_list_node *restrict const chunk,
  size_t const size_to_commit
) {
  void *const heap_commit_end_ptr = HEAP_COMMITTED_END(heap);
  void *const chunk_commit_end_ptr = (uint8_t *)chunk + size_to_commit;

  // Commit more memory if necessary
  if(chunk_commit_end_ptr > heap_commit_end_ptr) {
    size_t const unmallocd_sz = (
      (uint8_t *)chunk_commit_end_ptr
      - (uint8_t *)heap_commit_end_ptr
    );

    commit(heap, heap_commit_end_ptr, unmallocd_sz);
  }
}

void on_before_node_create(
  ms_free_list * const list,
  void * const ptr,
  size_t const size,
  void * const user
) {
  ((void)list);

  ms_heap * const heap = user;

  commit_free_list_node_memory(heap, ptr, size);
}

void on_before_alloc_from_node(
  ms_free_list * const list,
  ms_free_list_node * const node,
  size_t const size,
  void * const user
) {
  ((void)list);

  ms_heap * const heap = user;

  commit_free_list_node_memory(heap, node, size);
}

void ms_heap_construct(ms_heap *const heap, uint64_t const size, uint64_t const page_size) {
  MS_ASSERT(ms_is_multiple(size, page_size));
  MS_ASSERT(ms_is_power2(page_size));

  ms_sys_info const *restrict const si = ms_get_sys_info();

  if(si->alloc_granularity < MS_DEFAULT_ALIGNMENT) {
    ms_errorf(
        "Reported OS allocation granularity %llu cannot satisfy minimum engine allocation " "requir" "ement " "%llu",
        (uint64_t)si->alloc_granularity,
        (uint64_t)MS_DEFAULT_ALIGNMENT
    );
  }

  void *const base_ptr = ms_reserve(size);

  *heap = (ms_heap){
      base_ptr,
      size,
      page_size,
      0, // committed_size
      (ms_free_list) {
        NULL,
        heap,
        on_before_node_create,
        on_before_alloc_from_node
      }
  };

  // Create first chunk
  ms_free_list_node *const chunk = base_ptr;
  commit(heap, chunk, page_size);
  ms_free_list_create_node(&heap->free_list, chunk, NULL, NULL, size);
}

void ms_heap_destroy(ms_heap *const heap) {
  if(heap->free_list.first->size != heap->size) {
    ms_warn("Memory leak detected.");
  }

  ms_release(heap->base, heap->size);

  heap->free_list.first = NULL;
  heap->committed_size = 0;
  heap->base = NULL;
}

void * ms_heap_malloc(ms_heap *const heap, size_t const count, size_t alignment) {
  // Minimum alignment requirement
  alignment = ms_max(alignment, MS_DEFAULT_ALIGNMENT);

  if(count > 0) {
    // Add header
    size_t chunk_size;
    uint8_t *const unaligned_ptr = ms_free_list_malloc(&heap->free_list, count, alignment, &chunk_size);

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

    return aligned_ptr;
  }

  return NULL;
}

void ms_heap_free(ms_heap *const heap, void *const ptr) {
  if(DOES_PTR_BELONG(heap, ptr)) {
    ms_header *restrict const head = ms_heap_get_header(ptr);
    ms_free_list_node *chunk = (ms_free_list_node *)((uint8_t *)head - head->padding);

    ms_free_list_free(&heap->free_list, chunk, head->size);

    if(chunk->next == NULL && chunk->size > MS_HEAP_DEALLOC_THR) {
      uint8_t *const chunk_start = (uint8_t *)chunk;
      uint8_t *const committed_end = HEAP_COMMITTED_END(heap);

      if(committed_end > chunk_start + MS_HEAP_DEALLOC_THR) {
        size_t const extra_mallocd_mem = committed_end - chunk_start - MS_HEAP_DEALLOC_THR;
        void *const dealloc_ptr = chunk_start + MS_HEAP_DEALLOC_THR;
        decommit(heap, dealloc_ptr, extra_mallocd_mem);
        heap->committed_size -= extra_mallocd_mem;
      }
    }
  } else {
    if(ptr != NULL) {
      ms_error("Attempting to free pointer not mallocd via this heap.");
    }
  }
}

bool ms_heap_owns(ms_heap *const heap, void *const ptr) { return DOES_PTR_BELONG(heap, ptr); }

static void * realloc_from_free_list(ms_heap *const heap, void *restrict const ptr, size_t const new_count) {
  ms_header *const hdr = ms_heap_get_header(ptr);
  size_t const available_size = hdr->size - hdr->padding - sizeof(ms_header);

  if(new_count > available_size) { // Not enough room for expansion
    void *restrict const new_ptr = ms_heap_malloc(heap, new_count, hdr->alignment);

    // Copy the old data and free the existing allocation
    if(new_ptr) {
      ms_free_list_node *chunk = (ms_free_list_node *)((uint8_t *)hdr - hdr->padding);

      memcpy(new_ptr, ptr, available_size);
      ms_free_list_free(&heap->free_list, chunk, available_size);

      return new_ptr;
    } else {
      ms_fatal("Unable to relocate memory.");
    }
  } else {
    MS_ASSERT(new_count > 0);
  }

  return ptr;
}

void *ms_heap_realloc(ms_heap *const heap, void *const ptr, size_t const new_count) {
  if(ptr) {
    if(new_count > 0) {
      if(DOES_PTR_BELONG(heap, ptr)) {
        return realloc_from_free_list(heap, ptr, new_count);
      } else {
        ms_error("Attempting to realloc a pointer not mallocd via this heap.");
      }
    } else {
      ms_heap_free(heap, ptr);
      return NULL;
    }
  }

  return ms_heap_malloc(heap, new_count, MS_DEFAULT_ALIGNMENT);
}
