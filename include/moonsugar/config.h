/**
 * @file
 *
 * Text-based configuration.
 *
 * This interface provides a parser for text-based configuration data. A sample
 * configuration string is the following:
 *
 * ```
 *  # This is a sample configuration file
 *  # This is a comment
 *
 *  this_is_a_key this is a value
 *  this_is_a_key_without_value
 * ```
 *
 *  Blank lines and spacing at the beginning of each line are ignored. Lines starting
 *  with a hash symbol ("#") represent comments and are ignored. All other lines
 *  are entries. Each entry is made of a key and an optional value. No newlines can
 *  be inserted in keys or values. Keys and values are separated by one or more space
 *  characters. Configuration files are encoded as UTF-8.
 */
#ifndef MS_CONFIG_H
#define MS_CONFIG_H

#include <moonsugar/api.h>

#define MS_CONFIG_MAX_LINE_LEN (4096u)

/**
 * Callback invoked for every entry of configuration data.
 *
 * @param ctx The context value as passed to the parse function.
 * @param key The key of the entry.
 * @param value The value of the entry. NULL if no value
 *  was provided.
 *
 * @return
 *  MS_RESULT_SUCCESS on success
 *  any other error code on failure.
 */
typedef int (*ms_config_clbk)(void * const ctx, char const * const key, char const * const value);

/**
 * Parse text configuration.
 *
 * @param ctx The context value to pass to the callback.
 * @param config The configuration.
 * @param on_entry The callback to invoke for every configuration entry.
 *
 * @return
 *  MS_RESULT_SUCCESS on success
 *  MS_RESULT_PARSE on parse error
 *  The error as returned by the callback otherwise.
 */
ms_result MSAPI ms_config_parse(
  char const * const config,
  ms_config_clbk const on_entry,
  void * const ctx
);

/**
 * Parse a single line of text configuration.
 *
 * @param ctx The context value to pass to the callback.
 * @param config The configuration.
 * @param on_entry The callback to invoke if the line represents an entry.
 *
 * @return
 *  MS_RESULT_SUCCESS on success
 *  MS_RESULT_PARSE on parse error
 *  The error as returned by the callback otherwise.
 */
ms_result MSAPI ms_config_parse_line(
  char const * const line,
  ms_config_clbk const on_entry,
  void * const ctx
);

#endif // MS_CONFIG_H
