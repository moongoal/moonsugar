#include <memory.h>
#include <moonsugar/assert.h>
#include <moonsugar/log.h>
#include <moonsugar/util.h>
#include <moonsugar/platform.h>
#include <moonsugar/memory.h>

#ifndef _WIN32
  #define memcpy_s(dest, dest_count, orig, orig_count) memcpy(dest, orig, orig_count)
#endif

// End pointer of chunk
#define CHUNK_END(chunk) ((void *)((uint8_t *)(chunk) + (chunk)->size))

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

static uint64_t compute_chunk_size(uint64_t const alloc_size, uint32_t const alignment) {
  return ms_align_sz(alloc_size + sizeof(ms_header) + alignment - 1, alignment);
}

static void create_chunk(
  ms_heap *restrict const heap,
  ms_chunk *restrict const chunk,
  ms_chunk *restrict const prev,
  ms_chunk *restrict const next,
  uint64_t const size
) {
  MS_ASSERT(size <= heap->size);

  chunk->size = size;
  chunk->prev = prev;
  chunk->next = next;

  MS_ASSERT(chunk->size > 0);

  if(prev) {
    MS_ASSERT(prev < chunk);
    prev->next = chunk;
  } else {
    heap->first = chunk;
  }

  if(next) {
    MS_ASSERT(next > chunk);
    next->prev = chunk;
  }
}

static void detach_chunk(ms_heap *restrict const heap, ms_chunk *restrict const chunk) {
  MS_ASSERT(chunk);

  if(heap->first == chunk) {
    heap->first = chunk->next;
  }

  if(chunk->prev) {
    chunk->prev->next = chunk->next;
  }

  if(chunk->next) {
    chunk->next->prev = chunk->prev;
  }
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
      NULL // first
  };

  // Create first chunk
  ms_chunk *const chunk = base_ptr;
  commit(heap, chunk, page_size);
  create_chunk(heap, chunk, NULL, NULL, size);
}

void ms_heap_destroy(ms_heap *const heap) {
  if(heap->first->size != heap->size) {
    ms_warn("Memory leak detected.");
  }

  ms_release(heap->base, heap->size);

  heap->first = NULL;
  heap->committed_size = 0;
  heap->base = NULL;
}

static ms_chunk *
find_smallest_free_chunk(ms_heap *const heap, size_t const aligned_count) {
  ms_chunk *smallest = NULL;
  ms_chunk *chunk;

  // Find first chunk that can at least cover the requirement
  for(chunk = heap->first; chunk != NULL; chunk = chunk->next) {
    if(chunk->size >= aligned_count) {
      smallest = chunk;
      break;
    }
  }

  // Refine to see if there is any smaller chunk satisfying the
  // requirement
  if(chunk && smallest) {
    for(chunk = chunk->next; chunk != NULL && chunk->size < smallest->size; chunk = chunk->next) {
      if(chunk->size >= aligned_count) {
        smallest = chunk;
      }
    }
  }

  return smallest;
}

/**
 * Commit chunk memory if necessary.
 *
 * @param heap The heap.
 * @param chunk The chunk to commit memory for.
 * @param size_to_commit The amount of chunk memory to commit, in bytes.
 */
static void commit_chunk_memory(
  ms_heap *restrict const heap,
  ms_chunk *restrict const chunk,
  size_t const size_to_commit
) {
  void *const heap_commit_end_ptr = HEAP_COMMITTED_END(heap);
  void *const chunk_commit_end_ptr = (uint8_t *)chunk + size_to_commit;

  // Commit more memory if necessary
  if(chunk_commit_end_ptr > heap_commit_end_ptr) {
    size_t const unmallocd_sz
        = ((uint8_t *)chunk_commit_end_ptr - (uint8_t *)heap_commit_end_ptr);

    commit(heap, heap_commit_end_ptr, unmallocd_sz);
  }
}

static size_t
malloc_from_chunk(ms_heap *const heap, ms_chunk *chunk, size_t const total_alloc_size) {
  MS_ASSERT(chunk);
  MS_ASSERT(chunk->size >= total_alloc_size);

  size_t const remaining_size = chunk->size - total_alloc_size;
  size_t new_total_alloc_size = total_alloc_size;

  if(remaining_size >= sizeof(ms_chunk)) { // Can be split
    ms_chunk *const remaining_chunk = (ms_chunk *)((uint8_t *)chunk + total_alloc_size);

    commit_chunk_memory(heap, remaining_chunk, heap->commit_page_size);
    create_chunk(heap, remaining_chunk, chunk, chunk->next, remaining_size);

    MS_ASSERT(chunk->size > remaining_size);
    MS_ASSERT(!remaining_chunk->next || remaining_chunk < remaining_chunk->next);

    chunk->size -= remaining_size;
  } else { // Cannot be split, take the whole chunk
    new_total_alloc_size = chunk->size;
  }

  detach_chunk(heap, chunk);
  MS_ASSERT(!chunk->next || CHUNK_END(chunk) <= (void *)chunk->next);

  return new_total_alloc_size;
}

static void *malloc_from_free_list(
  ms_heap *restrict const heap,
  size_t const count,
  uint32_t const alignment,
  size_t *restrict const out_chunk_size
) {
  size_t total_size = compute_chunk_size(count, alignment);
  ms_chunk *const chunk = find_smallest_free_chunk(heap, total_size);

  if(chunk) {
    commit_chunk_memory(heap, chunk, total_size);
    total_size = malloc_from_chunk(heap, chunk, total_size);
  }

  *out_chunk_size = total_size;

  return chunk;
}

void * ms_heap_malloca(ms_heap *const heap, size_t const count, uint32_t alignment) {
  // Minimum alignment requirement
  alignment = ms_max(alignment, MS_DEFAULT_ALIGNMENT);

  if(count > 0) {
    // Add header
    size_t chunk_size;
    uint8_t *const unaligned_ptr = malloc_from_free_list(heap, count, alignment, &chunk_size);

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

void *ms_heap_malloc(ms_heap *const heap, size_t const count) {
  return ms_heap_malloca(heap, count, MS_DEFAULT_ALIGNMENT);
}

static void coalesce(ms_heap *restrict const heap, ms_chunk *const left, ms_chunk *const right) {
  MS_ASSERT(left);
  MS_ASSERT(right);

  detach_chunk(heap, right);
  left->size += right->size;
}

/**
 * Coalesce neighbors around the given chunk, if they are contiguous.
 *
 * @param chunk The chunk to attempt coalescing with its neighbors.
 *
 * @return The first chunk of the sequence, after the merge.
 */
static ms_chunk * try_coalesce_neighbors(ms_heap *restrict const heap, ms_chunk *const chunk) {
  MS_ASSERT(chunk);

  ms_chunk *const prev = chunk->prev;
  ms_chunk *const next = chunk->next;

  MS_ASSERT((prev == NULL) || (CHUNK_END(prev) <= (void *)chunk));
  MS_ASSERT((next == NULL) || (CHUNK_END(chunk) <= (void *)next));

  if(next && (next == CHUNK_END(chunk))) {
    // Coalesce right
    coalesce(heap, chunk, next);
  }

  if(prev && (chunk == CHUNK_END(prev))) {
    // Coalesce left
    coalesce(heap, prev, chunk);

    return prev;
  }

  return chunk;
}

static ms_chunk *find_prev_chunk(ms_heap *restrict const heap, ms_chunk *const subject) {
  for(ms_chunk *c = heap->first; c != NULL; c = c->next) {
    if(c < subject) { // c is a previous chunk
      if(
          (c->next == NULL) // c is last chunk of heap
          || (c->next > subject) // subject is in between c and next
      ) {
        return c;
      }
    } else {
      break; // All remaining chunks > subject
    }
  }

  return NULL;
}

static void free_to_free_list(ms_heap *restrict const heap, void *const ptr) {
  ms_header *restrict const head = ms_heap_get_header(ptr);
  ms_chunk *chunk = (ms_chunk *)((uint8_t *)head - head->padding);

  if(heap->first) {
    ms_chunk *const prev = find_prev_chunk(heap, chunk);

    MS_ASSERT(!prev || CHUNK_END(prev) <= (void *)chunk);

    create_chunk(heap, chunk, prev, prev ? prev->next : heap->first, head->size);

    chunk = try_coalesce_neighbors(heap, chunk);
  } else {
    // Only chunk of heap
    create_chunk(heap, chunk, NULL, NULL, chunk->size);
  }

  // Decommit memory if necessary
  if(chunk->next == NULL) {
    if(chunk->size > MS_HEAP_DEALLOC_THR) {
      uint8_t *const chunk_start = (uint8_t *)chunk;
      uint8_t *const committed_end = HEAP_COMMITTED_END(heap);

      if(committed_end > chunk_start + MS_HEAP_DEALLOC_THR) {
        size_t const extra_mallocd_mem = committed_end - chunk_start - MS_HEAP_DEALLOC_THR;
        void *const dealloc_ptr = chunk_start + MS_HEAP_DEALLOC_THR;
        decommit(heap, dealloc_ptr, extra_mallocd_mem);
        heap->committed_size -= extra_mallocd_mem;
      }
    }
  }
}

void ms_heap_free(ms_heap *const heap, void *const ptr) {
  if(DOES_PTR_BELONG(heap, ptr)) {
    free_to_free_list(heap, ptr);
  } else {
    if(ptr != NULL) {
      ms_error("Attempting to free pointer not mallocd via this heap.");
    }
  }
}

static void * realloc_from_free_list(ms_heap *const heap, void *restrict const ptr, size_t const new_count) {
  ms_header *const hdr = ms_heap_get_header(ptr);
  size_t const available_size = hdr->size - hdr->padding - sizeof(ms_header);

  if(new_count > available_size) { // Not enough room for expansion
    void *restrict const new_ptr = ms_heap_malloca(heap, new_count, hdr->alignment);

    // Copy the old data and free the existing allocation
    if(new_ptr) {
      memcpy_s(new_ptr, new_count, ptr, available_size);
      ms_heap_free(heap, ptr);

      return new_ptr;
    } else {
      ms_fatal("Unable to relocate memory.");
    }
  } else {
    MS_ASSERT(new_count > 0);
  }

  return ptr;
}

bool ms_heap_owns(ms_heap *const heap, void *const ptr) { return DOES_PTR_BELONG(heap, ptr); }

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
    }
  }

  return ms_heap_malloc(heap, new_count);
}
