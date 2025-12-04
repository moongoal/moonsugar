/**
 * @file
 *
 * Hash map implementation.
 */
#ifndef MS_CONTAINERS_MAP_H
#define MS_CONTAINERS_MAP_H

#include <moonsugar/api.h>
#include <moonsugar/memory.h>
#include <moonsugar/functional.h>
#include <moonsugar/hash.h>

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

#endif // MS_CONTAINERS_MAP_H

