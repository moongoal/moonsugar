#include <moondance/test.h>
#include <moonsugar/uri.h>

MD_CASE(decode) {
  char uri[] = "my-id:abcd?this is a query#fragment";
  char out[sizeof(uri)];
  ms_uri data;

  ms_result const result = ms_uri_decode(uri, out, &data);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(data.scheme, "my-id") == 0);
  md_assert(strcmp(data.path, "abcd") == 0);
  md_assert(strcmp(data.query, "this is a query") == 0);
  md_assert(strcmp(data.fragment, "fragment") == 0);
}

MD_CASE(decode__no_scheme) {
  char uri[] = ":abcd?this is a query#fragment";
  char out[sizeof(uri)];
  ms_uri data;

  ms_result const result = ms_uri_decode(uri, out, &data);

  md_assert(result == MS_RESULT_INVALID_ARGUMENT);
}

MD_CASE(decode__empty) {
  char uri[] = "";
  char out[sizeof(uri)];
  ms_uri data;

  ms_result const result = ms_uri_decode(uri, out, &data);

  md_assert(result == MS_RESULT_INVALID_ARGUMENT);
}

MD_CASE(decode__no_query) {
  char uri[] = "my-id:abcd#fragment";
  char out[sizeof(uri)];
  ms_uri data;

  ms_result const result = ms_uri_decode(uri, out, &data);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(data.scheme, "my-id") == 0);
  md_assert(strcmp(data.path, "abcd") == 0);
  md_assert(data.query == NULL);
  md_assert(strcmp(data.fragment, "fragment") == 0);
}

MD_CASE(decode__no_fragment) {
  char uri[] = "my-id:abcd?this is a query";
  char out[sizeof(uri)];
  ms_uri data;

  ms_result const result = ms_uri_decode(uri, out, &data);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(data.scheme, "my-id") == 0);
  md_assert(strcmp(data.path, "abcd") == 0);
  md_assert(strcmp(data.query, "this is a query") == 0);
  md_assert(data.fragment == NULL);
}

MD_CASE(decode__no_path) {
  char uri[] = "my-id:?this is a query#fragment";
  char out[sizeof(uri)];
  ms_uri data;

  ms_result const result = ms_uri_decode(uri, out, &data);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(data.scheme, "my-id") == 0);
  md_assert(strcmp(data.path, "") == 0);
  md_assert(strcmp(data.query, "this is a query") == 0);
  md_assert(strcmp(data.fragment, "fragment") == 0);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  md_add(&suite, decode);
  md_add(&suite, decode__no_scheme);
  md_add(&suite, decode__empty);
  md_add(&suite, decode__no_query);
  md_add(&suite, decode__no_fragment);
  md_add(&suite, decode__no_path);

  return md_run(argc, argv, &suite);
}
