#include <string.h>
#include <moondance/test.h>
#include <moonsugar/path.h>

MD_CASE(get_filename) {
  char input[] = "/my/file/name";
  char expected_output[] = "name";
  char buff[256];

  ms_result const result = ms_path_get_filename(input, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(get_filename__truncated) {
  char input[] = "/my/file/name";
  char buff[3];

  ms_result const result = ms_path_get_filename(input, buff, 3);

  md_assert(result == MS_RESULT_LENGTH);
}

MD_CASE(get_parent) {
  char input[] = "/my/file/name";
  char expected_output[] = "/my/file";
  char buff[256];

  ms_result const result = ms_path_get_parent(input, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(get_parent__truncated) {
  char input[] = "/my/file/name";
  char buff[3];

  ms_result const result = ms_path_get_parent(input, buff, 3);

  md_assert(result == MS_RESULT_LENGTH);
}

MD_CASE(get_parent__win32) {
  char input[] = "c:/my/file/name";
  char expected_output[] = "c:/my/file";
  char buff[256];

  ms_result const result = ms_path_get_parent(input, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(get_parent__win32_root_1) {
  char input[] = "c:/";
  char expected_output[] = "c:";
  char buff[256];

  ms_result const result = ms_path_get_parent(input, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(get_parent__win32_root_2) {
  char input[] = "d:";
  char expected_output[] = "d:";
  char buff[256];

  ms_result const result = ms_path_get_parent(input, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(get_parent__win32_root_2_truncated) {
  char input[] = "d:";
  char buff[2];

  ms_result const result = ms_path_get_parent(input, buff, 2);

  md_assert(result == MS_RESULT_LENGTH);
}

MD_CASE(convert_slashes) {
  char input[] = "d:\\my\\path";
  char expected_output[] = "d:/my/path";
  char buff[256];

  ms_path_convert_slashes(input, buff);

  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(convert_slashes__empty) {
  char input[] = "";
  char expected_output[] = "";
  char buff[256] = "<invalid>";

  ms_path_convert_slashes(input, buff);

  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(join) {
  char input1[] = "a";
  char input2[] = "b";
  char expected_output[] = "a/b";
  char buff[256] = "<invalid>";

  ms_result const result = ms_path_join(input1, input2, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(join__first_ends_with_slash) {
  char input1[] = "a/";
  char input2[] = "b";
  char expected_output[] = "a/b";
  char buff[256] = "<invalid>";

  ms_result const result = ms_path_join(input1, input2, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(join__second_begins_with_slash) {
  char input1[] = "a";
  char input2[] = "/b";
  char expected_output[] = "a/b";
  char buff[256] = "<invalid>";

  ms_result const result = ms_path_join(input1, input2, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(join__two_slashes) {
  char input1[] = "a/";
  char input2[] = "/b";
  char expected_output[] = "a/b";
  char buff[256] = "<invalid>";

  ms_result const result = ms_path_join(input1, input2, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(join__first_too_long) {
  char input1[] = "aaaaa/";
  char input2[] = "/b";
  char buff[4] = "";

  ms_result const result = ms_path_join(input1, input2, buff, 4);

  md_assert(result == MS_RESULT_LENGTH);
}

MD_CASE(join__second_too_long) {
  char input1[] = "a/";
  char input2[] = "/bbbbbbbb";
  char buff[4] = "";

  ms_result const result = ms_path_join(input1, input2, buff, 4);

  md_assert(result == MS_RESULT_LENGTH);
}

MD_CASE(join__first_empty) {
  char input1[] = "";
  char input2[] = "/b";
  char expected_output[] = "b";
  char buff[256] = "<invalid>";

  ms_result const result = ms_path_join(input1, input2, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(join__second_empty) {
  char input1[] = "a/";
  char input2[] = "";
  char expected_output[] = "a/";
  char buff[256] = "<invalid>";

  ms_result const result = ms_path_join(input1, input2, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(get_extension) {
  char input[] = "/my/file/name.ext";
  char expected_output[] = "ext";
  char buff[256];

  ms_result const result = ms_path_get_extension(input, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(get_extension__multiple) {
  char input[] = "/my/file/name.ext.2";
  char expected_output[] = "ext.2";
  char buff[256];

  ms_result const result = ms_path_get_extension(input, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(get_extension__no_separator) {
  char input[] = "name.ext.2";
  char expected_output[] = "ext.2";
  char buff[256];

  ms_result const result = ms_path_get_extension(input, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(strcmp(buff, expected_output) == 0);
}

MD_CASE(get_extension__empty) {
  char input[] = "";
  char buff[256];

  ms_result const result = ms_path_get_extension(input, buff, 256);

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(buff[0] == '\0');
}

MD_CASE(get_extension__too_long) {
  char input[] = "a.b";
  char buff[1];

  ms_result const result = ms_path_get_extension(input, buff, 1);

  md_assert(result == MS_RESULT_LENGTH);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  md_add(&suite, get_filename);
  md_add(&suite, get_filename__truncated);
  md_add(&suite, get_parent);
  md_add(&suite, get_parent__truncated);
  md_add(&suite, get_parent__win32);
  md_add(&suite, get_parent__win32_root_1);
  md_add(&suite, get_parent__win32_root_2);
  md_add(&suite, get_parent__win32_root_2_truncated);
  md_add(&suite, convert_slashes);
  md_add(&suite, convert_slashes__empty);
  md_add(&suite, join);
  md_add(&suite, join__first_ends_with_slash);
  md_add(&suite, join__second_begins_with_slash);
  md_add(&suite, join__two_slashes);
  md_add(&suite, join__first_too_long);
  md_add(&suite, join__second_too_long);
  md_add(&suite, join__first_empty);
  md_add(&suite, join__second_empty);
  md_add(&suite, get_extension);
  md_add(&suite, get_extension__multiple);
  md_add(&suite, get_extension__no_separator);
  md_add(&suite, get_extension__empty);
  md_add(&suite, get_extension__too_long);

  return md_run(argc, argv, &suite);
}
