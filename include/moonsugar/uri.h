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
   * The user of a URI.
   * This field is NULL when not present.
   */
  char * user;

  /**
   * The host of a URI.
   * This field is NULL when not present.
   */
  char * host;

  /**
   * The port of a URI.
   * This field is NULL when not present.
   */
  char * port;

  /**
   * The path part of a URI.
   * This field is NULL when not present.
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

/**
 * Encode a URI string.
 *
 * @param description The URI description.
 * @param out_buff The output buffer string.
 * @param out_buff_len
 *  If `out_buff` is NULL, this is a pointer to the memory
 *  that will receive the minimum required length for `out_buff`.
 *  If `out_buff` is not NULL, this is a pointer to the
 *  length of `out_buff`.
 *
 * @retrun
 *  MS_RESULT_SUCCESS on success.
 */
ms_result MSAPI ms_uri_encode(
  ms_uri const * const description,
  char * const out_buff,
  size_t * const inout_buff_len
);

#endif // MS_URI_H
