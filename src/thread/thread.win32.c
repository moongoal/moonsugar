#include <string.h>
#include <Windows.h>
#include <moonsugar/util.h>
#include <moonsugar/log.h>
#include <moonsugar/thread.h>

#define DEFAULT_STACK_SIZE 0
#define UNKNOWN_THREAD_NAME "<Unknown Thread>"

static DWORD WINAPI thread_main(void * const param) {
  ms_thread_start_descriptor * const gdesc = param;
  ms_thread_start_descriptor desc = *gdesc;

  ms_set_current_thread_name(desc.name);

  // Data copied locally, descriptor no more needed
  ms_atomic_clear(&gdesc->in_use, MS_MEMORY_ORDER_SEQ_CST);

  ms_debugf("Thread %s has started.", ms_get_current_thread_name());
  desc.main(desc.ctx);
  ms_debugf("Thread %s has terminated.", ms_get_current_thread_name());

  return 0;
}

ms_result ms_thread_spawn(
  ms_thread *const t,
  ms_thread_description const * const description
) {
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

  *t = CreateThread(
    NULL,
    DEFAULT_STACK_SIZE,
    thread_main,
    descriptor,
    0,
    NULL
  );

  if(*t == NULL) {
    ms_error("Unable to create new thread.");

    return MS_RESULT_UNKNOWN;
  }

  return MS_RESULT_SUCCESS;
}

bool ms_thread_join(ms_thread *const t) {
  DWORD const result = WaitForSingleObject(*t, INFINITE);

  switch(result) {
    case WAIT_OBJECT_0:
      CloseHandle(*t);
      return true;

    default:
      ms_error("Error while attempting to join thread.");

    // Fall-through
    case WAIT_TIMEOUT:
      return false;
  }
}

void ms_thread_yield(void) { SwitchToThread(); }
void ms_thread_sleep(ms_time const count) { Sleep(ms_max(1, ms_time_to_ms(count))); }

