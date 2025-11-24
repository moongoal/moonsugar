#include <stdlib.h>
#include <string.h>
#include <moondance/test.h>
#include <moonsugar/config.h>

struct parse_full_ctx {
  int n;
};

static ms_result on_kv_entry(void * const ctx, char const * const k, char const * const v) {
  ((void)ctx);

  if(strcmp(k, "my_key") == 0 && strcmp(v, "this is a value") == 0) {
    return MS_RESULT_SUCCESS;
  }

  return MS_RESULT_INVALID_ARGUMENT;
}

static ms_result on_k_entry(void * const ctx, char const * const k, char const * const v) {
  ((void)ctx);

  if(strcmp(k, "no_value_key") == 0 && v == NULL) {
    return MS_RESULT_SUCCESS;
  }

  return MS_RESULT_INVALID_ARGUMENT;
}

static ms_result on_parse_full(void * const ctx, char const * const k, char const * const v) {
  struct parse_full_ctx * const c = ctx;

  if(strcmp(k, "add") == 0) {
    c->n += atoi(v);

    return MS_RESULT_SUCCESS;
  }

  return MS_RESULT_INVALID_ARGUMENT;
}

MD_CASE(parse_line_kv__ok) {
  char config[] = "my_key this is a value ";

  md_assert(ms_config_parse_line(config, on_kv_entry, NULL) == MS_RESULT_SUCCESS);
}

MD_CASE(parse_line_kv__not_ok) {
  // Note the extra space
  char config[] = "my_key this is a  value ";

  md_assert(ms_config_parse_line(config, on_kv_entry, NULL) == MS_RESULT_INVALID_ARGUMENT);
}

MD_CASE(parse_line__key_only) {
  char config[] = "no_value_key   ";

  md_assert(ms_config_parse_line(config, on_k_entry, NULL) == MS_RESULT_SUCCESS);
}

MD_CASE(parse_line__comment) {
  char config[] = "  # no_value_key   ";

  md_assert(ms_config_parse_line(config, on_kv_entry, NULL) == MS_RESULT_SUCCESS);
}

MD_CASE(parse_line__empty) {
  char config[] = "\t\n\r";

  md_assert(ms_config_parse_line(config, on_kv_entry, NULL) == MS_RESULT_SUCCESS);
}

MD_CASE(parse) {
  struct parse_full_ctx ctx = { 0 };
  char config[] = "\t\n\r# Let this be a comment\nadd 1\n add    3\n     add 4   ";

  md_assert(ms_config_parse(config, on_parse_full, &ctx) == MS_RESULT_SUCCESS);
  md_assert(ctx.n == 8);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  md_add(&suite, parse_line_kv__ok);
  md_add(&suite, parse_line_kv__not_ok);
  md_add(&suite, parse_line__key_only);
  md_add(&suite, parse_line__comment);
  md_add(&suite, parse_line__empty);
  md_add(&suite, parse);

  return md_run(argc, argv, &suite);
}
