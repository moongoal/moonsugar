#include <limits.h>
#include <memory.h>
#include <moonsugar/assert.h>
#include <moonsugar/containers/indexed-pool.h>

// Div by 64 shift count
#define DIV64(x) (x >> 6)
#define MUL64(x) (x << 6)

/**
 * @def MS_IPOOL_STATE_COUNT(size)
 * @brief Compute the state size depending on pool size.
 *
 * @param size The pool size.
 *
 * @return The number of state blocks required to hold the state
 *  of the pool of `size` items.
 */
#define STATE_BLOCK_COUNT(size) (DIV64(MS_ALIGN_SZ_STATIC(size, 64)))

ms_result ms_ipool_construct(ms_ipool *const this, ms_ipool_description const *const desc) {
  MS_ASSERT(this);
  MS_ASSERT(desc);

  if(desc->capacity % 64 != 0) {
    return MS_RESULT_INVALID_ARGUMENT;
  }

  uint32_t const state_count = STATE_BLOCK_COUNT(desc->capacity);
  uint32_t const state_size = state_count * sizeof(ms_ipool_state_block);
  ms_ipool_state_block *const state = ms_malloc(&desc->allocator, state_size, MS_DEFAULT_ALIGNMENT);

  if(state == NULL && state_size > 0) {
    return MS_RESULT_MEMORY;
  }

  memset(state, 0, state_size);

  *this = (ms_ipool){
    state,
    desc->capacity,
    0, // last_state
    desc->allocator
  };

  return MS_RESULT_SUCCESS;
}

void ms_ipool_destroy(ms_ipool *const this) {
  MS_ASSERT(this);

  ms_free(&this->allocator, this->state);

  this->item_count = 0;
  this->state = NULL;
}

static uint32_t find_block_offset(uint64_t const block) {
  for(uint32_t i = 0; i < sizeof(block) * 8; ++i) {
    if(ms_test(block, 1llu << i) == 0) {
      return i;
    }
  }

  return UINT_MAX;
}

static uint32_t try_acquire(ms_ipool *const this, uint64_t const i) {
  uint32_t offset;

  if(this->state[i] != MS_IPOOL_STATE_FULL) {
    offset = find_block_offset(this->state[i]);
  } else {
    return UINT32_MAX;
  }

  this->state[i] = ms_set(this->state[i], (1ull << offset));
  this->last_state = i;

  return MUL64(i) + offset;
}

uint32_t ms_ipool_acquire(ms_ipool *const this) {
  MS_ASSERT(this);

  uint32_t const state_count = DIV64(this->item_count);
  uint_fast32_t const last_state = this->last_state;

  for(uint_fast32_t i = last_state; i < state_count; ++i) {
    uint32_t const h = try_acquire(this, i);

    if(h != UINT32_MAX) {
      return h;
    }
  }

  for(uint_fast32_t i = 0; i < last_state; ++i) {
    uint32_t const h = try_acquire(this, i);

    if(h != UINT32_MAX) {
      return h;
    }
  }

  return UINT32_MAX;
}

ms_result ms_ipool_release(ms_ipool *const this, uint32_t const item) {
  MS_ASSERT(this);

  uint32_t const block = DIV64(item);
  uint32_t const offset = item & 0xffffff;
  uint64_t const set_mask = 1ull << offset;
  uint64_t const prev_state = this->state[block];
  bool const was_set = prev_state & set_mask;

  this->state[block] = ms_clear(this->state[block], set_mask);

  return ms_choose(MS_RESULT_SUCCESS, MS_RESULT_INVALID_ARGUMENT, was_set);
}

MSAPI ms_result ms_ipool_resize(ms_ipool *const this, uint32_t const new_count) {
  MS_ASSERT(this);

  if(new_count % 64 != 0) {
    return MS_RESULT_INVALID_ARGUMENT;
  }

  uint32_t const old_state_count = STATE_BLOCK_COUNT(this->item_count);
  uint32_t const new_state_count = STATE_BLOCK_COUNT(new_count);

  ms_ipool_state_block *const new_state = ms_realloc(
    &this->allocator,
    this->state,
    new_state_count * sizeof(ms_ipool_state_block)
  );

  if(new_state == NULL && new_state_count > 0) {
    return MS_RESULT_MEMORY;
  }

  this->state = new_state;

  if(new_state_count > old_state_count) {
    uint32_t state_count_delta = new_state_count - old_state_count;

    memset(new_state + old_state_count, 0, sizeof(ms_ipool_state_block) * state_count_delta);
  }

  this->state = new_state;
  this->last_state = 0;
  this->item_count = new_count;

  return MS_RESULT_SUCCESS;
}
