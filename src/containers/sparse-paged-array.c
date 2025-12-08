#include <memory.h>
#include <moonsugar/assert.h>
#include <moonsugar/util.h>
#include <moonsugar/containers/sparse-paged-array.h>

ms_result ms_sparray_construct(
  ms_sparray * const restrict array,
  ms_sparray_description const * const restrict description
) {
  MS_ASSERT(array);
  MS_ASSERT(description);

  if(
    description->item_size == 0
    || description->items_per_page == 0
  ) {
    return MS_RESULT_INVALID_ARGUMENT;
  }

  *array = (ms_sparray) {
    NULL,
    description->allocator,
    description->page_initializer,
    0,
    description->items_per_page,
    description->item_size
  };

  return MS_RESULT_SUCCESS;
}

void ms_sparray_destroy(ms_sparray * const restrict array) {
  MS_ASSERT(array);

  for(uint32_t i = 0; i < array->page_count; ++i) {
    ms_sparray_page const p = array->pages[i];

    if(p.items != NULL) {
      ms_free(&array->allocator, p.items);
    }
  }

  ms_free(&array->allocator, array->pages);
  array->pages = NULL;
  array->page_count = 0;
}

static bool resize_pages(
  ms_sparray * const restrict array,
  uint32_t const min_page_count
) {
  MS_ASSERT(min_page_count > array->page_count);

  array->pages = ms_realloc(
    &array->allocator,
    array->pages,
    sizeof(ms_sparray_page)
    * min_page_count
  );
  
  if(array->pages == NULL) {
    return false;
  }

  for(uint32_t i = array->page_count; i < min_page_count; ++i) {
    array->pages[i].items = NULL;
  }

  array->page_count = min_page_count;

  return true;
}

void *ms_sparray_get_ptr(
  ms_sparray * const restrict array,
  uint32_t const index
) {
  MS_ASSERT(array);

  uint32_t const page_index = index / array->items_per_page;
  uint32_t const item_index = index % array->items_per_page;

  if(page_index >= array->page_count) {
    if(!resize_pages(array, page_index + 1)) {
      return NULL;
    }
  }

  ms_sparray_page * const restrict page = &array->pages[page_index];

  if(page->items == NULL) {
    page->items = ms_malloc(
      &array->allocator,
      array->items_per_page * array->item_size,
      MS_DEFAULT_ALIGNMENT
    );

    if(page->items == NULL) {
      return NULL;
    }

    if(array->page_initializer != NULL) {
      array->page_initializer(array, page->items);
    }
  }

  return (uint8_t*)page->items + item_index * array->item_size;
}

void *ms_sparray_get_ptr_unsafe(
  ms_sparray * const restrict array,
  uint32_t const index
) {
  MS_ASSERT(array);

  uint32_t const page_index = index / array->items_per_page;
  uint32_t const item_index = index % array->items_per_page;
  ms_sparray_page * const restrict page = &array->pages[page_index];

  return (uint8_t*)page->items + item_index * array->item_size;
}

void ms_sparray_zero_page_initializer(
  ms_sparray const * const restrict array,
  void * const restrict page
) {
  MS_ASSERT(array);
  MS_ASSERT(page);

  uint32_t const page_size = array->items_per_page * array->item_size;

  memset(page, 0, page_size);
}

void ms_sparray_foreach(
  ms_sparray * const restrict array,
  ms_sparray_item_iter const callback,
  void * const context
) {
  MS_ASSERT(array);
  MS_ASSERT(callback);

  for(uint32_t i = 0; i < array->page_count; ++i) {
    uint8_t * const restrict page = array->pages[i].items;

    for(uint32_t j = 0; j < array->items_per_page; ++j) {
      void * const restrict item = page + array->item_size * j;

      callback(array, item, context);
    }
  }
}

