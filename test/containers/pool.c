#include <moonsugar/test.h>
#include <moonsugar/assert.h>
#include <moonsugar/containers/pool.h>

static ms_pool pool;

static void suite_setup(md_suite *suite) { ((void)suite); MST_MEMORY_INIT(); }
static void suite_cleanup(md_suite *suite) { ((void)suite); MST_MEMORY_DESTROY(); }

void each_setup(void *ctx) {
  ((void)ctx);
  ms_pool_description desc = {g_allocator, sizeof(int), 2};

  ms_result const result = ms_pool_construct(&pool, &desc);
  MS_ASSERT(result == MS_RESULT_SUCCESS);
}

void each_cleanup(void *ctx) { ((void)ctx); ms_pool_destroy(&pool); }

MD_CASE(ctor) {
  md_assert(pool.item_count == 0);
  md_assert(pool.items == NULL);
  md_assert(pool.pages == NULL);
  md_assert(pool.items_per_page == 2);
  md_assert(pool.item_size == sizeof(int));
}

MD_CASE(acquire) {
  int* const p = ms_pool_acquire(&pool);

  md_assert(p != NULL);
}

MD_CASE(release) {
  int* const p = ms_pool_acquire(&pool);
  ms_pool_node * const old_items = pool.items;

  ms_pool_release(&pool, p);
  md_assert(pool.items->data == (void*)p);
  md_assert(pool.items->next = old_items);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  suite.suite_setup = suite_setup;
  suite.suite_cleanup = suite_cleanup;

  suite.each_setup = each_setup;
  suite.each_cleanup = each_cleanup;

  md_add(&suite, ctor);
  md_add(&suite, acquire);
  md_add(&suite, release);

  return md_run(argc, argv, &suite);
}


