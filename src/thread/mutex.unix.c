#include <moonsugar/log.h>
#include <moonsugar/assert.h>
#include <moonsugar/thread.h>

static initialized;
static pthread_mutexattr_t mutex_attrs;

static void init() {
  int const result = pthread_mutexattr_init(&g_ftcc_backend.mutex_attrs);

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

  int const result FTUNUSED = pthread_mutex_init(m, &mutex_attrs);
  FT_ASSERT(result == 0);
}
