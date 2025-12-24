#include <stdio.h>
#include <string.h>
#include <moonsugar/log.h>

static void log_message(
  const ms_log_level log_level,
  const char * const message,
  void * const ctx
) {
  ((void)log_level);
  FILE * const f = ctx;

  fwrite(message, sizeof(char), strlen(message), f);
}

bool ms_logger_file_construct(ms_logger * const logger, const char * const path) {
  FILE * const f = fopen(path, "w");

  if(f != NULL) {
    *logger = (ms_logger) {
      MS_LOG_LEVEL_INFO,
      log_message, // warning
      log_message, // debug
      log_message, // trace
      log_message, // info
      log_message, // error
      log_message, // fatal
      f // ctx
    };
  }

  return f != NULL;
}

void ms_logger_file_destroy(ms_logger * logger) {
  if(logger->ctx) {
    fclose(logger->ctx);
    logger->ctx = NULL;
  }
}
