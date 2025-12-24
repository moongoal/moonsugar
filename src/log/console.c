#include <stdio.h>
#include <string.h>
#include <moonsugar/log.h>

static void log_message(
  const ms_log_level log_level,
  const char * const message,
  void * const ctx
) {
  ((void)log_level);
  ((void)ctx);

  fwrite(message, sizeof(char), strlen(message), stderr);
}

void ms_logger_console_construct(ms_logger * const logger) {
  *logger = (ms_logger) {
    MS_LOG_LEVEL_INFO,
    log_message, // warning
    log_message, // debug
    log_message, // trace
    log_message, // info
    log_message, // error
    log_message, // fatal
    NULL // ctx
  };
}

void ms_logger_console_destroy(ms_logger *logger) { ((void)logger); }
