#include <moonsugar/thread.h>

ms_result ms_rwlock_construct(ms_rwlock *const lock) {
  pthread_rwlock_init(lock, NULL);

  return MS_RESULT_SUCCESS;
}

void ms_rwlock_lock_read(ms_rwlock *const lock) {
  pthread_rwlock_rdlock(lock);
}

void ms_rwlock_lock_write(ms_rwlock *const lock) {
  pthread_rwlock_wrlock(lock);
}

bool ms_rwlock_try_lock_read(ms_rwlock *const lock) {
  int const result = pthread_rwlock_tryrdlock(lock);

  return result == 0;
}

bool ms_rwlock_try_lock_write(ms_rwlock *const lock) {
  int const result = pthread_rwlock_trywrlock(lock);

  return result == 0;
}

void ms_rwlock_unlock_read(ms_rwlock *const lock) {
  pthread_rwlock_unlock(lock);
}

void ms_rwlock_unlock_write(ms_rwlock *const lock) {
  pthread_rwlock_unlock(lock);
}

void ms_rwlock_destroy(ms_rwlock *const lock) {
  pthread_rwlock_destroy(lock);
}
