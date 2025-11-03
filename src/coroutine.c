#include <memory.h>
#include <assert.h>
#include <moonsugar/coroutine.h>

static void remove_coroutine(ms_co_scheduler * const scheduler, unsigned const coro_index) {
  scheduler->coroutines[coro_index] = scheduler->coroutines[--scheduler->count];
}

void ms_co_scheduler_construct(ms_co_scheduler * const scheduler) {
  scheduler->count = 0;
}

void ms_co_scheduler_run_once(ms_co_scheduler * const scheduler) {
  for(long long i = (long long)scheduler->count - 1; i >= 0; --i) {
    if(ms_co_execute(&scheduler->coroutines[i])) {
      remove_coroutine(scheduler, i);
    }
  }
}

void ms_co_scheduler_loop(ms_co_scheduler * const scheduler) {
  while(scheduler->count > 0) {
    ms_co_scheduler_run_once(scheduler);
  }
}

void ms_co_construct(
  ms_co_coroutine * const c,
  ms_co_handler const handler,
  void * const context
) {
  memset(c, 0, sizeof(ms_co_coroutine));

  *c = (ms_co_coroutine) {
    .handler = handler,
    .context = context,
    .status = MS_CO_STATUS_BEGIN
  };
}

int ms_co_schedule(
  ms_co_scheduler * const scheduler,
  ms_co_handler const handler,
  void * const context
) {
  assert(scheduler);
  assert(handler);

  if(scheduler->count < MS_CO_SCHEDULER_MAX_COROUTINES) {
    ms_co_coroutine * const c = &scheduler->coroutines[scheduler->count++];
    ms_co_construct(c, handler, context);

    return 1;
  }

  return 0;
}

int ms_co_execute(ms_co_coroutine * const c) {
  c->status = c->handler(c->context, c->status);

  return c->status == MS_CO_STATUS_COMPLETED;
}
