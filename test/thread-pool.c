#include <moonsugar/test.h>
#include <moonsugar/thread.h>

static ms_thread_pool pool;

#define WORKER_COUNT (8u)
#define TASK_CAPACITY (64u)

struct task_ctx {
  MS_ATOMIC(uint32_t) counter;
};

static void suite_setup(md_suite * const suite) {
  ((void)suite);
  MST_MEMORY_INIT();
}

static void suite_cleanup(md_suite * const suite) {
  ((void)suite);
  MST_MEMORY_DESTROY();
}

static void each_setup(void * ctx) {
  ((void)ctx);

  ms_thread_pool_construct(
    &pool,
    &(ms_thread_pool_description) {
      g_allocator,
      WORKER_COUNT,
      TASK_CAPACITY
    }
  );
}

static void each_cleanup(void * ctx) {
  ((void)ctx);
  ms_thread_pool_destroy(&pool);
}

static void counter_incrementer(void *const param) {
  struct task_ctx *const ctx = param;
  ms_atomic_fetch_add(&ctx->counter, 1, MS_MEMORY_ORDER_SEQ_CST);
}

MD_CASE(dispatch__no_parents) {
  int safety_count = 200;
  struct task_ctx ctx = {0};
  ms_task t[] = {
    {counter_incrementer, NULL, &ctx, 0},
    {counter_incrementer, NULL, &ctx, 0},
    {counter_incrementer, NULL, &ctx, 0}
  };
  size_t task_count = sizeof(t) / sizeof(ms_task);

  for(unsigned i = 0; i < task_count; ++i) {
    ms_thread_pool_dispatch(&pool, &t[i]);
  }

  while(ctx.counter != task_count && safety_count) {
    ms_thread_sleep(ms_time_from_ms(100));
    safety_count--;
  }

  md_assert(safety_count > 0);
  md_assert(ctx.counter == task_count);
}

MD_CASE(dispatch__with_parents) {
  int safety_count = 100;
  struct task_ctx ctx = {0};
  ms_task t[] = {
    {counter_incrementer, NULL, &ctx, 1},
    {counter_incrementer, NULL, &ctx, 1},
    {counter_incrementer, NULL, &ctx, 1}
  };
  ms_task t2[] = {
    {counter_incrementer, &t[0], &ctx, 0},
    {counter_incrementer, &t[1], &ctx, 0},
    {counter_incrementer, &t[2], &ctx, 0}
  };
  size_t task_count = sizeof(t) / sizeof(ms_task);

  for(unsigned i = 0; i < task_count; ++i) {
    ms_thread_pool_dispatch(&pool, &t2[i]);
    ms_thread_pool_dispatch(&pool, &t[i]);
  }

  task_count *= 2; // For each top-level task, we added a child

  while(ctx.counter != task_count && safety_count) {
    ms_thread_sleep(ms_time_from_ms(100));
    safety_count--;
  }

  md_assert(safety_count > 0);
  md_assert(ctx.counter == task_count);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  suite.suite_setup = suite_setup;
  suite.suite_cleanup = suite_cleanup;
  suite.each_setup = each_setup;
  suite.each_cleanup = each_cleanup;
  suite.each_cleanup = each_cleanup;

  md_add(&suite, dispatch__no_parents);
  md_add(&suite, dispatch__with_parents);

  return md_run(argc, argv, &suite);
}

