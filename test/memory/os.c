#include <moondance/test.h>
#include <moonsugar/memory.h>

MD_CASE(reserve) {
  void *const ptr = ms_reserve(1024);
  md_assert(ptr);
}

MD_CASE(commit) {
  void *const ptr = ms_reserve(1024);
  md_assert(ptr);

  bool const commit_result = ms_commit(ptr, 1024);
  md_assert(commit_result);
}

MD_CASE(release) {
  void *const ptr = ms_reserve(1024);
  md_assert(ptr);

  bool const commit_result = ms_commit(ptr, 1024);
  md_assert(commit_result);

  ms_release(ptr, 1024);
}

MD_CASE(decommit) {
  void *const ptr = ms_reserve(1024);
  md_assert(ptr);

  bool const commit_result = ms_commit(ptr, 1024);
  md_assert(commit_result);

  ms_decommit(ptr, 1024);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  md_add(&suite, commit);
  md_add(&suite, decommit);
  md_add(&suite, reserve);
  md_add(&suite, release);

  return md_run(argc, argv, &suite);
}

