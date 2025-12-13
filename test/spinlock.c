#include <moondance/test.h>
#include <moonsugar/thread.h>

typedef struct {
  ms_spinlock lock;
  MS_ATOMIC(int) n;
} lifecycle_ctx;

static void lifecycle_main(void* raw_ctx) {
  lifecycle_ctx * const restrict ctx = raw_ctx;

  ms_spinlock_lock(&ctx->lock);
  ctx->n++;
  ms_spinlock_unlock(&ctx->lock);
}

MD_CASE(spinlock) {
  lifecycle_ctx ctx = {0};
  ms_thread t;
	ms_thread_description const d = { lifecycle_main, NULL, &ctx };

  const ms_result result = ms_thread_spawn(&t, &d);
  md_assert(result == MS_RESULT_SUCCESS);

  ms_spinlock_construct(&ctx.lock);

  ms_spinlock_lock(&ctx.lock);
  ms_thread_sleep(ms_time_from_ms(500));
  ms_spinlock_unlock(&ctx.lock);

  md_assert(ms_thread_join(&t));
  md_assert(ctx.n == 1);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  md_add(&suite, spinlock);

  return md_run(argc, argv, &suite);
}
