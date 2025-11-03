#include <stdlib.h>
#include <moondance/test.h>
#include <moonsugar/coroutine.h>

enum {
  STATE_INC1 = MS_CO_STATUS_BEGIN,
  STATE_INC2
};

MS_CO_COROUTINE(coro) {
  unsigned * const n = ctx;

  switch(status) {
    case STATE_INC1: {
      *n += 1;
    } ms_co_yield(STATE_INC2);

    case STATE_INC2: {
      *n += 2;
    } ms_co_return();

    default: { abort(); }
  }
}

MD_CASE(run_once) {
  ms_co_scheduler s;
  int n = 0;

  ms_co_scheduler_construct(&s);
  ms_co_schedule(&s, coro, &n);
  md_assert(s.count == 1);

  // First run
  ms_co_scheduler_run_once(&s);
  md_assert(s.count == 1);
  md_assert(n == 1);

  // Second run
  ms_co_scheduler_run_once(&s);
  md_assert(s.count == 0);
  md_assert(n == 3);

  // Nothing happens, nothing executed
  ms_co_scheduler_run_once(&s);
  md_assert(s.count == 0);
  md_assert(n == 3);
}

MD_CASE(loop) {
  ms_co_scheduler s;
  int n = 0;

  ms_co_scheduler_construct(&s);
  ms_co_schedule(&s, coro, &n);
  md_assert(s.count == 1);

  // Loop
  ms_co_scheduler_loop(&s);
  md_assert(s.count == 0);
  md_assert(n == 3);

  // Nothing happens, nothing executed
  ms_co_scheduler_loop(&s);
  md_assert(s.count == 0);
  md_assert(n == 3);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  md_add(&suite, run_once);
  md_add(&suite, loop);

  return md_run(argc, argv, &suite);
}
