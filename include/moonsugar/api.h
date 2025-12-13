/**
 * @file
 *
 * Library interface.
 */
#ifndef MS_API_H
#define MS_API_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define MSAPI

#define MSMALLOC __attribute__((malloc))
#define MSUNUSED __attribute__((unused))
#define MSUSERET __attribute__((warn_unused_result))
#define MSINLINE __attribute__((always_inline))
#define MSUNREACHABLE __builtin_unreachable()

#define MS_ALIGNED(x) __attribute__((aligned (x)))

#define MSFORMAT(format_index, first_arg) \
  __attribute__((format(printf, format_index, first_arg)))

#define MS_QUOTE_(x) #x
#define MS_QUOTE(x) MS_QUOTE_(x)

#define MS_CONCAT_(a, b) a##b
#define MS_CONCAT(a, b) MS_CONCAT_(a, b)

#define MS_CHECK(expr, on_error) \
  do { \
    ms_result const ms__check_result = (expr); \
    if(ms__check_result != MS_RESULT_SUCCESS) { \
      on_error; \
    } \
  } while(false)

#define MS_CKRET(expr) MS_CHECK((expr), return ms__check_result)

typedef enum {
  /**
   * The operation was successful.
   */
  MS_RESULT_SUCCESS,

  /**
   * The operation failed because the provided buffer is not long enough
   * to contain the output. Any output buffers contain invalid data and
   * must not be used.
   */
  MS_RESULT_LENGTH,

  /**
   * An unknown error has occurred.
   */
  MS_RESULT_UNKNOWN,

  /**
   * One or more arguments are invalid.
   */
  MS_RESULT_INVALID_ARGUMENT,

	/**
   * Memory allocation failed.
   */
	MS_RESULT_MEMORY,

  /**
   * The container is full.
   */
	MS_RESULT_FULL,

  /**
   * The container is empty.
   */
	MS_RESULT_EMPTY,

  /**
   * No more resources of this type available.
   */
  MS_RESULT_RESOURCE_LIMIT,

  /**
   * Resource access denied.
   */
  MS_RESULT_ACCESS,

  /**
   * Resource not found.
   */
  MS_RESULT_NOT_FOUND,

  /**
   * End of file reached.
   */
  MS_RESULT_EOF,

  /**
   * The task is scheduled.
   */
  MS_RESULT_SCHEDULED,

  /**
   * The requested operation is not supported.
   */
  MS_RESULT_UNSUPPORTED
} ms_result;

typedef enum {
  MS_MEMORY_ORDER_RELAXED = __ATOMIC_RELAXED,
  MS_MEMORY_ORDER_ACQUIRE = __ATOMIC_ACQUIRE,
  MS_MEMORY_ORDER_RELEASE = __ATOMIC_RELEASE,
  MS_MEMORY_ORDER_ACQ_REL = __ATOMIC_ACQ_REL,
  MS_MEMORY_ORDER_SEQ_CST = __ATOMIC_SEQ_CST
} ms_memory_order;

#define ms_atomic_load __atomic_load_n
#define ms_atomic_store __atomic_store_n
#define ms_atomic_exchange __atomic_exchange_n
#define ms_atomic_compare_exchange_weak(ptr, expected, desired, success, failure) __atomic_compare_exchange_n((ptr), (expected), (desired), true, (success), (failure))
#define ms_atomic_compare_exchange_strong(ptr, expected, desired, success, failure) __atomic_compare_exchange_n((ptr), (expected), (desired), false, (success), (failure))
#define ms_atomic_add_fetch __atomic_add_fetch
#define ms_atomic_sub_fetch __atomic_sub_fetch
#define ms_atomic_and_fetch __atomic_and_fetch
#define ms_atomic_or_fetch __atomic_or_fetch
#define ms_atomic_xor_fetch __atomic_xor_fetch
#define ms_atomic_fetch_add __atomic_fetch_add
#define ms_atomic_fetch_sub __atomic_fetch_sub
#define ms_atomic_fetch_and __atomic_fetch_and
#define ms_atomic_fetch_or __atomic_fetch_or
#define ms_atomic_fetch_xor __atomic_fetch_xor
#define ms_atomic_test_and_set __atomic_test_and_set
#define ms_atomic_clear __atomic_clear
#define ms_thread_fence __atomic_thread_fence

#define MS_ATOMIC(x) x
#define ms_atomic_flag MS_ATOMIC(bool)

#define MS_THREAD_LOCAL __thread

/**
 * The raw handle type.
 */
typedef uint32_t ms_handle;

/**
 * Declare a new handle type.
 *
 * @param name The handle type name.
 */
#define MS_HDECL(name) typedef struct { ms_handle raw; } name

/**
 * Numerically compare two handles.
 *
 * @param h1 The first handle.
 * @param h2 The second handle.
 *
 * @return `0` if the two are the same handle,
 *  `>0` if h2 is larger in value than h1,
 *  `<0` otherwise.
 */
#define ms_hrcmp(h1, h2) ((h2) - (h1))
#define ms_hcmp(h1, h2) (ms_hrcmp((h1).raw, (h2).raw))

/**
 * Compare two handles for equality.
 *
 * @param h1 The first handle.
 * @param h2 The second handle.
 *
 * @return True if the two are the same handle,
 *  false otherwise.
 */
#define ms_hreq(h1, h2) ((h2) == (h1))
#define ms_heq(h1, h2) (ms_hreq((h1).raw, (h2).raw))

/**
 * Test whether a handle is valid.
 *
 * @param h The handle to test.
 *
 * @return True if the handle is valid, false if not.
 */
#define ms_hraw_is_valid(h) ((h) != MS_HINVALID)
#define ms_his_valid(h) (ms_hraw_is_valid((h).raw))

/**
 * The invalid handle value.
 */
#define MS_HINVALID ((ms_handle)UINT32_MAX)

#define ms_break() __builtin_trap()

#if defined(__x86_64__)
    #include <emmintrin.h>

    #define ms_pause() _mm_pause()
    #define MS_CACHE_LINE_SIZE (64u)
#elif defined(__arm64__)
    #define ms_pause() __asm__("yield" :::)
    #define MS_CACHE_LINE_SIZE (128u)
#else
    #error Unsupported platform.
#endif

/**
 * Timestamp expressed in nanoseconds.
 */
typedef uint64_t ms_time;

/**
 * Infinitely long time period.
 */
#define MS_TIME_INFINITY ((ms_time)UINT64_MAX)

/**
 * Converts a timestamp into milliseconds.
 */
#define ms_time_to_us(t) ((ms_time)(t) / 1000llu)

/**
 * Converts a timestamp into milliseconds.
 */
#define ms_time_to_ms(t) ((ms_time)(t) / 1000000llu)

/**
 * Create a timestamp from milliseconds.
 */
#define ms_time_from_ms(t) ((ms_time)(t) * 1000000llu)

/**
 * Create a timestamp from nanoseconds.
 */
#define ms_time_from_ns(t) ((ms_time)(t))

#endif // MS_API_H
