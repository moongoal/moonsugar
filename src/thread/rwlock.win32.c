#include <moonsugar/thread.h>

void ms_rwlock_construct(ms_rwlock *const lock) {
  InitializeSRWLock(lock);
}

void ms_rwlock_lock_read(ms_rwlock *const lock) {
  AcquireSRWLockShared(lock);
}

void ms_rwlock_lock_write(ms_rwlock *const lock) {
  AcquireSRWLockExclusive(lock);
}

bool ms_rwlock_try_lock_read(ms_rwlock *const lock) {
  return TryAcquireSRWLockShared(lock) == TRUE;
}

bool ms_rwlock_try_lock_write(ms_rwlock *const lock) {
  return TryAcquireSRWLockExclusive(lock) == TRUE;
}

void ms_rwlock_unlock_read(ms_rwlock *const lock) {
  ReleaseSRWLockShared(lock);
}

void ms_rwlock_unlock_write(ms_rwlock *const lock) {
  ReleaseSRWLockExclusive(lock);
}

void ms_rwlock_destroy(ms_rwlock *const lock) { ((void)lock); }
