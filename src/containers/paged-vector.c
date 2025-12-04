#include <string.h>
#include <moonsugar/assert.h>
#include <moonsugar/containers/paged-vector.h>

ms_result ms_pvector_construct(
  ms_pvector *const this,
  ms_pvector_description const *const description
) {
  MS_ASSERT(this);

  if(description == NULL) {
    return MS_RESULT_INVALID_ARGUMENT;
  }

  if(description->page_capacity > 0 && description->item_size > 0) {
    *this = (ms_pvector){
      0, // page count
      description->page_capacity, // page capacity
      description->item_size, // item size
      NULL, // first
      NULL, // last
      description->allocator
    };

    return MS_RESULT_SUCCESS;
  }

  return MS_RESULT_INVALID_ARGUMENT;
}

void ms_pvector_destroy(ms_pvector *const this) {
  MS_ASSERT(this);

  MS_PVECTOR_FOREACH_PAGE(this, ms_free(&this->allocator, page));
}

ms_result ms_pvector_append(ms_pvector *const this, void const *const value_ptr) {
  MS_ASSERT(this);
  MS_ASSERT(value_ptr);

  if(this->last && this->last->count < this->page_capacity) {
    uint32_t const item_index = this->last->count++;

    memcpy(this->last->data + this->item_size * item_index, value_ptr, this->item_size);

    return MS_RESULT_SUCCESS;
  }

  ms_result const resize_result = ms_pvector_append_page(this);

  if(resize_result != MS_RESULT_SUCCESS) {
    return resize_result;
  }

  return ms_pvector_append(this, value_ptr);
}

ms_result ms_pvector_get_value(
  ms_pvector *const this,
  uint32_t const item_index,
  void *const out_ptr
) {
  void *internal_ptr = ms_pvector_get_ptr(this, item_index);

  if(internal_ptr != NULL) {
    memcpy(out_ptr, internal_ptr, this->item_size);

    return MS_RESULT_SUCCESS;
  }

  return MS_RESULT_INVALID_ARGUMENT;
}

void *ms_pvector_get_ptr(ms_pvector *const this, uint32_t const item_index) {
  MS_ASSERT(this);

#ifdef MS_FEAT_CHECK_BOUNDS
  if(this->last) {
    uint32_t const total_item_count = (this->page_count - 1) * this->page_capacity + this->last->count;

    if(item_index >= total_item_count) {
      return NULL;
    }
  } else {
    return NULL;
  }
#endif // MS_FEAT_CHECK_BOUNDS

  uint32_t page_index = item_index / this->page_capacity;
  uint32_t const item_offset = (item_index % this->page_capacity) * this->item_size;
  ms_pvector_page *page = this->first;

  for(; page != NULL; page = page->next) {
    if(page_index-- == 0) {
      break;
    }
  }

  return page->data + item_offset;
}

ms_result ms_pvector_set(
  ms_pvector *const this,
  uint32_t const item_index,
  void const *const value_ptr
) {
  MS_ASSERT(this);
  MS_ASSERT(value_ptr);

  void *item_ptr = ms_pvector_get_ptr(this, item_index);

  if(item_ptr != NULL) {
    MS_ASSERT(item_ptr);

    memcpy(item_ptr, value_ptr, this->item_size);

    return MS_RESULT_SUCCESS;
  }

  return MS_RESULT_INVALID_ARGUMENT;
}

ms_result ms_pvector_append_page(ms_pvector *const this) {
  MS_ASSERT(this);

  ms_pvector_page *const page = ms_malloc(&this->allocator, MS_PVECTOR_PAGE_SIZE(this), MS_DEFAULT_ALIGNMENT);

  if(page == NULL) {
    return MS_RESULT_MEMORY;
  }

  page->count = 0;
  page->next = NULL;

  if(this->last == NULL) {
    this->first = this->last = page;
  } else {
    this->last->next = page;
    this->last = page;
  }

  this->page_count++;

  return MS_RESULT_SUCCESS;
}


