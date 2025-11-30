/**
 * @file
 *
 * Library interface.
 */
#ifndef MS_API_H
#define MS_API_H

#include <stddef.h>

#define MSAPI

#define MSMALLOC __attribute__((malloc))
#define MSUSERET __attribute__((warn_unused_result))

#define ms_u8 unsigned char
#define ms_i8 char
#define ms_u16 unsigned short
#define ms_i16 short
#define ms_u32 unsigned
#define ms_i32 int
#define ms_u64 unsigned long long
#define ms_i64 long long
#define ms_f32 float
#define ms_f64 double

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

#endif // MS_API_H
