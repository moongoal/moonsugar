#include <string.h>
#include <moonsugar/config.h>

static char *find_non_blank(char *s) {
  for(char c = *s; c != '\0'; ++s, c = *s) {
    if(
      c != ' '
      && c != '\t'
      && c != '\n'
      && c != '\r'
    ) {
      return s;
    }
  }

  return NULL;
}

static char *find_blank(char *s) {
  for(char c = *s; c != '\0'; ++s, c = *s) {
    if(
      c == ' '
      || c == '\t'
      || c == '\n'
      || c == '\r'
    ) {
      return s;
    }
  }

  return NULL;
}

static void trim_right(char *s) {
  char *p;

  for(p = s; *p != '\0'; ++p);

  for(--p; p >= s && (*p == '\n' || *p == '\r' || *p == ' ' || *p == '\t'); --p) {
    *p = '\0';
  }
}

ms_result ms_config_parse_line(char const * const config, ms_config_clbk const on_entry, void * const ctx) {
  char line[MS_CONFIG_MAX_LINE_LEN];
  strncpy(line, config, MS_CONFIG_MAX_LINE_LEN);

  char * const key = find_non_blank(line);

  if(key != NULL) {
    if(*key == '#') {
      return MS_RESULT_SUCCESS;
    }

    char * const sep = find_blank(key);
    char * value = NULL;

    if(sep != NULL) {
      value = find_non_blank(sep);
      *sep = '\0';

      if(value != NULL) {
        trim_right(value);
      }
    }

    return on_entry(ctx, key, value);
  }
  
  return MS_RESULT_SUCCESS;
}

ms_result ms_config_parse(char const * const config, ms_config_clbk const on_entry, void * const ctx) {
  char line[MS_CONFIG_MAX_LINE_LEN] = "";
  unsigned i, j;

  for(i = j = 0; config[i] != '\0'; ++i) {
    if(config[i] == '\n' || config[i] == '\r') {
      line[j] = '\0';
      j = 0;

      ms_result const r = ms_config_parse_line(line, on_entry, ctx);

      if(r != MS_RESULT_SUCCESS) {
        return r;
      }
    } else {
      line[j++] = config[i];
    }
  }

  return ms_config_parse_line(line, on_entry, ctx);
}
