#include <moonsugar/test.h>
#include <moonsugar/containers/map.h>

static ms_result test_map_construct(ms_map * const map, uint32_t const initial_capacity) {
  return ms_map_construct(map, &(ms_map_description) {
    g_allocator,
    ms_equals_u32,
    ms_hash_u32,
    ms_none_test_max_u32,
    ms_none_set_max_u32,
    initial_capacity,
    0,
    sizeof(uint32_t),
    sizeof(uint32_t)
  });
}

static void suite_setup(md_suite *suite) { ((void)suite); MST_MEMORY_INIT(); }
static void suite_cleanup(md_suite *suite) { ((void)suite); MST_MEMORY_DESTROY(); }

MD_CASE(construct) {
  ms_map map;

  ms_result const result = test_map_construct(&map, 64);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(map.capacity == 64);
  md_assert(map.growth_factor == 0);
  md_assert(map.keys != NULL);
  md_assert(map.values != NULL);
}

MD_CASE(construct__capacity_not_power_2) {
  ms_map map;

  ms_result const result = test_map_construct(&map, 63);

  md_assert(result == MS_RESULT_INVALID_ARGUMENT);
}

MD_CASE(get) {
  ms_map map;

  ms_result const result = test_map_construct(&map, 16);
  md_assert(result == MS_RESULT_SUCCESS);

  ms_map_set(&map, &(uint32_t){0x01ff}, &(uint32_t){0x01ff});

  // Same key found
  uint32_t *const ptr2 = ms_map_get_value(&map, &(uint32_t){0x01ff});
  md_assert(*ptr2 == 0x01ff);

  // Key not found
  uint32_t *const ptr3 = ms_map_get_value(&map, &(uint32_t){0x0200});
  md_assert(ptr3 == NULL);
}

MD_CASE(remove) {
  ms_map map;

  ms_result const result = test_map_construct(&map, 16);
  md_assert(result == MS_RESULT_SUCCESS);

  ms_map_set(&map, &(uint32_t){0x01ff}, &(uint32_t){0x01ff});
  ms_map_remove(&map, &(uint32_t){0x01ff});

  uint32_t *const ptr2 = ms_map_get_value(&map, &(uint32_t){0x01ff});
  md_assert(ptr2 == NULL);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  suite.suite_setup = suite_setup;
  suite.suite_cleanup = suite_cleanup;

  md_add(&suite, construct);
  md_add(&suite, construct__capacity_not_power_2);
  md_add(&suite, get);
  md_add(&suite, remove);

  return md_run(argc, argv, &suite);
}


