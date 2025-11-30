#include <moondance/test.h>

// Override ms_break()
#include <moonsugar/api.h>
#undef ms_break
#define ms_break() do { ctx.break_invoked = true; } while(false)

// Define testing logging API
#ifndef MSLIB
  #define MSLIB
#endif

#include <moonsugar/log.h>

typedef struct {
  bool break_invoked;
  bool log_message_invoked;
  ms_log_level level;
} test_context;

static test_context ctx;
static ms_logger logger;

void log_message(
  const ms_log_level level,
  const char * msg,
  void* logctx
) {
  ((void)msg);
  ((void)logctx);
  ctx.log_message_invoked = true;
  ctx.level = level;
}

static void each_setup(void *tctx) {
  ((void)tctx);
  memset(&ctx, 0, sizeof(test_context));
}

MD_CASE(fatal) {
  ms_fatal("test");

  md_assert(ctx.log_message_invoked);
  md_assert(ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_FATAL);
}

MD_CASE(fatalf) {
  ms_fatalf("test %d", 2);

  md_assert(ctx.log_message_invoked);
  md_assert(ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_FATAL);
}

MD_CASE(error) {
  ms_error("test");

  md_assert(ctx.log_message_invoked);
  md_assert(!ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_ERROR);
}

MD_CASE(errorf) {
  ms_errorf("test %f", 2.0f);

  md_assert(ctx.log_message_invoked);
  md_assert(!ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_ERROR);
}

MD_CASE(warn) {
  ms_warn("test");

  md_assert(ctx.log_message_invoked);
  md_assert(!ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_WARNING);
}

MD_CASE(warnf) {
  ms_warnf("test %d", 2);

  md_assert(ctx.log_message_invoked);
  md_assert(!ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_WARNING);
}

MD_CASE(info) {
  ms_info("test");

  md_assert(ctx.log_message_invoked);
  md_assert(!ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_INFO);
}

MD_CASE(infof) {
  ms_infof("test %d", 2);

  md_assert(ctx.log_message_invoked);
  md_assert(!ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_INFO);
}

MD_CASE(debug) {
  ms_debug("test");

  md_assert(ctx.log_message_invoked);
  md_assert(!ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_DEBUG);
}

MD_CASE(debugf) {
  ms_debugf("test %d", 2);

  md_assert(ctx.log_message_invoked);
  md_assert(!ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_DEBUG);
}

MD_CASE(trace) {
  ms_trace_test("test");

  md_assert(ctx.log_message_invoked);
  md_assert(!ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_TRACE);
}

MD_CASE(tracef) {
  ms_trace_testf("test %d", 2);

  md_assert(ctx.log_message_invoked);
  md_assert(!ctx.break_invoked);
  md_assert(ctx.level == MS_LOG_LEVEL_TRACE);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  logger.trace = log_message;
  logger.debug = log_message;
  logger.info = log_message;
  logger.warning = log_message;
  logger.error = log_message;
  logger.fatal = log_message;
  logger.level = MS_LOG_LEVEL_TRACE;

  ms_set_global(&logger);

  suite.user_ctx = &ctx;
  suite.each_setup = each_setup;

  md_add(&suite, fatal);
  md_add(&suite, fatalf);
  md_add(&suite, error);
  md_add(&suite, errorf);
  md_add(&suite, warn);
  md_add(&suite, warnf);
  md_add(&suite, info);
  md_add(&suite, infof);
  md_add(&suite, debug);
  md_add(&suite, debugf);
  md_add(&suite, trace);
  md_add(&suite, tracef);

  return md_run(argc, argv, &suite);
}
