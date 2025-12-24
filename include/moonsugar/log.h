/*
 * Logger interface.
 *
 * Multi-level logging interface allows logging messages with or without printf-style formatting.
 *
 * A global logger must be set to use the general, macro-based interface, or individual functions
 * can be called with local loggers.
 *
 * Tracing information does not have a general interface because it is intended to be selectively
 * enabled for components during debugging. An example of how to enable it is found below.
 */
#ifndef MS_LOGGER_H
#define MS_LOGGER_H

#include <moonsugar/api.h>

#define MS_LOG_MAX_MESSAGE_LENGTH (4096u)

typedef enum {
  MS_LOG_LEVEL_OFF,
  MS_LOG_LEVEL_FATAL, // Application will terminate
  MS_LOG_LEVEL_ERROR, // Application will not terminate but may misbehave
  MS_LOG_LEVEL_WARNING, // Application had issue but has recovered
  MS_LOG_LEVEL_INFO, // No issue
  MS_LOG_LEVEL_DEBUG, // Information to assist debugging
  MS_LOG_LEVEL_TRACE // Detailed debugging information - per component
} ms_log_level;

typedef void (*ms_message)(const ms_log_level level, const char * const message, void * const ctx);

typedef struct {
  ms_log_level level; // Discard messages with level higher than this

  // Handlers
  ms_message warning;
  ms_message debug;
  ms_message trace;
  ms_message info;
  ms_message error;
  ms_message fatal;

  void *ctx; // Logger internal context
} ms_logger;

MSAPI void ms_log(const ms_log_level level, const char * const file, const char * const message);
MSAPI void ms_logf(const ms_log_level level, const char * const file, const char * const format, ...) MSFORMAT(3, 4);
MSAPI void ms_logger_set_global(ms_logger * const logger); // Set global logger - pass NULL to disable

#define ms_fatal(message) do { ms_log(MS_LOG_LEVEL_FATAL, __FILE__, (message)); ms_break(); } while(false)
#define ms_error(message) ms_log(MS_LOG_LEVEL_ERROR, __FILE__, (message))
#define ms_warn(message) ms_log(MS_LOG_LEVEL_WARNING, __FILE__, (message))
#define ms_info(message) ms_log(MS_LOG_LEVEL_INFO, __FILE__, (message))
#define ms_debug(message) ms_log(MS_LOG_LEVEL_DEBUG, __FILE__, (message))

#define ms_fatalf(format, ...) do { ms_logf(MS_LOG_LEVEL_FATAL, __FILE__, (format), __VA_ARGS__); ms_break(); } while(false)
#define ms_errorf(format, ...) ms_logf(MS_LOG_LEVEL_ERROR, __FILE__, (format), __VA_ARGS__)
#define ms_warnf(format, ...) ms_logf(MS_LOG_LEVEL_WARNING, __FILE__, (format), __VA_ARGS__)
#define ms_infof(format, ...) ms_logf(MS_LOG_LEVEL_INFO, __FILE__, (format), __VA_ARGS__)
#define ms_debugf(format, ...) ms_logf(MS_LOG_LEVEL_DEBUG, __FILE__, (format), __VA_ARGS__)

// Example of how to enable tracing
#ifdef MSLIB
  #define ms_trace_test(message) ms_log(MS_LOG_LEVEL_TRACE, __FILE__, (message))
  #define ms_trace_testf(format, ...) ms_logf(MS_LOG_LEVEL_TRACE, __FILE__, (format), __VA_ARGS__)
#endif

// File logger
MSAPI bool ms_logger_file_construct(ms_logger * const logger, const char * const path);
MSAPI void ms_logger_file_destroy(ms_logger * const logger);

// Console logger
MSAPI void ms_logger_console_construct(ms_logger * const logger);
MSAPI void ms_logger_console_destroy(ms_logger * const logger);

#endif // MS_LOGGER_H

