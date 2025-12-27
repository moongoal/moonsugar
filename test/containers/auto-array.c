#include <moonsugar/log.h>
#include <moonsugar/test.h>
#include <moonsugar/containers/auto-array.h>

static ms_autoarray array;

#define ITEM_SIZE (sizeof(int))
#define INITIAL_CAP (32u)

void suite_setup(md_suite *suite) {
  ((void)suite);
  MST_MEMORY_INIT();
}

void suite_cleanup(md_suite *suite) {
  ((void)suite);
  MST_MEMORY_DESTROY();
}

void each_setup(void *ctx) {
  ((void)ctx);

  ms_autoarray_description const description = {
    g_allocator,
    ITEM_SIZE,
    INITIAL_CAP
  };

  if(ms_autoarray_construct(&array, &description) != MS_RESULT_SUCCESS) {
    ms_fatal("Unable to construct auto array");
  }
}

void each_cleanup(void *ctx) {
  ((void)ctx);
  ms_autoarray_destroy(&array);
}

MD_CASE(construct) {
  md_assert(array.item_size == ITEM_SIZE);
  md_assert(array.count == 0);
  md_assert(array.capacity == INITIAL_CAP);
}

MD_CASE(resize__no_reserve) {
  ms_autoarray_resize(&array, 2);

  md_assert(array.count == 2);
  md_assert(array.capacity == INITIAL_CAP);
}

MD_CASE(resize__reserve) {
  ms_autoarray_resize(&array, INITIAL_CAP * 2);

  md_assert(array.count == INITIAL_CAP * 2);
  md_assert(array.capacity == INITIAL_CAP * 2);
}

MD_CASE(reserve) {
  ms_autoarray_reserve(&array, INITIAL_CAP * 2);

  md_assert(array.count == 0);
  md_assert(array.capacity == INITIAL_CAP * 2);
}

MD_CASE(get__append) {
  *(int*)ms_autoarray_append(&array) = 123;
  *(int*)ms_autoarray_append(&array) = 124;

  md_assert(*(int*)ms_autoarray_get(&array, 1) == 124);
  md_assert(*(int*)ms_autoarray_get(&array, 0) == 123);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  suite.suite_setup = suite_setup;
  suite.suite_cleanup = suite_cleanup;
  suite.each_setup = each_setup;
  suite.each_cleanup = each_cleanup;

  md_add(&suite, construct);
  md_add(&suite, resize__no_reserve);
  md_add(&suite, resize__reserve);
  md_add(&suite, reserve);
  md_add(&suite, get__append);

  return md_run(argc, argv, &suite);
}
