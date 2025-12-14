#include <moonsugar/test.h>
#include <moonsugar/assert.h>
#include <moonsugar/containers/indexed-pool.h>

#define POOL_SIZE (16u)

static ms_ipool pool;

static void suite_setup(md_suite *suite) { ((void)suite); MST_MEMORY_INIT(); }
static void suite_cleanup(md_suite *suite) { ((void)suite); MST_MEMORY_DESTROY(); }

void each_setup(void *ctx) {
  ((void)ctx);
  ms_ipool_description desc = {g_allocator, 64};

  ms_result const result = ms_ipool_construct(&pool, &desc);
  MS_ASSERT(result == MS_RESULT_SUCCESS);
}

void each_cleanup(void *ctx) { ((void)ctx); ms_ipool_destroy(&pool); }

MD_CASE(ctor) {
  md_assert(pool.item_count = 64);
  md_assert(pool.last_state == 0);
}

MD_CASE(acquire) {
  uint32_t const index = ms_ipool_acquire(&pool);

  md_assert(index == 0);
}

MD_CASE(acquire__all) {
  uint32_t index;

  for(unsigned i = 0; i < pool.item_count; ++i) {
    index = ms_ipool_acquire(&pool);

    md_assert(index != UINT32_MAX);
  }

  index = ms_ipool_acquire(&pool);

  md_assert(index == UINT32_MAX);
  md_assert(pool.state[0] == MS_IPOOL_STATE_FULL);
}

MD_CASE(release) {
  uint32_t h, h2;

  for(unsigned i = 0; i < pool.item_count; ++i) {
    h = ms_ipool_acquire(&pool);
  }

  ms_ipool_release(&pool, h);
  md_assert(pool.state[0] != MS_IPOOL_STATE_FULL);

  // Triggered a second time to attempt releasing an
  // already released item
  md_assert(ms_ipool_release(&pool, h) == MS_RESULT_INVALID_ARGUMENT);

  h2 = ms_ipool_acquire(&pool);
  md_assert(h2 == h);
}

MD_CASE(resize__grow) {
  for(unsigned i = 0; i < pool.item_count; ++i) {
    (void)ms_ipool_acquire(&pool);
  }

  ms_result const resize_result = ms_ipool_resize(&pool, 128);

  md_assert(resize_result == MS_RESULT_SUCCESS);
  md_assert(pool.item_count = 128);
  md_assert(pool.last_state == 0);
  md_assert(pool.state[0] == MS_IPOOL_STATE_FULL);
  md_assert(pool.state[1] == 0);
}

MD_CASE(resize__shrink) {
  (void)ms_ipool_acquire(&pool);

  ms_result resize_result = ms_ipool_resize(&pool, 128);
  md_assert(resize_result == MS_RESULT_SUCCESS);

  resize_result = ms_ipool_resize(&pool, 64);

  md_assert(resize_result == MS_RESULT_SUCCESS);
  md_assert(pool.item_count = 64);
  md_assert(pool.last_state == 0);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  suite.suite_setup = suite_setup;
  suite.suite_cleanup = suite_cleanup;

  suite.each_setup = each_setup;
  suite.each_cleanup = each_cleanup;

  md_add(&suite, ctor);
  md_add(&suite, acquire);
  md_add(&suite, acquire__all);
  md_add(&suite, release);
  md_add(&suite, resize__grow);
  md_add(&suite, resize__shrink);

  return md_run(argc, argv, &suite);
}

