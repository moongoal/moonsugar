/*
 * Internal thread interface.
 */
#ifndef MS_THREAD_INTERNAL_H
#define MS_THREAD_INTERNAL_H

#include <moonsugar/thread.h>

typedef struct {
  ms_atomic_flag in_use;
  ms_thread_main main;
  void * ctx;
  char name[MS_THREAD_NAME_MAX_LEN];
} ms_thread_start_descriptor;

ms_thread_start_descriptor * ms_thread_acquire_start_descriptor(void);
void ms_thread_release_start_descriptor(ms_thread_start_descriptor * const d);

#endif // MS_THREAD_INTERNAL_H
