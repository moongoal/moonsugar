#include <string.h>
#include <moonsugar/assert.h>
#include <moonsugar/thread.h>

ms_result ms_task_queue_construct(ms_task_queue *const q, ms_task_queue_description const * const description) {
  memset(q, 0, sizeof(ms_task_queue));

  ms_rwlock_construct(&q->lock);

  q->store = ms_malloc(
    &description->allocator,
    sizeof(ms_task*) * description->capacity,
    MS_DEFAULT_ALIGNMENT
  );

  if(q->store == NULL) {
    return MS_RESULT_MEMORY;
  }

  MS_CKRET(
    ms_ring_construct(
      &q->queue,
      &(ms_ring_description){
        description->capacity,
        sizeof(ms_task *),
        description->allocator
      }
    )
  );

  return MS_RESULT_SUCCESS;
}

void ms_task_queue_destroy(ms_task_queue *const q) {
  ms_free(&q->queue.allocator, q->store);
  ms_ring_destroy(&q->queue);
  ms_rwlock_destroy(&q->lock);

  q->store = NULL;
}

bool ms_task_queue_enqueue(ms_task_queue *const q, ms_task *const task) {
  bool success = false;

  ms_rwlock_lock_write(&q->lock);
  {
    ms_task **const item = ms_ring_enqueue(&q->queue);

    if(item != NULL) {
      *item = task;
      success = true;
    }
  }
  ms_rwlock_unlock_write(&q->lock);

  return success;
}

bool ms_task_queue_enqueue_many(ms_task_queue *const q, unsigned const count, ms_task **const tasks) {
  bool success = false;

  ms_rwlock_lock_write(&q->lock);
  {
    success = q->queue.capacity - q->queue.count >= count;

    if(success) {
      for(unsigned i = 0; i < count; ++i) {
        ms_task **const t = ms_ring_enqueue(&q->queue);
        MS_ASSERT(t != NULL);

        *t = tasks[i];
      }
    }
  }
  ms_rwlock_unlock_write(&q->lock);

  return success;
}

ms_task *ms_task_queue_dequeue(ms_task_queue *const q) {
  ms_task *task = NULL;

  ms_rwlock_lock_read(&q->lock);
  {
    ms_task **const t = ms_ring_dequeue(&q->queue);

    if(t) { task = *t; }
  }
  ms_rwlock_unlock_read(&q->lock);

  return task;
}

