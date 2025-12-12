#include <stdatomic.h>
#include <moondance/test.h>
#include <moonsugar/thread.h>

typedef struct {
  atomic_int n;
} lifecycle_ctx;

static void suite_setup(md_suite *suite) {
  ((void)suite);
}

static void lifecycle_main(void* ctx) {
  ((lifecycle_ctx *)ctx)->n++;
}

MD_CASE(lifecycle__one) {
  lifecycle_ctx ctx = {0};
  ms_thread t;

  ms_thread_description const d = {
    lifecycle_main,
    NULL,
    &ctx
  };

  const ms_result result = ms_thread_spawn(&t, &d);

  md_assert(result == MS_RESULT_SUCCESS);

  ms_thread_join(&t);
  md_assert(ctx.n == 1);
}

MD_CASE(lifecycle__multiple) {
  lifecycle_ctx ctx = {0};
  ms_thread t[4];

  for(int i = 0; i < 4; ++i) {
    ms_thread_description const d = {
      lifecycle_main,
      NULL,
      &ctx
    };

    ms_result const result = ms_thread_spawn(&t[i], &d);
    md_assert(result == MS_RESULT_SUCCESS);
  }

  for(int i = 0; i < 4; ++i) {
    ms_thread_join(&t[i]);
  }

  md_assert(ctx.n == 4);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  suite.suite_setup = suite_setup;

  md_add(&suite, lifecycle__one);
  md_add(&suite, lifecycle__multiple);

  return md_run(argc, argv, &suite);
}
