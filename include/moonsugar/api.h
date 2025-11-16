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
