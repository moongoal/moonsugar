#include <stdlib.h>
#include <string.h>
#include <moonsugar/uri.h>

#define SCHEME_SEP ':'
#define QUERY_SEP '?'
#define FRAG_SEP '#'
#define PATH_SEP '/'
#define USER_SEP '@'
#define PORT_SEP ':'
#define PASS_SEP ':'

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
  char * next;

  // Invalid URI means either
  // 1. no `scheme:path` structure
  // 2. scheme is empty
  if(scheme_delim == NULL || scheme_delim == out_buff) {
    return MS_RESULT_INVALID_ARGUMENT;
  }

  *scheme_delim = '\0';
  out_data->scheme = out_buff;
  char * const hier = scheme_delim + 1;

  // Contains authority
  if(hier[0] == '/' && hier[1] == '/') {
    char * const authority = &hier[2];
    char * const host_delim = find(authority, USER_SEP);
    char * userinfo;

    if(host_delim != NULL) {
      *host_delim = '\0';

      userinfo = authority;
      out_data->host = host_delim + 1;
    } else {
      userinfo = NULL;
      out_data->host = authority;
    }

    if(userinfo != NULL) {
      char * const pass_delim = find(userinfo, PASS_SEP);

      out_data->user = userinfo;

      if(pass_delim != NULL) {
        *pass_delim = '\0';

        out_data->password = pass_delim + 1;
      } else {
        out_data->password = NULL;
      }
    } else {
      out_data->user = NULL;
      out_data->password = NULL;
    }

    next = out_data->host;
    char * const port_delim = find(next, PORT_SEP);

    if(port_delim != NULL) {
      *port_delim = '\0';

      char * const port = next = port_delim + 1;
      out_data->port = atoi(port);
    } else {
      out_data->port = -1;
    }

    char * const path_delim = find(next, PATH_SEP);

    if(path_delim != NULL) {
      *path_delim = '\0';
      next = out_data->path = path_delim + 1;
    } else {
      out_data->path = NULL;
      next = authority;
    }
  } else {
    out_data->user = NULL;
    out_data->password = NULL;
    out_data->host = NULL;
    out_data->port = -1;

    if(hier[0] != QUERY_SEP && hier[0] != FRAG_SEP) {
      next = out_data->path = hier;
    } else {
      out_data->path = NULL;
      next = hier;
    }
  }

  char * const query_delim = find(next, QUERY_SEP);

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

// static size_t estrlen(char const * s) {
//   size_t size;
// 
//   for(size = 0; *s != '\0'; ++s) {
//     
//   }
// 
//   return size;
// }
// 
// ms_result ms_uri_encode(
//   ms_uri const * const description,
//   char * const out_buff,
//   size_t * const inout_buff_len
// ) {
//   if(out_buff == NULL) {
//     size_t const required_part = (
//       strlen(description->scheme)
//       + 1
//       + strlen(description->path)
//     );
// 
//     size_t const query_part = (
//       description->query != NULL
//       ? 1 + sizeof(description->query)
//       : 0
//     );
// 
//     size_t const fragment_part = (
//       description->fragment != NULL
//       ? 1 + sizeof(description->fragment)
//       : 0
//     );
// 
//     *inout_buff_len = (
//       required_part
//       + query_part
//       + fragment_part
//     );
// 
//     return MS_RESULT_SUCCESS;
//   }
// }
