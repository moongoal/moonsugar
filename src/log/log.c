#include <memory.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <moonsugar/path.h>
#include <moonsugar/log.h>

#ifdef _WIN32
  #define localtime_r(t, tm) (localtime_s(tm, t) == 0 ? tm : NULL)
#endif // _WIN32

static ms_logger *g_logger_global = NULL;

static const char * get_level_text(const ms_log_level level) {
  switch(level) {
    #define GET_LEVEL(name) case MS_LOG_LEVEL_ ## name: return MS_QUOTE(name)
      GET_LEVEL(DEBUG);
      GET_LEVEL(WARNING);
      GET_LEVEL(TRACE);
      GET_LEVEL(INFO);
      GET_LEVEL(ERROR);
      GET_LEVEL(FATAL);
      GET_LEVEL(OFF);
    #undef GET_LEVEL
  }
}

static void format_log_message(
  char * const out_buff,
  const ms_log_level level,
  const char * const file,
  const char * const message
) {
  char file_name[1024];
  
  ms_path_get_filename(file, file_name, 1024);

  const char * const level_str = get_level_text(level);
  const time_t cur_time = time(NULL);
  struct tm cur_local_time;

  if(localtime_r(&cur_time, &cur_local_time) != NULL) {
    memset(&cur_local_time, 0, sizeof(struct tm));
  }

  sprintf(
    out_buff,
    "[%d:%d:%d %s] %s: %s\n",
    cur_local_time.tm_hour,
    cur_local_time.tm_min,
    cur_local_time.tm_sec,
    file_name, level_str, message
  );
}

void ms_log(const ms_log_level level, const char * const file, const char * const message) {
  if(g_logger_global && level <= g_logger_global->level) {
    ms_message log;

    switch(level) {
      case MS_LOG_LEVEL_OFF:
        return;

      #define SET_LOG(level, field) case level: log = g_logger_global->field; break
        SET_LOG(MS_LOG_LEVEL_DEBUG, debug);
        SET_LOG(MS_LOG_LEVEL_WARNING, warning);
        SET_LOG(MS_LOG_LEVEL_TRACE, trace);
        SET_LOG(MS_LOG_LEVEL_INFO, info);
        SET_LOG(MS_LOG_LEVEL_ERROR, error);
        SET_LOG(MS_LOG_LEVEL_FATAL, fatal);
      #undef SET_LOG
    }

    if(log != NULL) {
      char message_to_log[MS_LOG_MAX_MESSAGE_LENGTH];

      format_log_message(message_to_log, level, file, message);
      log(level, message_to_log, g_logger_global->ctx);
    }
  }
}

void ms_logf(ms_log_level level, const char * const file, const char * const format, ...) {
  if(g_logger_global && level <= g_logger_global->level) {
    va_list args;
    char message[MS_LOG_MAX_MESSAGE_LENGTH];

    va_start(args, format);
    vsprintf(message, format, args);
    va_end(args);

    ms_log(level, file, message);
  }
}

void ms_logger_set_global(ms_logger * const logger) {
  g_logger_global = logger;
}
