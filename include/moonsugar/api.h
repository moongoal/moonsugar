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
  MS_RESULT_INVALID_ARGUMENT
} ms_result;

typedef enum {
  MS_MEMORY_ORDER_RELAXED = __ATOMIC_RELAXED,
  MS_MEMORY_ORDER_ACQUIRE = __ATOMIC_ACQUIRE,
  MS_MEMORY_ORDER_RELEASE = __ATOMIC_RELEASE,
  MS_MEMORY_ORDER_ACQ_REL = __ATOMIC_ACQ_REL,
  MS_MEMORY_ORDER_SEQ_CST = __ATOMIC_SEQ_CST
} ms_memory_order;

#define ms_atomic_load __atomic_load_n
#define ms_atomic_store __atomic_store
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

#endif // MS_API_H
