#include <memory.h>
#include <moonsugar/assert.h>
#include <moonsugar/util.h>
#include <moonsugar/containers/map.h>

ms_result ms_map_construct(
  ms_map *const map, 
  ms_map_description const *const description
) { 
  MS_ASSERT(map); 
  MS_ASSERT(description); 
  MS_ASSERT(description->allocator); 
  MS_ASSERT(description->are_keys_equal);
  MS_ASSERT(description->hash_key);
  MS_ASSERT(description->is_key_none);
  MS_ASSERT(description->set_key_none);

  if(!ms_is_power2(description->capacity)) { 
    return MS_RESULT_INVALID_ARGUMENT; 
  } 

  void *const keys = ms_malloc(
    &description->allocator, 
    description->key_size * description->capacity,
		MS_DEFAULT_ALIGNMENT
  ); 

  if(description->capacity > 0 && keys == NULL) { 
    return MS_RESULT_MEMORY; 
  } 

  void *const values = ms_malloc(
    &description->allocator, 
    description->value_size * description->capacity,
		MS_DEFAULT_ALIGNMENT
  ); 

  if(description->capacity > 0 && values == NULL) { 
    ms_free(&description->allocator, keys); 

    return MS_RESULT_MEMORY; 
  } 

  *map = (ms_map){
    description->capacity, 
    description->growth_factor, 
    description->key_size,
    description->value_size,
    keys, 
    values, 
    description->allocator,
    description->are_keys_equal,
    description->hash_key,
    description->is_key_none,
    description->set_key_none
  }; 

  for(uint32_t i = 0; i < map->capacity; ++i) { 
    void *const key = (uint8_t*)map->keys + i * map->key_size; 

    map->set_key_none(key);
  } 

  return MS_RESULT_SUCCESS; 
} 

void ms_map_destroy(ms_map *const map) { 
  MS_ASSERT(map); 

  ms_free(&map->allocator, map->values); 
  ms_free(&map->allocator, map->keys); 

  map->keys = NULL; 
  map->values = NULL; 
} 

static uint32_t ms_map_get_index( 
  ms_map const *const map, 
  void const *const key
) { 
  uint64_t const h = map->hash_key(key); 
  uint32_t const w = map->capacity - 1; 

  for(uint32_t local_index = 0; local_index < MS_MAP_SLICE_SIZE; 
    ++local_index) { 
    uint32_t global_index = (h + local_index) & w; 
    void const *const key2 = (uint8_t*)map->keys + global_index * map->key_size; 
    void const *const key1 = key; 

    if(map->are_keys_equal(key1, key2)) { 
      return global_index; 
    } 
  } 

  return UINT32_MAX; 
} 

void *ms_map_get_value(
  ms_map const *const map, 
  void const *const key
) { 
  MS_ASSERT(map); 
  MS_ASSERT(key); 

  uint32_t const global_index = ms_map_get_index(map, key); 

  if(global_index != UINT32_MAX) { 
    return MS_MAP_GET_VALUE(map, global_index);
  } 

  return NULL; 
} 

void *ms_map_get_key(
  ms_map const *const map, 
  void const *const key
) { 
  MS_ASSERT(map); 
  MS_ASSERT(key); 

  uint32_t const global_index = ms_map_get_index(map, key); 

  if(global_index != UINT32_MAX) { 
    return MS_MAP_GET_KEY(map, global_index);
  } 

  return NULL; 
} 

static uint32_t find_empty( 
  ms_map const *const map, 
  void const *const key
) { 
  uint64_t const h = map->hash_key(key); 
  uint32_t const w = map->capacity - 1; 

  for(uint32_t local_index = 0; local_index < MS_MAP_SLICE_SIZE; ++local_index) { 
    uint32_t const global_index = (h + local_index) & w; 
    void const *const key = MS_MAP_GET_KEY(map, global_index);

    if(map->is_key_none(key)) { 
      return global_index; 
    } 
  } 

  return UINT32_MAX; 
} 

ms_result ms_map_resize(
  ms_map *const map, 
  uint32_t const new_capacity
) { 
  MS_ASSERT(map); 

  if(!ms_is_power2(new_capacity)) { 
    return MS_RESULT_INVALID_ARGUMENT; 
  } 

  void *const new_keys = ms_malloc(&map->allocator, new_capacity * map->key_size, MS_DEFAULT_ALIGNMENT); 

  if(new_keys == NULL && new_capacity > 0) { 
    return MS_RESULT_MEMORY; 
  } 

  void *const new_values = map->values = ms_malloc(
    &map->allocator,
    new_capacity * map->value_size,
		MS_DEFAULT_ALIGNMENT
  ); 

  if(new_values == NULL & new_capacity > 0) { 
    ms_free(&map->allocator, new_keys); 

    return MS_RESULT_MEMORY; 
  } 

  for(uint32_t i = 0; i < new_capacity; ++i) { 
    void *const key = MS_MAP_GET_KEY(map, i);

    map->set_key_none(key); 
  } 

  uint32_t const w = new_capacity - 1; 

  for(unsigned i = 0; i < new_capacity; ++i) { 
    void *const key = MS_MAP_GET_KEY(map, i);

    if(!map->is_key_none(key)) { 
      void *const value = MS_MAP_GET_VALUE(map, i);
      uint64_t const key_hash = map->hash_key(key); 
      uint32_t const new_index = key_hash & w; 
      void *const new_key = (uint8_t*)new_keys + new_index * map->key_size; 
      void *const new_value = (uint8_t*)new_values + new_index * map->value_size; 

      memcpy(new_key, key, map->key_size);
      memcpy(new_value, value, map->value_size);
    } 
  } 

  ms_free(&map->allocator, map->values); 
  ms_free(&map->allocator, map->keys); 

  map->keys = new_keys; 
  map->values = new_values; 
  map->capacity = new_capacity; 

  return MS_RESULT_SUCCESS; 
} 

ms_result ms_map_set(
  ms_map *const map, 
  void const *const key, 
  void const *const value
) { 
  MS_ASSERT(map); 
  MS_ASSERT(key); 
  MS_ASSERT(value); 

  uint32_t index = ms_map_get_index(map, key); 

  if(index == UINT32_MAX) { 
    index = find_empty(map, key); 
  } 

  if(index != UINT32_MAX) { 
    void *const dest_key = MS_MAP_GET_KEY(map, index);
    void *const dest_value = MS_MAP_GET_VALUE(map, index);

    memcpy(dest_key, key, map->key_size);
    memcpy(dest_value, value, map->value_size);

    return MS_RESULT_SUCCESS; 
  } 

  if(map->growth_factor > 0) { 
    ms_result const resize_result = ms_map_resize(
      map, 
      ms_max(1, map->capacity) * map->growth_factor
    ); 

    if(resize_result == MS_RESULT_SUCCESS) { 
      return ms_map_set(map, key, value); 
    } 

    return resize_result; 
  } 

  return MS_RESULT_FULL; 
} 

bool ms_map_remove(
  ms_map const *const map, 
  void const *const key
) { 
  uint32_t const index = ms_map_get_index(map, key); 

  if(index != UINT32_MAX) { 
    void *const key = MS_MAP_GET_KEY(map, index);
    map->set_key_none(key);

    return true; 
  } 

  return false; 
}

