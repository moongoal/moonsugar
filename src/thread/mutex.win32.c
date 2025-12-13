#include <moonsugar/thread.h>

void ms_mutex_lock(ms_mutex *const m) {
  EnterCriticalSection(m);
}

bool ms_mutex_try_lock(ms_mutex *const m) {
  return TryEnterCriticalSection(m);
}

void ms_mutex_unlock(ms_mutex *const m) {
  LeaveCriticalSection(m);
}

void ms_mutex_construct(ms_mutex *const m) {
  InitializeCriticalSection(m);
}

void ms_mutex_destroy(ms_mutex *const m) {
  DeleteCriticalSection(m);
}
