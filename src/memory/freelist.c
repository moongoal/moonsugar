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
#define NODE_END(chunk) ((void *)((uint8_t *)(chunk) + (chunk)->size))

static uint64_t compute_free_list_node_size(uint64_t const alloc_size, uint32_t const alignment) {
  return ms_align_sz(alloc_size + sizeof(ms_header) + alignment - 1, alignment);
}

void ms_free_list_create_node(
  ms_free_list* const list,
  ms_free_list_node *restrict const chunk,
  ms_free_list_node *restrict const prev,
  ms_free_list_node *restrict const next,
  uint64_t const size
) {
  list->on_before_node_create(list, chunk, sizeof(ms_free_list_node), list->user);

  chunk->size = size;
  chunk->prev = prev;
  chunk->next = next;

  MS_ASSERT(chunk->size > 0);

  if(prev) {
    MS_ASSERT(prev < chunk);
    prev->next = chunk;
  } else {
    list->first = chunk;
  }

  if(next) {
    MS_ASSERT(next > chunk);
    next->prev = chunk;
  }
}

static void detach_free_list_node(ms_free_list *restrict const list, ms_free_list_node *restrict const chunk) {
  MS_ASSERT(chunk);

  if(list->first == chunk) {
    list->first = chunk->next;
  }

  if(chunk->prev) {
    chunk->prev->next = chunk->next;
  }

  if(chunk->next) {
    chunk->next->prev = chunk->prev;
  }
}

static ms_free_list_node * find_smallest_free_free_list_node(
  ms_free_list *const list,
  size_t const aligned_count
) {
  ms_free_list_node *smallest = NULL;
  ms_free_list_node *chunk;

  // Find first chunk that can at least cover the requirement
  for(chunk = list->first; chunk != NULL; chunk = chunk->next) {
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

static size_t ms_free_list_malloc_node(
  ms_free_list *const list,
  ms_free_list_node *chunk,
  size_t const total_alloc_size
) {
  MS_ASSERT(chunk);
  MS_ASSERT(chunk->size >= total_alloc_size);

  size_t const remaining_size = chunk->size - total_alloc_size;
  size_t new_total_alloc_size = total_alloc_size;

  if(remaining_size >= sizeof(ms_free_list_node)) { // Can be split
    ms_free_list_node *const remaining_free_list_node = (ms_free_list_node *)((uint8_t *)chunk + total_alloc_size);

    ms_free_list_create_node(list, remaining_free_list_node, chunk, chunk->next, remaining_size);

    MS_ASSERT(chunk->size > remaining_size);
    MS_ASSERT(!remaining_free_list_node->next || remaining_free_list_node < remaining_free_list_node->next);

    chunk->size -= remaining_size;
  } else { // Cannot be split, take the whole chunk
    new_total_alloc_size = chunk->size;
  }

  detach_free_list_node(list, chunk);
  MS_ASSERT(!chunk->next || NODE_END(chunk) <= (void *)chunk->next);

  return new_total_alloc_size;
}

void *ms_free_list_malloc(
  ms_free_list *restrict const list,
  size_t const count,
  uint32_t const alignment,
  size_t *restrict const out_total_size
) {
  size_t total_size = compute_free_list_node_size(count, alignment);
  ms_free_list_node *const chunk = find_smallest_free_free_list_node(list, total_size);

  if(chunk) {
    list->on_before_alloc_from_node(list, chunk, total_size, list->user);
    total_size = ms_free_list_malloc_node(list, chunk, total_size);
  }

  *out_total_size = total_size;

  return chunk;
}

static void coalesce(ms_free_list *restrict const list, ms_free_list_node *const left, ms_free_list_node *const right) {
  MS_ASSERT(left);
  MS_ASSERT(right);

  detach_free_list_node(list, right);
  left->size += right->size;
}

/**
 * Coalesce neighbors around the given chunk, if they are contiguous.
 *
 * @param chunk The chunk to attempt coalescing with its neighbors.
 *
 * @return The first chunk of the sequence, after the merge.
 */
static ms_free_list_node * try_coalesce_neighbors(ms_free_list *restrict const list, ms_free_list_node *const chunk) {
  MS_ASSERT(chunk);

  ms_free_list_node *const prev = chunk->prev;
  ms_free_list_node *const next = chunk->next;

  MS_ASSERT((prev == NULL) || (NODE_END(prev) <= (void *)chunk));
  MS_ASSERT((next == NULL) || (NODE_END(chunk) <= (void *)next));

  if(next && (next == NODE_END(chunk))) {
    // Coalesce right
    coalesce(list, chunk, next);
  }

  if(prev && (chunk == NODE_END(prev))) {
    // Coalesce left
    coalesce(list, prev, chunk);

    return prev;
  }

  return chunk;
}

static ms_free_list_node *find_prev_free_list_node(ms_free_list *restrict const list, ms_free_list_node *const subject) {
  for(ms_free_list_node *c = list->first; c != NULL; c = c->next) {
    if(c < subject) { // c is a previous chunk
      if(
        (c->next == NULL) // c is last chunk of list
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

void ms_free_list_free(ms_free_list *restrict const list, void * const ptr, size_t const size) {
  ms_free_list_node * node = ptr;

  if(list->first) {
    ms_free_list_node *const prev = find_prev_free_list_node(list, node);

    MS_ASSERT(!prev || NODE_END(prev) <= (void *)node);

    ms_free_list_create_node(list, node, prev, prev ? prev->next : list->first, size);

    node = try_coalesce_neighbors(list, node);
  } else {
    // Only chunk of list
    ms_free_list_create_node(list, node, NULL, NULL, node->size);
  }
}

