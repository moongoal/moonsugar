/**
 * @file
 *
 * Multithreading interface.
 */
#ifndef MS_THREAD_H
#define MS_THREAD_H

#include <moonsugar/api.h>

/**
 * Maximum length of a thread name.
 */
#define MS_THREAD_NAME_MAX_LEN (256u)

/**
 * A native OS thread handle.
 */
typedef void* ms_thread;

/**
 * Thread main function.
 */
typedef void (*ms_thread_main)(void * const ctx);

typedef struct {
  /**
   * Thread routine.
   */
  ms_thread_main main;

  /**
   * Name of the thread or NULL.
   *
   * If provided, the name must be less than MS_THREAD_NAME_MAX_LEN characters long.
   */
  char const *name;

  /**
   * Thread routine context value.
   */
  void *ctx;
} ms_thread_description;

/**
 * Spawn a new thread.
 *
 * @param t The thread object to construct and start.
 * @param description The thread description.
 *
 * @return
 *  MS_RESULT_SUCESS on success
 *  MS_RESULT_INVALID_ARGUMENT if the description is invalid
 *  MS_RESULT_UNKNOWN the thread fails creation
 */
MSAPI ms_result ms_thread_spawn(
  ms_thread * const t,
  ms_thread_description const * const description
);

/**
 * Join a thread.
 *
 * @param t The thread.
 *
 * @return True if the thread was successfully joined, false otherwise.
 */
MSAPI bool ms_thread_join(ms_thread * const t);

/**
 * Yield execution of the current thread, allowing the scheduler to
 * run another.
 */
MSAPI void ms_thread_yield(void);

/**
 * Sleep for the given amout of time.
 *
 * @param count The amount of time to sleep.
 */
MSAPI void ms_thread_sleep(const ms_time count);

/**
 * @return The current thread name.
 */
MSAPI const char * ms_get_current_thread_name(void);

/**
 * Set the current thread name.
 *
 * @param name The thread name.
 */
MSAPI void ms_set_current_thread_name(char const * const name);

#ifdef MSAPI
  typedef struct {
    ms_atomic_flag in_use;
    ms_thread_main main;
    void * ctx;
    char name[MS_THREAD_NAME_MAX_LEN];
  } ms_thread_start_descriptor;

  ms_thread_start_descriptor * ms_thread_acquire_start_descriptor(void);
  void ms_thread_release_start_descriptor(ms_thread_start_descriptor * const d);
#endif // MSAPI

#ifdef _WIN32
  #include <Windows.h>

	typedef CRITICAL_SECTION ms_mutex;
#else
  #include <pthread.h>

	typedef pthread_mutex_t ms_mutex;
#endif

/**
 * Lock the mutex.
 *
 * @param m The mutex.
 */
MSINLINE inline static void ms_mutex_lock(ms_mutex *const m) {
#ifdef _WIN32
  EnterCriticalSection(m);
#else
  pthread_mutex_lock(m);
#endif
}

/**
 * Attempt locking the mutex.
 *
 * @param m The mutex.
 *
 * @return True if the mutex was locked; false if another thread
 *  already owns the mutex.
 */
MSINLINE inline static bool ms_mutex_try_lock(ms_mutex *const m) {
#ifdef _WIN32
  return TryEnterCriticalSection(m);
#else
  return pthread_mutex_trylock(m) == 0;
#endif
}

/**
 * Unlock the heap, allowing access from other threads.
 *
 * @param m The mutex.
 */
MSINLINE inline static void ms_mutex_unlock(ms_mutex *const m) {
#ifdef _WIN32
  LeaveCriticalSection(m);
#else
  pthread_mutex_unlock(m);
#endif
}

/**
 * Construct the mutex.
 *
 * @param m The mutex object to construct.
 */
#ifdef _WIN32
	MSINLINE inline static void ms_mutex_construct(ms_mutex *const m) { InitializeCriticalSection(m); }
#else
	MSAPI void ms_mutex_construct(ms_mutex *const m);
#endif

/**
 * Destroy the mutex.
 *
 * @param m The mutex.
 */
MSINLINE inline static void ms_mutex_destroy(ms_mutex *const m) {
#ifdef _WIN32
  DeleteCriticalSection(m);
#else
  pthread_mutex_destroy(m);
#endif
}

/**
 * A lock that spins on wait.
 */
typedef struct {
  /**
   * Clear when released, set when acquired.
   */
  MS_ALIGNED(MS_CACHE_LINE_SIZE) ms_atomic_flag lock;
} ms_spinlock;

MSAPI void ms_spinlock_construct(ms_spinlock * const lock);
MSAPI void ms_spinlock_destroy(ms_spinlock * const lock);
MSAPI void ms_spinlock_lock(ms_spinlock * const lock);
MSAPI void ms_spinlock_unlock(ms_spinlock * const lock);

#ifdef _WIN32
	/*
	 * A lock that allows multiple reader threads
	 * to acquire it concurrently, or one writer
	 * thread to acquire it exclusively.
	 */
	typedef SRWLOCK ms_rwlock;
	
	/**
	 * Construct a new rw lock.
	 *
	 * @param lock The lock.
	 */
	inline MSINLINE void ms_rwlock_construct(ms_rwlock *const lock) {
	  InitializeSRWLock(lock);
	}
	
	/**
	 * Lock an rw lock in read mode. Multiple threads can
	 * succeed acquiring the lock in this mode at the same time.
	 *
	 * @param lock The lock.
	 */
	inline MSINLINE void ms_rwlock_lock_read(ms_rwlock *const lock) {
	  AcquireSRWLockShared(lock);
	}
	
	/**
	 * Lock an rw lock in write mode. Only one thread can
	 * succeed acquire the lock in this mode at a given time.
	 *
	 * @param lock The lock.
	 */
	inline MSINLINE void ms_rwlock_lock_write(ms_rwlock *const lock) {
	  AcquireSRWLockExclusive(lock);
	}
	
	/**
	 * Try locking an rw lock in read mode. Multiple threads can
	 * succeed acquiring the lock in this mode at the same time.
	 *
	 * @param lock The lock.
	 *
	 * @return True if the lock was acquired, false if locking failed.
	 */
	inline MSINLINE bool ms_rwlock_try_lock_read(ms_rwlock *const lock) {
	  return TryAcquireSRWLockShared(lock) == TRUE;
	}
	
	/**
	 * Try locking an rw lock in write mode. Only one thread can
	 * succeed acquiring the lock in this mode at any given time.
	 *
	 * @param lock The lock.
	 *
	 * @return True if the lock was acquired, false if locking failed.
	 */
	inline MSINLINE bool ms_rwlock_try_lock_write(ms_rwlock *const lock) {
	  return TryAcquireSRWLockExclusive(lock) == TRUE;
	}
	
	/**
	 * Release an rw lock previously acquired in read mode.
	 *
	 * @param lock The lock.
	 */
	inline MSINLINE void ms_rwlock_unlock_read(ms_rwlock *const lock) {
	  ReleaseSRWLockShared(lock);
	}
	
	/**
	 * Release an rw lock previously acquired in write mode.
	 *
	 * @param lock The lock.
	 */
	inline MSINLINE void ms_rwlock_unlock_write(ms_rwlock *const lock) {
	  ReleaseSRWLockExclusive(lock);
	}
	
	/**
	 * Destroy an existing rwlock.
	 *
	 * @param lock The lock.
	 */
	inline MSINLINE void ms_rwlock_destroy(ms_rwlock *const lock) { ((void)lock); }
#else // UNIX
	/*
	 * A lock that allows multiple reader threads
	 * to acquire it concurrently, or one writer
	 * thread to acquire it exclusively.
	 */
	typedef pthread_rwlock_t ms_rwlock;
	
	/**
	 * Construct a new rw lock.
	 *
	 * @param lock The lock.
	 */
  void MSAPI ms_rwlock_construct(ftcc_rwlock *const lock) {
    pthread_rwlock_init(lock, NULL);
  }
  
	/**
	 * Lock an rw lock in read mode. Multiple threads can
	 * succeed acquiring the lock in this mode at the same time.
	 *
	 * @param lock The lock.
	 */
  void MSAPI ms_rwlock_lock_read(ftcc_rwlock *const lock) {
    pthread_rwlock_rdlock(lock);
  }
  
	/**
	 * Lock an rw lock in write mode. Only one thread can
	 * succeed acquire the lock in this mode at a given time.
	 *
	 * @param lock The lock.
	 */
  void MSAPI ms_rwlock_lock_write(ftcc_rwlock *const lock) {
    pthread_rwlock_wrlock(lock);
  }
  
	/**
	 * Try locking an rw lock in read mode. Multiple threads can
	 * succeed acquiring the lock in this mode at the same time.
	 *
	 * @param lock The lock.
	 *
	 * @return True if the lock was acquired, false if locking failed.
	 */
  void MSAPI ms_rwlock_try_lock_read(ftcc_rwlock *const lock) {
    int const result = pthread_rwlock_tryrdlock(lock);
  
    return result == 0;
  }
  
	/**
	 * Try locking an rw lock in write mode. Only one thread can
	 * succeed acquiring the lock in this mode at any given time.
	 *
	 * @param lock The lock.
	 *
	 * @return True if the lock was acquired, false if locking failed.
	 */
  void MSAPI ms_rwlock_try_lock_write(ftcc_rwlock *const lock) {
    int const result = pthread_rwlock_trywrlock(lock);
  
    return result == 0;
  }
  
	/**
	 * Release an rw lock previously acquired in read mode.
	 *
	 * @param lock The lock.
	 */
  void MSAPI ms_rwlock_unlock_read(ftcc_rwlock *const lock) {
    pthread_rwlock_unlock(lock);
  }
  
	/**
	 * Release an rw lock previously acquired in write mode.
	 *
	 * @param lock The lock.
	 */
  void MSAPI ms_rwlock_unlock_write(ftcc_rwlock *const lock) {
    pthread_rwlock_unlock(lock);
  }
  
	/**
	 * Destroy an existing rwlock.
	 *
	 * @param lock The lock.
	 */
  void MSAPI ms_rwlock_destroy(ftcc_rwlock *const lock) {
    pthread_rwlock_destroy(lock);
  }
#endif

#endif // MS_THREAD_H
