#include <string.h>
#include <moonsugar/uri.h>

#define SCHEME_SEP ':'
#define QUERY_SEP '?'
#define FRAG_SEP '#'

static char * find(char * s, char const c) {
  for(; *s != '\0'; ++s) {
    if(*s == c) {
      return s;
    }
  }

  return NULL;
}

ms_result ms_uri_decode(
  char const * const uri,
  char * const out_buff,
  ms_uri * const out_data
) {
  strcpy(out_buff, uri);

  char * const scheme_delim = find(out_buff, SCHEME_SEP);

  // Invalid URI means either
  // 1. no `scheme:path` structure
  // 2. scheme is empty
  if(scheme_delim == NULL || scheme_delim == out_buff) {
    return MS_RESULT_INVALID_ARGUMENT;
  }

  *scheme_delim = '\0';
  out_data->scheme = out_buff;
  out_data->path = scheme_delim + 1;

  char * const query_delim = find(out_data->path, QUERY_SEP);

  if(query_delim) {
    *query_delim = '\0';
    out_data->query = query_delim + 1;
  } else {
    out_data->query = NULL;
  }

  char * const frag_delim = find(
    out_data->query
    ? out_data->query
    : out_data->path,
    FRAG_SEP
  );

  if(frag_delim) {
    *frag_delim = '\0';
    out_data->fragment = frag_delim + 1;
  } else {
    out_data->fragment = NULL;
  }

  return MS_RESULT_SUCCESS;
}
