/**
 * @file
 *
 * Paged array implementation.
 */
#ifndef MS_CONTAINERS_PAGED_ARRAY_H
#define MS_CONTAINERS_PAGED_ARRAY_H

#include <moonsugar/api.h>
#include <moonsugar/memory.h>

/**
 * @def MS_PVECTOR_FOREACH_PAGE(array, actions)
 *
 * @brief Expands to a for-loop that iterates across
 * all the pages of a paged array. The page is exposed
 * as a restricted pointer `page`.
 *
 * @param array The array to iterate through.
 * @param actions The statements to place in the body of the loop.
 */
#define MS_PVECTOR_FOREACH_PAGE(array, actions) \
  for(ms_parray_page *page = (array)->first, *next_page = NULL; page != NULL; \
      page = next_page) { \
    next_page = page->next; \
    actions; \
  }

/**
 * @def MS_PVECTOR_FOREACH(array, item_type, actions)
 *
 * @brief Expands to a for-loop that iterates across
 * all the items of a paged array. The page is exposed
 * as a restricted pointer `page`, the item is exposed as a
 * restricted const pointer `item`.
 *
 * @param array The array to iterate through.
 * @param item_type The type of item stored in the array.
 * @param actions The statements to place in the body of the loop.
 */
#define MS_PVECTOR_FOREACH(array, item_type, actions) \
  MS_PVECTOR_FOREACH_PAGE( \
      (array), \
      for(uint32_t item_page_index = 0; item_page_index < page->count; ++item_page_index) { \
        item_type *const item = (item_type *)page->data + item_page_index; \
        actions; \
      } \
  )

/**
 * @def MS_PVECTOR_PAGE_SIZE_STATIC(item_type, page_capacity)
 *
 * @brief Expand to the size of a paged array.
 *
 * @param item_type The type of the item stored in the page.
 * @param page_capacity The maximum number of items that can be stored in a page.
 *
 * @return The size of a page, in bytes.
 */
#define MS_PVECTOR_PAGE_SIZE_STATIC(item_type, page_capacity) \
  (sizeof(ms_parray_page) + sizeof(item_type) * page_capacity)

/**
 * @def MS_PVECTOR_PAGE_SIZE(array)
 *
 * @brief Expand to the size of a paged array.
 *
 * @param array The array object.
 *
 * @return The size of a page, in bytes.
 */
#define MS_PVECTOR_PAGE_SIZE(array) \
  (sizeof(ms_parray_page) + (array)->item_size * (array)->page_capacity)

/**
 * A page of items for the paged array.
 */
typedef struct ms_parray_page ms_parray_page;

struct ms_parray_page {
  /**
   * Number of items stored in this page.
   *
   * This must always be the same as `ms_parray::page_capacity`
   * for all pages but the last.
   */
  uint32_t count;

  /**
   * Pointer to the next page or NULL if this is the last page.
   */
  ms_parray_page *next;

  /**
   * Page data.
   */
  uint8_t data[];
};

/**
 * A array whose items are arranged in pages. When the
 * array reaches its capacity, a new page may be added
 * without relocating any memory.
 */
typedef struct {
  /**
   * Number of pages currently stored in the array.
   */
  uint32_t page_count;

  /**
   * Number of items that can be stored in each page.
   */
  uint32_t page_capacity;

  /**
   * Size of one array item, in bytes.
   */
  uint32_t item_size;

  /**
   * First page or NULL if the array is empty.
   */
  ms_parray_page *first;

  /**
   * Last page or NULL if the array is empty.
   */
  ms_parray_page *last;

  /**
   * Memory allocator.
   */
  ms_allocator allocator;
} ms_parray;

typedef struct {
  /**
   * The number of items storable in a page.
   */
  uint32_t page_capacity;

  /**
   * The size of one item, in bytes.
   */
  uint32_t item_size;

  /**
   * Allocator.
   */
  ms_allocator allocator;
} ms_parray_description;

/**
 * Construct a new paged array.
 *
 * @param this The array.
 * @param description The array description.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_INVALID_ARGUMENT if page_capacity or item_size are 0 or description is NULL
 */
ms_result MSAPI ms_parray_construct(
  ms_parray *const this,
  ms_parray_description const *const description
);

/**
 * Destroy a paged array.
 *
 * @param this The array.
 */
void MSAPI ms_parray_destroy(ms_parray *const this);

/**
 * Append an item.
 *
 * @param this The array.
 * @param value_ptr The pointer to the value to store.
 *
 * @return MS_RESULT_SUCCESS on success, an error if resizing failed.
 */
MSAPI ms_result ms_parray_append(
  ms_parray *const this,
  void const *const value_ptr
);

/**
 * Get the pointer to an item.
 *
 * @param this The array.
 * @param item_index The index at which to retrieve the item.
 *
 * @return
 *  - The item pointer on success
 *  - NULL if the index is out of bounds; this
 *      is only a valid return code if boundary checks are enabled.
 */
MSAPI void *ms_parray_get_ptr(ms_parray *const this, uint32_t const item_index);

/**
 * Get the pointer to an item.
 *
 * @param this The array.
 * @param item_index The index at which to retrieve the item.
 * @param out_value_ptr When the return value is MS_RESULT_SUCCESS,
 *  the item data will be copied to the referenced memory location.
 *  Otherwise unchanged.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success,
 *  - MS_RESULT_INVALID_ARGUMENT if the index is out of bounds; this
 *      is only a valid return code if boundary checks are enabled.
 */
MSAPI ms_result ms_parray_get_value(
  ms_parray *const this,
  uint32_t const item_index,
  void *const out_value_ptr
);

/**
 * Set the value of an item.
 *
 * @param this The array.
 * @param item_index The index at which to set the item.
 * @param value_ptr The pointer to the value to store. The pointed memory
 *  area must be at least of `ms_parray::item_size` bytes.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success,
 *  - MS_RESULT_INVALID_ARGUMENT if the index is out of bounds; this
 *      is only a valid return code if boundary checks are enabled.
 */
MSAPI ms_result ms_parray_set(
  ms_parray *const this,
  uint32_t const item_index,
  void const *const value_ptr
);

/**
 * Append a new page.
 *
 * @param this The array.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if allocation for the new page failed
 */
MSAPI ms_result ms_parray_append_page(ms_parray *const this);

#endif // MS_CONTAINERS_PAGED_ARRAY_H
