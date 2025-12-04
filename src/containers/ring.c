#include <moonsugar/assert.h>
#include <moonsugar/containers/ring.h>
#include <moonsugar/util.h>

ms_result ms_ring_construct(ms_ring *const this, ms_ring_description const *const description) {
  if(!ms_is_power2(description->capacity) || description->capacity == 0) {
    return MS_RESULT_INVALID_ARGUMENT;
  }

  if(description->item_size == 0) {
    return MS_RESULT_INVALID_ARGUMENT;
  }

  *this = (ms_ring){
      0, // count
      0, // windex
      description->capacity,
      description->item_size,
      ms_malloc(&description->allocator, description->item_size * description->capacity, MS_DEFAULT_ALIGNMENT),
      description->allocator
  };

  if(this->values == NULL) {
    return MS_RESULT_MEMORY;
  }

  return MS_RESULT_SUCCESS;
}

ms_result ms_ring_destroy(ms_ring *const this) {
  ms_free(&this->allocator, this->values);

  this->values = NULL;

  return MS_RESULT_SUCCESS;
}

void *ms_ring_enqueue(ms_ring *const this) {
  if(this->count < this->capacity) {
    uint32_t const item_index = (this->windex++) & (this->capacity - 1);
    this->count++;

    return (uint8_t *)this->values + this->item_size * item_index;
  }

  return NULL;
}

void *ms_ring_dequeue(ms_ring *const this) {
  if(this->count > 0) {
    uint32_t const item_index
        = (this->capacity + (this->windex - this->count)) & (this->capacity - 1);
    this->count--;

    return (uint8_t *)this->values + this->item_size * item_index;
  }

  return NULL;
}


