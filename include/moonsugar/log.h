/**
 * @file
 *
 * Logger interface.
 */
#ifndef MS_LOGGER_H
#define MS_LOGGER_H

#include <moonsugar/api.h>

#define MS_LOG_MAX_MESSAGE_LENGTH (4096u)

typedef enum {
  /**
   * Logging is disabled.
   */
  MS_LOG_LEVEL_OFF,

  /**
   * Fatal error. The application will terminate.
   */
  MS_LOG_LEVEL_FATAL,

  /**
   * Non-fatal log message. The application will not terminate
   * but may misbehave.
   */
  MS_LOG_LEVEL_ERROR,

  /**
   * Warning log message. The application has encountered a
   * non-standard situation but has recovered and will not
   * mis-behave.
   */
  MS_LOG_LEVEL_WARNING,

  /**
   * Informational log message. A standard event has happened.
   */
  MS_LOG_LEVEL_INFO,

  /**
   * Debug log message. Contains information to help during
   * debugging.
   */
  MS_LOG_LEVEL_DEBUG,

  /**
   * Trace log message. Contains tracing information for select
   * components.
   */
  MS_LOG_LEVEL_TRACE
} ms_log_level;

/**
 * Message logging function.
 *
 * @param level The log message level.
 * @param message The message to be logged.
 * @param ctx The logger context.
 */
typedef void (*ms_message)(
  const ms_log_level level,
  const char * const message,
  void * const ctx
);

/**
 * An object capable of logging events.
 */
typedef struct {
  /**
   * The logging level.
   *
   * Log messages with level higher than this
   * will be discarded.
   */
  ms_log_level level;

  ms_message warning;
  ms_message debug;
  ms_message trace;
  ms_message info;
  ms_message error;
  ms_message fatal;

  /**
   * Logger internal context.
   */
  void *ctx;
} ms_logger;

/**
 * Log a message.
 *
 * @param level The log message level.
 * @param file The name of the file producing the message.
 * @param message The message to log.
 */
MSAPI void ms_log(
  const ms_log_level level,
  const char * const file,
  const char * const message
);

/**
 * Log a formatted message (format is same as printf).
 *
 * @param level The log message level.
 * @param file The name of the file producing the message.
 * @param format The message format to log.
 * @param ... The arguments to be interpreted according to format.
 */
MSAPI void ms_logf(
  const ms_log_level level,
  const char * const file,
  const char * const format,
  ...
) MSFORMAT(3, 4);

/**
 * Set the global logger.
 *
 * @param logger The new global logger. Can be null.
 */
MSAPI void ms_set_global(ms_logger * const restrict logger);

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

#ifdef MSLIB
  #define ms_trace_test(message) ms_log(MS_LOG_LEVEL_TRACE, __FILE__, (message))
  #define ms_trace_testf(format, ...) ms_logf(MS_LOG_LEVEL_TRACE, __FILE__, (format), __VA_ARGS__)
#endif

/**
 * Construct a new file logger.
 *
 * @param logger The logger to initialize.
 * @param path The path to the log file.
 *
 * @return True if the logger was correctly initialised. False otherwise.
 */
MSAPI bool ms_file_construct(ms_logger * const logger, const char * const path);

/**
 * Destroy a file logger.
 *
 * @param logger The logger to destroy.
 */
MSAPI void ms_file_destroy(ms_logger * const logger);

/**
 * Construct a new console logger.
 *
 * @param logger The logger to initialize.
 */
MSAPI void ms_console_construct(ms_logger * const logger);

/**
 * Destroy a console logger.
 *
 * @param logger The logger to destroy.
 */
MSAPI void ms_console_destroy(ms_logger * const logger);

#endif // MS_LOGGER_H

