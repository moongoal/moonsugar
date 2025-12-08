#include <moonsugar/test.h>
#include <moonsugar/containers/sparse-paged-array.h>

#define ITEMS_PER_PAGE (256u)

ms_sparray array;

void suite_setup(md_suite * const suite) {
  ((void)suite);
  MST_MEMORY_INIT();
}

void suite_cleanup(md_suite * const suite) {
  ((void)suite);
  MST_MEMORY_DESTROY();
}

void each_cleanup(void* ctx) {
  ((void)ctx);
  ms_sparray_destroy(&array);
}

MD_CASE(ctor) {
  ms_sparray_description const d = { g_allocator, NULL, sizeof(int), ITEMS_PER_PAGE };
  ms_result const result = ms_sparray_construct(&array, &d);

  md_assert(result == MS_RESULT_SUCCESS);
}

MD_CASE(ctor__0_size_item) {
  ms_sparray_description const d = { g_allocator, NULL, 0, ITEMS_PER_PAGE };
  ms_result const result = ms_sparray_construct(&array, &d);

  md_assert(result == MS_RESULT_INVALID_ARGUMENT);
}

MD_CASE(ctor__0_items_per_page) {
  ms_sparray_description const d = { g_allocator, NULL, sizeof(int), 0 };
  ms_result const result = ms_sparray_construct(&array, &d);

  md_assert(result == MS_RESULT_INVALID_ARGUMENT);
}

MD_CASE(get_ptr) {
  ms_sparray_description const d = { g_allocator, NULL, sizeof(int), ITEMS_PER_PAGE };
  ms_result const result = ms_sparray_construct(&array, &d);
  md_assert(result == MS_RESULT_SUCCESS);

  void * const ptr1 = ms_sparray_get_ptr(&array, 5);
  md_assert(ptr1 != NULL);
  md_assert(array.page_count == 1);
  md_assert(array.pages != NULL);

  void * const ptr2 = ms_sparray_get_ptr(&array, 6);
  md_assert(ptr2 != ptr1);
  md_assert(array.page_count == 1);

  void * const ptr3 = ms_sparray_get_ptr(&array, 257);
  md_assert(ptr3 != NULL);
  md_assert(array.page_count == 2);
}

MD_CASE(get_ptr__out_of_memory) {
  ms_sparray_description const d = { g_allocator, ms_sparray_zero_page_initializer, sizeof(int), ITEMS_PER_PAGE };
  ms_result const result = ms_sparray_construct(&array, &d);
  md_assert(result == MS_RESULT_SUCCESS);

  void * const ptr = ms_sparray_get_ptr(&array, UINT32_MAX);
  md_assert(ptr == NULL);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  suite.suite_setup = suite_setup;
  suite.suite_cleanup = suite_cleanup;
  suite.each_cleanup = each_cleanup;

  md_add(&suite, ctor);
  md_add(&suite, ctor__0_size_item);
  md_add(&suite, ctor__0_items_per_page);
  md_add(&suite, get_ptr);
  md_add(&suite, get_ptr__out_of_memory);

  return md_run(argc, argv, &suite);
}

