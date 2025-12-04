/**
 * @file
 *
 * Data containers.
 */
#ifndef MS_CONTAINERS_H
#define MS_CONTAINERS_H

#include <moonsugar/api.h>
#include <moonsugar/memory.h>
#include <moonsugar/functional.h>
#include <moonsugar/hash.h>

typedef struct {
  ms_allocator allocator;

  /**
   * The bit map.
   *
   * Each integer uses all 64 bits, with the LSB
   * being the one with the lowest index.
   */
  uint64_t * data;

  /**
   * The number of integers contained in `data`.
   */
  uint32_t capacity;

  /**
   * True if the bitset automatically resizes to acommodate
   * out of bounds indices.
   */
  bool resizable;
} ms_bitset;

typedef struct {
  /**
   * The allocator.
   */
  ms_allocator allocator;

  /**
   * The initial number of bits to allocate capacity for.
   */
  uint32_t initial_capacity;

  /**
   * True if the bitset automatically resizes to acommodate
   * out of bounds indices.
   */
  bool resizable;
} ms_bitset_description;

/**
 * Construct a new bit set.
 *
 * @param bitset The bitset to construct.
 * @param description The description.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY on memory allocation failure
 *  - MS_RESULT_INVALID_ARGUMENT if heap is NULL
 */
ms_result MSAPI ms_bitset_construct(
  ms_bitset * const bitset,
  ms_bitset_description const * const description
);

/**
 * Destroy a bit set.
 *
 * @param bitset The bitset.
 */
void MSAPI ms_bitset_destroy(ms_bitset * const bitset);

/**
 * Set a bit.
 *
 * @param bitset The bitset.
 * @param index The index of the bit to set.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if reallocation fails on growth
 *  - MS_RESULT_INVALID_ARGUMENT if growth is 0 and the index is out of bounds
 */
ms_result MSAPI ms_bitset_set(ms_bitset * const bitset, uint32_t const index);

/**
 * Clear a bit.
 *
 * @param bitset The bitset.
 * @param index The index of the bit to clear.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if reallocation fails on growth
 *  - MS_RESULT_INVALID_ARGUMENT if growth is 0 and the index is out of bounds
 */
ms_result MSAPI ms_bitset_clear(ms_bitset * const bitset, uint32_t const index);

/**
 * Clear all bits.
 *
 * @param bitset The bitset to clear.
 */
void MSAPI ms_bitset_clear_all(ms_bitset * const bitset);

/**
 * Resize a bitset.
 *
 * @param bitset The bitset.
 * @param new_capacity The new number of bits to allocate capacity for.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if reallocation fails on growth
 */
ms_result MSAPI ms_bitset_resize(ms_bitset * const bitset, uint32_t const new_capacity);

/**
 * Get the value of a bit. This function assumes the bit index is within bounds.
 *
 * @param bitset The bitset.
 * @param index The index of the bit to get.
 *
 * @return True if the bit is set, false if not.
 */
bool MSAPI ms_bitset_get(ms_bitset * const bitset, uint32_t const index);

/**
 * @def MS_PVECTOR_FOREACH_PAGE(vector, actions)
 *
 * @brief Expands to a for-loop that iterates across
 * all the pages of a paged vector. The page is exposed
 * as a restricted pointer `page`.
 *
 * @param vector The vector to iterate through.
 * @param actions The statements to place in the body of the loop.
 */
#define MS_PVECTOR_FOREACH_PAGE(vector, actions) \
  for(ms_pvector_page *page = (vector)->first, *next_page = NULL; page != NULL; \
      page = next_page) { \
    next_page = page->next; \
    actions; \
  }

/**
 * @def MS_PVECTOR_FOREACH(vector, item_type, actions)
 *
 * @brief Expands to a for-loop that iterates across
 * all the items of a paged vector. The page is exposed
 * as a restricted pointer `page`, the item is exposed as a
 * restricted const pointer `item`.
 *
 * @param vector The vector to iterate through.
 * @param item_type The type of item stored in the vector.
 * @param actions The statements to place in the body of the loop.
 */
#define MS_PVECTOR_FOREACH(vector, item_type, actions) \
  MS_PVECTOR_FOREACH_PAGE( \
      (vector), \
      for(uint32_t item_page_index = 0; item_page_index < page->count; ++item_page_index) { \
        item_type *const item = (item_type *)page->data + item_page_index; \
        actions; \
      } \
  )

/**
 * @def MS_PVECTOR_PAGE_SIZE_STATIC(item_type, page_capacity)
 *
 * @brief Expand to the size of a paged vector.
 *
 * @param item_type The type of the item stored in the page.
 * @param page_capacity The maximum number of items that can be stored in a page.
 *
 * @return The size of a page, in bytes.
 */
#define MS_PVECTOR_PAGE_SIZE_STATIC(item_type, page_capacity) \
  (sizeof(ms_pvector_page) + sizeof(item_type) * page_capacity)

/**
 * @def MS_PVECTOR_PAGE_SIZE(vector)
 *
 * @brief Expand to the size of a paged vector.
 *
 * @param vector The vector object.
 *
 * @return The size of a page, in bytes.
 */
#define MS_PVECTOR_PAGE_SIZE(vector) \
  (sizeof(ms_pvector_page) + (vector)->item_size * (vector)->page_capacity)

/**
 * A page of items for the paged vector.
 */
typedef struct ms_pvector_page ms_pvector_page;

struct ms_pvector_page {
  /**
   * Number of items stored in this page.
   *
   * This must always be the same as `ms_pvector::page_capacity`
   * for all pages but the last.
   */
  uint32_t count;

  /**
   * Pointer to the next page or NULL if this is the last page.
   */
  ms_pvector_page *next;

  /**
   * Page data.
   */
  uint8_t data[];
};

/**
 * A vector whose items are arranged in pages. When the
 * vector reaches its capacity, a new page may be added
 * without relocating any memory.
 */
typedef struct {
  /**
   * Number of pages currently stored in the vector.
   */
  uint32_t page_count;

  /**
   * Number of items that can be stored in each page.
   */
  uint32_t page_capacity;

  /**
   * Size of one vector item, in bytes.
   */
  uint32_t item_size;

  /**
   * First page or NULL if the vector is empty.
   */
  ms_pvector_page *first;

  /**
   * Last page or NULL if the vector is empty.
   */
  ms_pvector_page *last;

  /**
   * Memory allocator.
   */
  ms_allocator allocator;
} ms_pvector;

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
} ms_pvector_description;

/**
 * Construct a new paged vector.
 *
 * @param this The vector.
 * @param description The vector description.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_INVALID_ARGUMENT if page_capacity or item_size are 0 or description is NULL
 */
ms_result MSAPI ms_pvector_construct(
  ms_pvector *const this,
  ms_pvector_description const *const description
);

/**
 * Destroy a paged vector.
 *
 * @param this The vector.
 */
void MSAPI ms_pvector_destroy(ms_pvector *const this);

/**
 * Append an item.
 *
 * @param this The vector.
 * @param value_ptr The pointer to the value to store.
 *
 * @return MS_RESULT_SUCCESS on success, an error if resizing failed.
 */
MSAPI ms_result ms_pvector_append(
  ms_pvector *const this,
  void const *const value_ptr
);

/**
 * Get the pointer to an item.
 *
 * @param this The vector.
 * @param item_index The index at which to retrieve the item.
 *
 * @return
 *  - The item pointer on success
 *  - NULL if the index is out of bounds; this
 *      is only a valid return code if boundary checks are enabled.
 */
MSAPI void *ms_pvector_get_ptr(ms_pvector *const this, uint32_t const item_index);

/**
 * Get the pointer to an item.
 *
 * @param this The vector.
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
MSAPI ms_result ms_pvector_get_value(
  ms_pvector *const this,
  uint32_t const item_index,
  void *const out_value_ptr
);

/**
 * Set the value of an item.
 *
 * @param this The vector.
 * @param item_index The index at which to set the item.
 * @param value_ptr The pointer to the value to store. The pointed memory
 *  area must be at least of `ms_pvector::item_size` bytes.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success,
 *  - MS_RESULT_INVALID_ARGUMENT if the index is out of bounds; this
 *      is only a valid return code if boundary checks are enabled.
 */
MSAPI ms_result ms_pvector_set(
  ms_pvector *const this,
  uint32_t const item_index,
  void const *const value_ptr
);

/**
 * Append a new page.
 *
 * @param this The vector.
 *
 * @return
 *  - MS_RESULT_SUCCESS on success
 *  - MS_RESULT_HOST_MEMORY if allocation for the new page failed
 */
MSAPI ms_result ms_pvector_append_page(ms_pvector *const this);

#define MS_MAP_SLICE_SIZE (16u)

#define MS_MAP_GET_KEY(map, index) ((void*)((uint8_t*)(map)->keys + (index) * (map)->key_size))
#define MS_MAP_GET_VALUE(map, index) ((void*)((uint8_t*)(map)->values + (index) * (map)->value_size))

typedef struct {
  ms_allocator allocator;
  ms_equals_clbk are_keys_equal;
  ms_hash_clbk hash_key;
  ms_none_test_clbk is_key_none;
  ms_none_set_clbk set_key_none;
  uint32_t capacity;
  uint32_t growth_factor;
  uint32_t key_size;
  uint32_t value_size;
} ms_map_description;

/**
 * A hash map structure.
 */
typedef struct {
  uint32_t capacity;
  uint32_t growth_factor;
  uint32_t key_size;
  uint32_t value_size;
  void *keys;
  void *values;
  ms_allocator allocator;
  ms_equals_clbk are_keys_equal;
  ms_hash_clbk hash_key;
  ms_none_test_clbk is_key_none;
  ms_none_set_clbk set_key_none;
} ms_map;

MSAPI ms_result ms_map_construct( 
  ms_map *const map, 
  ms_map_description const *const description
); 

MSAPI void ms_map_destroy(ms_map *const map); 

MSAPI void *MSUSERET ms_map_get_value( 
  ms_map const *const map, 
  void const *const key
); 

MSAPI void *MSUSERET ms_map_get_key( 
  ms_map const *const map, 
  void const *const key
); 

MSAPI ms_result ms_map_set( 
  ms_map *const map, 
  void const *const key, 
  void const *const value
); 

MSAPI bool ms_map_remove( 
  ms_map const *const map, 
  void const *const key
);


#endif // MS_CONTAINERS_H
