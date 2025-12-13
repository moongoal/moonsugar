#include <moonsugar/log.h>
#include <moonsugar/assert.h>
#include <moonsugar/thread.h>

static bool initialized;
static pthread_mutexattr_t mutex_attrs;

static void init(void) {
  int const result = pthread_mutexattr_init(&mutex_attrs);

  if(result != 0) {
    ms_fatal("Unable to initialize concurrency backend.");
  }

  pthread_mutexattr_setprotocol(&mutex_attrs, PTHREAD_PRIO_NONE);
  pthread_mutexattr_setpshared(&mutex_attrs, PTHREAD_PROCESS_PRIVATE);
  pthread_mutexattr_setprotocol(&mutex_attrs, PTHREAD_MUTEX_RECURSIVE);

  initialized = true;
}

void ms_mutex_construct(ms_mutex *const m) {
  if(!initialized) {
    init();
  }

  int const result = pthread_mutex_init(m, &mutex_attrs);
  ((void)result);
  MS_ASSERT(result == 0);
}

void ms_mutex_lock(ms_mutex *const m) {
  pthread_mutex_lock(m);
}

bool ms_mutex_try_lock(ms_mutex *const m) {
  return pthread_mutex_trylock(m) == 0;
}

void ms_mutex_unlock(ms_mutex *const m) {
  pthread_mutex_unlock(m);
}

void ms_mutex_destroy(ms_mutex *const m) {
  pthread_mutex_destroy(m);
}
