#include <limits.h>
#include <memory.h>
#include <moonsugar/assert.h>
#include <moonsugar/containers/pool.h>

ms_result ms_pool_construct(ms_pool *const this, ms_pool_description const *const desc) {
  MS_ASSERT(this);
  MS_ASSERT(desc);
  MS_ASSERT(desc->allocator);

  if(desc->item_size == 0 || desc->items_per_page == 0) {
    return MS_RESULT_INVALID_ARGUMENT;
  }

  *this = (ms_pool) {
    desc->allocator,
    NULL,
    NULL,
    0,
    desc->item_size,
    desc->items_per_page
  };

  return MS_RESULT_SUCCESS;
}

void ms_pool_destroy(ms_pool *const this) {
  MS_ASSERT(this);

  unsigned const page_count = this->item_count / this->items_per_page;

  for(unsigned i = 0; i < page_count; ++i) {
    ms_free(&this->allocator, this->pages[i]);
  }

  ms_free(&this->allocator, this->pages);

  this->item_count = 0;
  this->items = NULL;
  this->pages = NULL;
}

static void append_page(ms_pool *const this) {
  MS_ASSERT(this->items == NULL); // This function assumes no available nodes

  uint32_t const node_size = sizeof(ms_pool_node) + this->item_size;
  uint32_t const size = node_size * this->items_per_page;
  ms_pool_node * const page = ms_malloc(&this->allocator, size, MS_DEFAULT_ALIGNMENT);
  uint32_t const page_count = this->item_count / this->items_per_page;

  this->pages = ms_realloc(&this->allocator, this->pages, sizeof(void*) * (page_count + 1));
  this->pages[page_count] = page;
  this->item_count += this->items_per_page;

  for(uint32_t i = 0; i < this->items_per_page - 1; ++i) {
    ms_pool_node * const c = (ms_pool_node*)((uint8_t*)page + i * node_size);
    ms_pool_node * const n = (ms_pool_node*)((uint8_t*)page + (1 + i) * node_size);

    c->next = n;
  }

  ms_pool_node * const last = (ms_pool_node*)(
    (uint8_t*)page
    + (this->items_per_page - 1) * node_size
  );

  last->next = NULL;
  this->items = page;
}

void* ms_pool_acquire(ms_pool *const this) {
  MS_ASSERT(this);

  if(this->items == NULL) {
    append_page(this);
  }

  void* const ptr = this->items->data;
  this->items = this->items->next;

  return ptr;
}

void ms_pool_release(ms_pool *const this, void * const item) {
  MS_ASSERT(this);
  MS_ASSERT(item);

  ms_pool_node * const node = (ms_pool_node*)((uint8_t*)item - sizeof(ms_pool_node));

  node->next = this->items;
  this->items = node;
}

