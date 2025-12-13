/*
 * Multithreading interface.
 */
#ifndef MS_THREAD_H
#define MS_THREAD_H

#include <moonsugar/api.h>
#include <moonsugar/memory.h>
#include <moonsugar/containers/ring.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <pthread.h>
#endif

#define MS_THREAD_NAME_MAX_LEN (256u)

typedef void* ms_thread; // Thread handle
typedef void (*ms_thread_main)(void * const ctx); // Thread main routine pointer

typedef struct {
  ms_thread_main main; // Main routine
  char const *name; // Can be NULL
  void *ctx; // Context value to pass to the main thread routine
} ms_thread_description;

MSAPI ms_result ms_thread_spawn(ms_thread * const t, ms_thread_description const * const description);
MSAPI bool ms_thread_join(ms_thread * const t); // Returns true on success
MSAPI void ms_thread_yield(void);
MSAPI void ms_thread_sleep(const ms_time count);
MSAPI const char * ms_get_current_thread_name(void);
MSAPI void ms_set_current_thread_name(char const * const name);

#ifdef _WIN32
	typedef CRITICAL_SECTION ms_mutex;
#else
	typedef pthread_mutex_t ms_mutex;
#endif

MSAPI void ms_mutex_construct(ms_mutex *const m);
MSAPI void ms_mutex_destroy(ms_mutex *const m);
MSAPI void ms_mutex_lock(ms_mutex *const m);
MSAPI bool ms_mutex_try_lock(ms_mutex *const m); // Returns true on success
MSAPI void ms_mutex_unlock(ms_mutex *const m);

/*
 * A lock that spins on wait.
 */
typedef struct {
  MS_ALIGNED(MS_CACHE_LINE_SIZE) ms_atomic_flag lock; // Set when acquired
} ms_spinlock;

MSAPI void ms_spinlock_construct(ms_spinlock * const lock);
MSAPI void ms_spinlock_destroy(ms_spinlock * const lock);
MSAPI void ms_spinlock_lock(ms_spinlock * const lock);
MSAPI void ms_spinlock_unlock(ms_spinlock * const lock);

/*
 * A lock that allows multiple reader threads
 * to acquire it concurrently, or one writer
 * thread to acquire it exclusively.
 */
#ifdef _WIN32
  typedef SRWLOCK ms_rwlock;
#else
	typedef pthread_rwlock_t ms_rwlock;
#endif

MSAPI void ms_rwlock_construct(ms_rwlock *const lock);
MSAPI void ms_rwlock_destroy(ms_rwlock *const lock);

MSAPI void ms_rwlock_lock_read(ms_rwlock *const lock);
MSAPI void ms_rwlock_lock_write(ms_rwlock *const lock);

MSAPI bool ms_rwlock_try_lock_read(ms_rwlock *const lock); // Returns true if the lock becomes acquired
MSAPI bool ms_rwlock_try_lock_write(ms_rwlock *const lock); // Returns true if the lock becomes acquired
MSAPI void ms_rwlock_unlock_read(ms_rwlock *const lock);
MSAPI void ms_rwlock_unlock_write(ms_rwlock *const lock);

typedef void (*ms_task_handler)(void *ctx);
typedef struct ms_task ms_task;

struct ms_task {
  ms_task_handler handler; // The task execution routine
  ms_task *parent; // Parent task - can be NULL. Do not edit after acquire
  void *ctx; // Handler context value
  MS_ATOMIC(uint16_t) unsatisfied_dependencies; // Count of unsatisfied dependencies
};

/**
 * Task queue. This component is intended to let one or more requestor
 * threads schedule work to be executed on a set of worker threads.
 */
typedef struct {
  ms_task *store; // Task store
  ms_rwlock lock; // Access synchronization
  ms_ring queue; // Task store allocation
} ms_task_queue;

typedef struct {
  ms_allocator allocator;
  uint32_t capacity; // Max number of tasks
} ms_task_queue_description;

MSAPI ms_result ms_task_queue_construct(ms_task_queue *const q, ms_task_queue_description const * const description);
MSAPI void ms_task_queue_destroy(ms_task_queue *const q);
MSAPI bool ms_task_queue_enqueue(ms_task_queue *const q, ms_task *const task); // Returns false if the queue on failure
MSAPI bool ms_task_queue_enqueue_many(ms_task_queue *const q, unsigned const count, ms_task **const task); // Returns true if all tasks were allocated, false if none was
MSAPI MSUSERET ms_task *ms_task_queue_dequeue(ms_task_queue *const q); // Returns the task or NULL if none available

#endif // MS_THREAD_H
