/**
 * @file
 *
 * Coroutine framework.
 */
#ifndef MS_COROUTINE_H
#define MS_COROUTINE_H

#include <moonsugar/api.h>

/**
 * @def MS_CO_SCHEDULER_MAX_COROUTINES
 *
 * Maximum number of coroutines that can be managed by a scheduler.
 */
#define MS_CO_SCHEDULER_MAX_COROUTINES (1024u)

/**
 * @def MS_CO_STATUS_BEGIN
 *
 * The coroutine was not started.
 */
#define MS_CO_STATUS_BEGIN (0u)

/**
 * @def MS_CO_STATUS_COMPLETED
 *
 * The coroutine has reached its final state.
 */
#define MS_CO_STATUS_COMPLETED (0xffffffffu)

/**
 * @def MS_CO_COROUTINE(name)
 *
 * Coroutine function prototype.
 *
 * @param name The name of the coroutine.
 */
#define MS_CO_COROUTINE(name) ms_co_status name(void *ctx, ms_co_status status)

/**
 * @def ms_co_yield(new_status)
 *
 * Pauses execution of the coroutine, recording that
 * the coroutine will next resume with `new_status`.
 */
#define ms_co_yield(new_status) return new_status

/**
 * @def ms_co_return()
 *
 * Terminates execution the coroutine. Terminated coroutines
 * must not be resumed.
 */
#define ms_co_return() return MS_CO_STATUS_COMPLETED

/**
 * The execution status of the coroutine.
 *
 * When a value of this type is `MS_CO_STATUS_BEGIN`, the
 * associated coroutine was never run.
 *
 * When a value of this type is `MS_CO_STATUS_COMPLETED`, the
 * associated coroutine has reached its final state.
 *
 * When a value of this type assumes any other value, that
 * number represents the internal coroutine status used
 * to resume execution.
 */
typedef unsigned ms_co_status;

/**
 * A coroutine handler. The coroutine handler acts as follows:
 *  1. Reads `coro_status` and jumps to the appropriate
 *    execution point.
 *  2. Executes the logic as needed.
 *  3. Yields a new status value that will be used to resume
 *    the coroutine on the next invocation or returns,
 *    terminating the coroutine.
 *
 * @param ctx The coroutine context object.
 * @param coro_status The current coroutine status.
 *  The first time the handler is invoked, the value of
 *  `coro_status` is `MS_CO_STATUS_BEGIN`.
 *
 * @return The new status of the coroutine after its invocation.
 */
typedef ms_co_status (*ms_co_handler)(void *ctx, ms_co_status coro_status);

typedef struct {
  /**
   * The coroutine handler.
   */
  ms_co_handler handler;

  /**
   * The context object containing inputs, outputs and persisting state
   * to pass to the coroutine handler.
   */
  void *context;

  /**
   * The current status of the coroutine.
   */
  ms_co_status status;
} ms_co_coroutine;

typedef struct {
  /**
   * Executing coroutines.
   */
  ms_co_coroutine coroutines[MS_CO_SCHEDULER_MAX_COROUTINES];

  /**
   * Number of used coroutine slots.
   *
   * Used slots are stored contiguously from the beginning of `coroutines`.
   * When a coroutine slot is freed, the last item in `coroutines` replaces
   * the removed one if there are more than 1 used slots.
   */
  unsigned count;
} ms_co_scheduler;

/**
 * Construct a new scheduler.
 *
 * @param scheduler The scheduler to construct.
 */
void MSAPI ms_co_scheduler_construct(ms_co_scheduler * const scheduler);

/**
 * Run the scheduler once, executing all the scheduled
 * coroutines once.
 *
 * @param scheduler The scheduler to construct.
 */
void MSAPI ms_co_scheduler_run_once(ms_co_scheduler * const scheduler);

/**
 * Run the scheduler in a loop, executing the scheduled
 * coroutines until all are terminated.
 *
 * @param scheduler The scheduler to construct.
 */
void MSAPI ms_co_scheduler_loop(ms_co_scheduler * const scheduler);

/**
 * Schedule a new coroutine for execution.
 *
 * @param scheduler The scheduler.
 * @param handler The coroutine handler.
 * @param context The coroutine context.
 *
 * @return 1 on success, 0 on failure.
 */
int MSAPI ms_co_schedule(
  ms_co_scheduler * const scheduler,
  ms_co_handler const handler,
  void * const context
);

/**
 * Construct a coroutine object.
 *
 * This is part of the low level API and ignores
 * the presence of a scheduler.
 *
 * @param c The coroutine.
 * @param handler The coroutine handler.
 * @param context The coroutine context.
 */
void MSAPI ms_co_construct(
  ms_co_coroutine * const c,
  ms_co_handler const handler,
  void * const context
);
  
/**
 * Execute a coroutine.
 *
 * This is part of the low level API and ignores
 * the presence of a scheduler.
 *
 * @param c The coroutine to execute.
 *
 * @return 1 if the coroutine has terminated, 0 if
 *  the coroutine has paused.
 */
int MSAPI ms_co_execute(ms_co_coroutine * const c);

#endif /* MS_COROUTINE_H */
