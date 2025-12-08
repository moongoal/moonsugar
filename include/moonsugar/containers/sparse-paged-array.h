/**
 * @file
 *
 * Sparse array of items allocated in pages.
 * The array uses an array list of pages
 * where page elements are allowed to be NULL.
 *
 * Only relevant pages are allocated.
 */
#ifndef MS_CONTAINERS_SPARSE_PAGED_ARRAY_H
#define MS_CONTAINERS_SPARSE_PAGED_ARRAY_H

#include <moonsugar/api.h>
#include <moonsugar/memory.h>

typedef struct ms_sparray ms_sparray;

typedef struct {
  void *items;
} ms_sparray_page;

/**
 * Initializer for page backing memory.
 *
 * @param array Pointer to the array.
 * @param page Pointer to the page being initialized.
 */
typedef void (*ms_sparray_page_initializer)(
  ms_sparray const * const array,
  void * const page
);

/**
 * Iteration function.
 *
 * @param array Pointer to the array.
 * @param item Pointer to the individual item.
 * @param context User-provided context value.
 */
typedef void (*ms_sparray_item_iter)(
  ms_sparray const * const array,
  void * const item,
  void * const context
);

struct ms_sparray {
  /**
   * Array of page pointers.
   */
  ms_sparray_page *pages;

  /**
   * Array page allocator.
   */
  ms_allocator allocator;

  /**
   * Page memory initializer. This function is called whenever
   * a new page is allocated.
   */
  ms_sparray_page_initializer page_initializer;

  /**
   * The number of page item slots within `pages`.
   */
  uint32_t page_count;

  /**
   * The number of items stored within
   * an individual page.
   */
  uint32_t items_per_page;

  /**
   * The size of one page item, in bytes.
   */
  uint32_t item_size;
};

typedef struct {
  /**
   * Array page allocator.
   */
  ms_allocator allocator;

  /**
   * Option page memory initializer. This function is called whenever
   * a new page is allocated. If this is NULL, memory is not initialized
   * after allocation.
   */
  ms_sparray_page_initializer page_initializer;

  /**
   * The size of one page item, in bytes.
   */
  uint32_t item_size;

  /**
   * The number of items stored within
   * an individual page.
   */
  uint32_t items_per_page;
} ms_sparray_description;

/**
 * Construct a sparse paged array.
 *
 * @param array The array.
 * @param description The description.
 *
 * @return The result of the operation.
 *  - MS_RESULT_SUCCESS on success.
 *  - MS_RESULT_INVALID_ARGUMENT if `description->item_size == 0`
 *    or `description->items_per_page == 0`.
 */
ms_result MSAPI ms_sparray_construct(
  ms_sparray * const array,
  ms_sparray_description const * const description
);

/**
 * Destroy a sparse paged array.
 *
 * @param array The array.
 */
void MSAPI ms_sparray_destroy(ms_sparray * const array);

/**
 * Get the pointer to an item. This function
 * will allocate a new page if needed.
 *
 * @param array The array.
 * @param index The index of the item.
 *
 * @return The pointer to the item or NULL if memory could not
 *  be allocated.
 */
void *MSAPI ms_sparray_get_ptr(
  ms_sparray * const array,
  uint32_t const index
);

/**
 * Get the pointer to an item. This function
 * will never allocate a new page. It's undefined behaviour
 * to call this function for any index that was not previously
 * used for a `ms_sparray_get_ptr()` invocation.
 *
 * This function is intended to be a faster alternative to
 * `ms_sparray_get_ptr()` when the caller is sure backing
 * memory for the item is present.
 *
 * @param array The array.
 * @param index The index of the item.
 *
 * @return The pointer to the item.
 */
MSUSERET void * MSAPI ms_sparray_get_ptr_unsafe(
  ms_sparray * const array,
  uint32_t const index
);

/**
 * Invoke a function for every item in the array.
 *
 * @param array The array.
 * @param callback The callback to invoke.
 * @param context The context as passed to the callback function.
 */
void MSAPI ms_sparray_foreach(
  ms_sparray * const array,
  ms_sparray_item_iter const callback,
  void * const context
);

/**
 * Initializer that fill the memory page with zeroes.
 */
void MSAPI ms_sparray_zero_page_initializer(
  ms_sparray const * const array,
  void * const page
);

#endif // MS_CONTAINERS_SPARSE_PAGED_ARRAY_H

