#include <moonsugar/assert.h>
#include <moonsugar/memory.h>
#include <moonsugar/thread.h>

#define SLEEP_INTERVAL ms_time_from_ns(100)

static void thread_main(void *const ctx) {
  ms_thread_pool *const pool = ctx;

  while(!ms_atomic_load(&pool->must_join, MS_MEMORY_ORDER_RELAXED)) {
    // We extract a task and if the task still has childrens to
    // complete, we won't be re-enqueueing. When the last child
    // completes, we'll re-enqueue the task.
    ms_task *const t = ms_task_queue_dequeue(&pool->tasks);

    if(t != NULL) {
      if(ms_atomic_load(&t->unsatisfied_dependencies, MS_MEMORY_ORDER_ACQUIRE) == 0) {
        t->handler(t->ctx);

        if(t->parent != NULL) {
          uint_fast16_t const parent_unsatisfied_count = ms_atomic_sub_fetch(
            &t->parent->unsatisfied_dependencies,
            1,
            MS_MEMORY_ORDER_RELEASE
          );

          if(parent_unsatisfied_count == 0) {
            ms_task_queue_enqueue(&pool->tasks, t->parent);
          }
        }
      }
    } else {
      ms_thread_sleep(SLEEP_INTERVAL);
    }
  }
}

ms_result ms_thread_pool_construct(ms_thread_pool *const pool, ms_thread_pool_description const * const description) {
  memset(pool, 0, sizeof(ms_thread_pool));

  if(description->thread_count == 0) {
    return MS_RESULT_INVALID_ARGUMENT;
  }

  *pool = (ms_thread_pool) {
    description->allocator,
    ms_malloc(&description->allocator, sizeof(ms_thread) * description->thread_count, MS_DEFAULT_ALIGNMENT),
    false,
    {0},
    description->thread_count
  };

  if(pool->threads == NULL) {
    return MS_RESULT_MEMORY;
  }

  MS_CKRET(
    ms_task_queue_construct(
      &pool->tasks,
      &(ms_task_queue_description) { pool->allocator, description->task_capacity }
    )
  );

  for(size_t i = 0; i < description->thread_count; ++i) {
    ms_thread_description const td = {
      thread_main,
      "Worker",
      pool
    };

    MS_CKRET(ms_thread_spawn(&pool->threads[i], &td));
  }

  return MS_RESULT_SUCCESS;
}

void ms_thread_pool_destroy(ms_thread_pool *const pool) {
  MS_ASSERT(pool);

  pool->must_join = true;

  for(size_t i = 0; i < pool->thread_count; ++i) {
    ms_thread_join(&pool->threads[i]);
  }

  ms_task_queue_destroy(&pool->tasks);
  ms_free(&pool->allocator, pool->threads);
  pool->threads = NULL;
  pool->thread_count = 0;
}

bool ms_thread_pool_dispatch(ms_thread_pool *const pool, ms_task *const task) {
  if( // Do not enqueue tasks that can't be run
    ms_atomic_load(&task->unsatisfied_dependencies, MS_MEMORY_ORDER_ACQUIRE) == 0
  ) {
    return ms_task_queue_enqueue(&pool->tasks, task);
  }

  return true; // The parent will be enqueued by the last completing child
}

