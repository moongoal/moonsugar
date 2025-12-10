#include <memory.h>
#include <moonsugar/assert.h>
#include <moonsugar/util.h>
#include <moonsugar/log.h>
#include <moonsugar/memory.h>

void ms_stack_construct(ms_stack * const restrict stack, uint64_t const max_size) {
  MS_ASSERT(stack);

  void * const memory = ms_reserve(max_size);

  *stack = (ms_stack) {
    memory, // base
    max_size, // size
    memory, // top
    memory // committed_top
  };
}

void ms_stack_destroy(ms_stack * const restrict stack) {
  MS_ASSERT(stack);

  if(stack->top != stack->base) {
    ms_warn("Memory leak detected.");
  }

  ms_release(stack->base, stack->size);
  memset(stack, 0, sizeof(ms_stack));
}

void* ms_stack_malloc(
  ms_stack * const restrict stack,
  size_t size,
  size_t const alignment
) {
  MS_ASSERT(stack);

  size = ms_choose(
    size,
    size + alignment - 1,
    alignment <= MS_DEFAULT_ALIGNMENT
  );

  uint8_t * const ptr = ms_atomic_fetch_add(&stack->top, size, MS_MEMORY_ORDER_RELAXED);
  uint8_t * committed_ptr = ms_atomic_load(&stack->committed_top, MS_MEMORY_ORDER_ACQUIRE);
  uint8_t * const end_ptr = ptr + size;

  if(end_ptr > committed_ptr) {
    uint64_t const size_to_commit = end_ptr - committed_ptr;
    uint8_t * const new_committed_top = committed_ptr + size_to_commit;

    bool const commit_result MSUNUSED = ms_commit(committed_ptr, size_to_commit);
    MS_ASSERT(commit_result);

    // If this fails, another write has succeeded. If the other commit is
    // smaller than this, we'll commit again sooner than expected, but we can
    // tolerate that, as it won't cause failures. If the other commit is larger,
    // we have more memory committed so don't need to do anything anyway.
    ms_atomic_compare_exchange_strong(
      &stack->committed_top,
      &committed_ptr,
      new_committed_top,
      MS_MEMORY_ORDER_RELEASE,
      MS_MEMORY_ORDER_RELAXED
    );
  }

  return ptr;
}

void ms_stack_clear(ms_stack * const restrict stack) {
  MS_ASSERT(stack);

  ms_atomic_store(&stack->top, stack->base, MS_MEMORY_ORDER_SEQ_CST);

  // This could cause failures under high pressure as we're reducing the
  // committed pointer to the base and AMSER performing memory decommittment.
  // A thread could be attempting to commit memory that we are about to decommit.
  uint8_t * const committed_top = ms_atomic_exchange(&stack->committed_top, stack->base, MS_MEMORY_ORDER_SEQ_CST);
  uint64_t const committed_size = committed_top - (uint8_t*)stack->base;

  if(committed_size > MS_HEAP_DEALLOC_THR) {
    uint64_t const size_to_decommit = committed_size - MS_HEAP_DEALLOC_THR;

    ms_decommit(committed_top - size_to_decommit, size_to_decommit);
  }
}
