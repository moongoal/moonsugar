#include <string.h>
#include <moonsugar/assert.h>
#include <moonsugar/thread.h>

ms_result ms_rwlock_construct(ms_rwlock *const lock) {
  memset(lock, 0, sizeof(ms_rwlock));

  InitializeSRWLock(&lock->lock);
  lock->event = CreateEvent(NULL, TRUE, TRUE, NULL);

  if(lock->event == NULL) {
    return MS_RESULT_UNKNOWN;
  }

  ms_atomic_store(&lock->writers_waiting, 0, MS_MEMORY_ORDER_SEQ_CST);

  return MS_RESULT_SUCCESS;
}

void ms_rwlock_destroy(ms_rwlock *const lock) {
  if(lock->event != NULL) {
    CloseHandle(lock->event);
    lock->event = NULL;
  }
}

void ms_rwlock_lock_read(ms_rwlock *const lock) {
  int writers_waiting;

  do {
    writers_waiting = ms_atomic_load(&lock->writers_waiting, MS_MEMORY_ORDER_ACQUIRE);

    if(writers_waiting > 0) {
      DWORD const wait_result = WaitForSingleObject(lock->event, INFINITE);
      MS_ASSERT(wait_result == WAIT_OBJECT_0);
    }
  } while(writers_waiting > 0);

  AcquireSRWLockShared(&lock->lock);
}

void ms_rwlock_lock_write(ms_rwlock *const lock) {
  int const writers_waiting = ms_atomic_fetch_add(&lock->writers_waiting, 1, MS_MEMORY_ORDER_SEQ_CST);

  AcquireSRWLockExclusive(&lock->lock);

  if(writers_waiting == 0) {
    ResetEvent(&lock->event);
  }
}

bool ms_rwlock_try_lock_read(ms_rwlock *const lock) {
  uint32_t writers_waiting = ms_atomic_load(&lock->writers_waiting, MS_MEMORY_ORDER_ACQUIRE);

  if(writers_waiting > 0) {
    return false;
  }

  return TryAcquireSRWLockShared(&lock->lock) == TRUE;
}

bool ms_rwlock_try_lock_write(ms_rwlock *const lock) {
  int const writers_waiting = ms_atomic_fetch_add(&lock->writers_waiting, 1, MS_MEMORY_ORDER_SEQ_CST);
  bool const acquired = TryAcquireSRWLockExclusive(&lock->lock) == TRUE;

  if(acquired && writers_waiting == 0) {
    ResetEvent(&lock->event);
  }

  return acquired;
}

void ms_rwlock_unlock_read(ms_rwlock *const lock) {
  ReleaseSRWLockShared(&lock->lock);
}

void ms_rwlock_unlock_write(ms_rwlock *const lock) {
  ReleaseSRWLockExclusive(&lock->lock);

  int const writers_waiting = ms_atomic_sub_fetch(&lock->writers_waiting, 1, MS_MEMORY_ORDER_SEQ_CST);
  MS_ASSERT(writers_waiting >= 0);

  if(writers_waiting == 0) {
    SetEvent(lock->event);
  }
}
