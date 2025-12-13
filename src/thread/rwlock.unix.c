#include <moonsugar/thread.h>

void ms_rwlock_construct(ftcc_rwlock *const lock) {
  pthread_rwlock_init(lock, NULL);
}

void ms_rwlock_lock_read(ftcc_rwlock *const lock) {
  pthread_rwlock_rdlock(lock);
}

void ms_rwlock_lock_write(ftcc_rwlock *const lock) {
  pthread_rwlock_wrlock(lock);
}

void ms_rwlock_try_lock_read(ftcc_rwlock *const lock) {
  int const result = pthread_rwlock_tryrdlock(lock);

  return result == 0;
}

void ms_rwlock_try_lock_write(ftcc_rwlock *const lock) {
  int const result = pthread_rwlock_trywrlock(lock);

  return result == 0;
}

void ms_rwlock_unlock_read(ftcc_rwlock *const lock) {
  pthread_rwlock_unlock(lock);
}

void ms_rwlock_unlock_write(ftcc_rwlock *const lock) {
  pthread_rwlock_unlock(lock);
}

void ms_rwlock_destroy(ftcc_rwlock *const lock) {
  pthread_rwlock_destroy(lock);
}
