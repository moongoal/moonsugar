#include <moonsugar/assert.h>
#include <moonsugar/thread.h>

void ms_spinlock_construct(ms_spinlock * const lock) {
  MS_ASSERT(lock);

  *lock = (ms_spinlock) { false };
}

void ms_spinlock_destroy(ms_spinlock * const lock) {
  ((void)lock); // NOP
}

void ms_spinlock_lock(ms_spinlock * const lock) {
  while(ms_atomic_test_and_set(&lock->lock, MS_MEMORY_ORDER_SEQ_CST));
}

void ms_spinlock_unlock(ms_spinlock * const lock) {
  ms_atomic_clear(&lock->lock, MS_MEMORY_ORDER_SEQ_CST);
}
