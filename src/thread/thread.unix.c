#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <moonsugar/log.h>
#include <moonsugar/util.h>
#include <moonsugar/thread-internal.h>

#define UNKNOWN_THREAD_NAME "<Unnamed thread>"

static bool initialized;
static pthread_attr_t thread_attrs;

static void init(void) {
  pthread_attr_init(&thread_attrs);

  initialized = true;
}

static void *thread_main(void *const param) {
  ms_thread_start_descriptor * const gdesc = param;
  ms_thread_start_descriptor desc = *gdesc;

  ms_set_current_thread_name(desc.name);

  // Data copied locally, descriptor no more needed
  ms_atomic_clear(&gdesc->in_use, MS_MEMORY_ORDER_SEQ_CST);

  ms_debugf("Thread %s has started.", ms_get_current_thread_name());
  desc.main(desc.ctx);
  ms_debugf("Thread %s has terminated.", ms_get_current_thread_name());

  return NULL;
}

ms_result ms_thread_spawn(
  ms_thread *const t,
  ms_thread_description const * const description
) {
  if(!initialized) {
    init();
  }

  ms_thread_start_descriptor * const descriptor = ms_thread_acquire_start_descriptor();

  descriptor->main = description->main;
  descriptor->ctx = description->ctx;

  if(description->name != NULL) {
    if(strlen(description->name) >= MS_THREAD_NAME_MAX_LEN) {
      return MS_RESULT_INVALID_ARGUMENT;
    }

    strcpy(descriptor->name, description->name);
  } else {
    strcpy(descriptor->name, UNKNOWN_THREAD_NAME);
  }

  int const result = pthread_create(
    (pthread_t *)t,
    &thread_attrs,
    thread_main,
    descriptor
  );

  if(result != 0) {
    // Thread creation failed, ownership of `desc` remains within this function
    ms_thread_release_start_descriptor(descriptor);
    ms_errorf("Unable to create new thread. Pthread returned error %d.", result);

    return MS_RESULT_UNKNOWN;
  }

  return MS_RESULT_SUCCESS;
}

bool ms_thread_join(ms_thread *const t) {
  int const result = pthread_join((pthread_t)*t, NULL);

  if(result != 0) {
    ms_errorf("Thread failed joining with error %d.", result);
  }

  return result == 0;
}

void ms_thread_yield(void) { sched_yield(); }
void ms_thread_sleep(ms_time const count) { usleep(ms_max(1, ms_time_to_us(count))); }

