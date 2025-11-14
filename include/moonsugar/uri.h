/**
 * @file
 *
 * Uniform Resource Identifier interface.
 */
#ifndef MS_URI_H

#include <moonsugar/api.h>

/**
 * @def MS_URI_MAX_LEN
 *
 * Maximum URI length in characters.
 */
#define MS_URI_MAX_LEN (4096u)

/**
 * Description of a URI.
 */
typedef struct {
  /**
   * The scheme part of a URI.
   * This field is always present and non-empty.
   */
  char * scheme;

  /**
   * The path part of a URI.
   * This field is always present and can be empty.
   */
  char * path;

  /**
   * The query part of a URI.
   * This field is NULL when not present.
   */
  char * query;

  /**
   * The fragment part of a URI.
   * This field is NULL when not present.
   */
  char * fragment;
} ms_uri;

/**
 * Decode a URI string.
 *
 * This function performs no memory allocation and
 * returns into `out_data` a set of pointers into
 * `out_buff` representing the different portions
 * of the URI.
 *
 * @param uri The input URI string.
 * @param out_buff The output buffer, which must be
 *  of the same length as `uri`.
 * @param out_data The output URI structured data.
 *
 * @return
 *  MS_RESULT_SUCCESS on success.
 *  MS_RESULT_INVALID_ARGUMENT if the URI is invalid.
 */
ms_result MSAPI ms_uri_decode(
  char const * const uri,
  char * const out_buff,
  ms_uri * const out_data
);

#endif // MS_URI_H
