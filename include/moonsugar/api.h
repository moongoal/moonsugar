/**
 * @file
 *
 * Library interface.
 */
#ifndef MS_API_H
#define MS_API_H

#define MSAPI

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
  MS_RESULT_LENGTH
} ms_result;


#endif // MS_API_H
