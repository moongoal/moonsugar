#include <moonsugar/test.h>
#include <moonsugar/util.h>
#include <moonsugar/containers/bitset.h>

#define INITIAL_CAPACITY 16

static ms_bitset bitset;

static void suite_setup(md_suite* suite) {
  ((void)suite);

  MST_MEMORY_INIT();
}

static void suite_cleanup(md_suite* suite) {
  ((void)suite);

  MST_MEMORY_DESTROY();
}

static void each_cleanup(void* ctx) {
  ((void)ctx);

  ms_bitset_destroy(&bitset);
}

MD_CASE(ctor) {
  ms_result const result = ms_bitset_construct(&bitset, &(ms_bitset_description) { g_allocator, INITIAL_CAPACITY, true });
  md_assert(result == MS_RESULT_SUCCESS);

  md_assert(bitset.capacity == 1);
  md_assert(bitset.data != NULL);
  md_assert(bitset.resizable);
}

MD_CASE(ctor__zero_capacity) {
  ms_result const result = ms_bitset_construct(&bitset, &(ms_bitset_description) { g_allocator, 0, false });
  md_assert(result == MS_RESULT_SUCCESS);

  md_assert(bitset.capacity == 0);
  md_assert(bitset.data == NULL);
  md_assert(!bitset.resizable);
}

MD_CASE(set__first_alloc) {
  ms_result const result = ms_bitset_construct(&bitset, &(ms_bitset_description) { g_allocator, 0, true });
  md_assert(result == MS_RESULT_SUCCESS);

  md_assert(ms_bitset_set(&bitset, 1) == MS_RESULT_SUCCESS);
  md_assert(bitset.capacity == 1);
  md_assert(!ms_bitset_get(&bitset, 0));
  md_assert(ms_bitset_get(&bitset, 1));
}

MD_CASE(set__no_growth) {
  ms_result const result = ms_bitset_construct(&bitset, &(ms_bitset_description) { g_allocator, INITIAL_CAPACITY, false });
  md_assert(result == MS_RESULT_SUCCESS);

  uint32_t const old_capacity = bitset.capacity;

  md_assert(ms_bitset_set(&bitset, 1) == MS_RESULT_SUCCESS);
  md_assert(old_capacity == bitset.capacity);
}

MD_CASE(set__no_alloc) {
  ms_result const result = ms_bitset_construct(&bitset, &(ms_bitset_description) { g_allocator, INITIAL_CAPACITY, false });
  md_assert(result == MS_RESULT_SUCCESS);

  md_assert(ms_bitset_set(&bitset, 666) == MS_RESULT_INVALID_ARGUMENT);
}

MD_CASE(set__further_alloc) {
  ms_result const result = ms_bitset_construct(&bitset, &(ms_bitset_description) { g_allocator, INITIAL_CAPACITY, true });
  md_assert(result == MS_RESULT_SUCCESS);

  md_assert(ms_bitset_set(&bitset, 666) == MS_RESULT_SUCCESS);
  md_assert(ms_bitset_get(&bitset, 666));
}

MD_CASE(clear) {
  ms_result const result = ms_bitset_construct(&bitset, &(ms_bitset_description) { g_allocator, INITIAL_CAPACITY, true });
  md_assert(result == MS_RESULT_SUCCESS);

  md_assert(ms_bitset_set(&bitset, 5) == MS_RESULT_SUCCESS);
  md_assert(ms_bitset_clear(&bitset, 5) == MS_RESULT_SUCCESS);
  md_assert(!ms_bitset_get(&bitset, 5));
}

MD_CASE(clear_all) {
  ms_result const result = ms_bitset_construct(&bitset, &(ms_bitset_description) { g_allocator, INITIAL_CAPACITY, true });
  md_assert(result == MS_RESULT_SUCCESS);

  md_assert(ms_bitset_set(&bitset, 5) == MS_RESULT_SUCCESS);
  ms_bitset_clear_all(&bitset);
  md_assert(!ms_bitset_get(&bitset, 5));
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  suite.suite_setup = suite_setup;
  suite.suite_cleanup = suite_cleanup;
  suite.each_cleanup = each_cleanup;

  md_add(&suite, ctor);
  md_add(&suite, ctor__zero_capacity);
  md_add(&suite, set__first_alloc);
  md_add(&suite, set__no_growth);
  md_add(&suite, set__no_alloc);
  md_add(&suite, set__further_alloc);
  md_add(&suite, clear);
  md_add(&suite, clear_all);

  return md_run(argc, argv, &suite);
}

