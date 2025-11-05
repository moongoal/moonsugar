/**
 * @file
 *
 * Path manipulation library.
 *
 * Most functions in this library only support forward-slash separators.
 * A conversion function is present to replace all backslashes with
 * forward slashes.
 */
#ifndef MS_PATH_H
#define MS_PATH_H

#include <moonsugar/api.h>

#define MS_PATH_SEP '/'
#define MS_EXT_SEP '.'

#define MS_PATH_WIN32_SEP '\\'
#define MS_PATH_WIN32_DRIVE_SEP ':'
#define MS_PATH_UNIX_SEP '/'

#ifdef __WIN32
  #define MS_PATH_NATIVE_SEP MS_PATH_WIN32_SEP
#else
  #define MS_PATH_NATIVE_SEP MS_PATH_UNIX_SEP
#endif

/**
 * Get the filename portion of a path.
 *
 * @param s The input path.
 * @param buff The output path.
 * @param buff_len The output buffer length, in characters.
 *
 * @return MS_RESULT_SUCCESS on success,
 *  MS_RESULT_LENGTH if the buffer is too small.
 */
ms_result MSAPI ms_path_get_filename(char const * const s, char * const buff, unsigned const buff_len);

/**
 * Get the parent portion of a path.
 *
 * @param s The input path.
 * @param buff The output path.
 * @param buff_len The output buffer length, in characters.
 *
 * @return MS_RESULT_SUCCESS on success,
 *  MS_RESULT_LENGTH if the buffer is too small.
 */
ms_result MSAPI ms_path_get_parent(char const * const s, char * const buff, unsigned const buff_len);

/**
 * Get the extension portion of a path.
 *
 * @param s The input path.
 * @param buff The output path.
 * @param buff_len The output buffer length, in characters.
 *
 * @return MS_RESULT_SUCCESS on success,
 *  MS_RESULT_LENGTH if the buffer is too small.
 */
ms_result MSAPI ms_path_get_extension(char const * const s, char * const buff, unsigned const buff_len);

/**
 * Normalize slashes in the path, turning all backslashes into forward slashes.
 *
 * @param s The input path.
 * @param buff The output path. This function assumes the output buffer is
 *  at least as large as the input path.
 */
void MSAPI ms_path_convert_slashes(char const * s, char * buff);

/**
 * Join two paths.
 *
 * @param s1 The first input path.
 * @param s2 The second input path.
 * @param buff The output path.
 * @param buff_len The output buffer length, in characters.
 *
 * @return MS_RESULT_SUCCESS on success,
 *  MS_RESULT_LENGTH if the buffer is too small.
 */
ms_result MSAPI ms_path_join(char const * const s1, char const * const s2, char * const buff, unsigned const buff_len);

#endif // MS_PATH_H
