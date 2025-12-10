#include <moondance/test.h>
#include <moonsugar/util.h>

MD_CASE(is_power2) {
  md_assert(ms_is_power2(1u));
  md_assert(ms_is_power2(2u));
  md_assert(ms_is_power2(4u));
  md_assert(ms_is_power2(1024u));
  md_assert(ms_is_power2(0x10000u));

  md_assert(!ms_is_power2(0u));
  md_assert(!ms_is_power2(3u));
  md_assert(!ms_is_power2(0xffffu));
}

MD_CASE(is_multiple) {
  md_assert(ms_is_multiple(4, 2));
  md_assert(!ms_is_multiple(4, 3));
  md_assert(!ms_is_multiple(2, 4));
}

MD_CASE(align_sz) {
  md_assert(ms_align_sz(10, 16) == 16);
  md_assert(ms_align_sz(16, 16) == 16);
}

MD_CASE(align_ptr) {
  md_assert(ms_align_ptr((void*)1, 16) == (void*)16);
  md_assert(ms_align_ptr((void*)16, 16) == (void*)16);
}

MD_CASE(align_back_sz) {
  md_assert(ms_align_back_sz(10, 16) == 0);
  md_assert(ms_align_back_sz(17, 16) == 16);
  md_assert(ms_align_back_sz(16, 16) == 16);
}

MD_CASE(align_back_ptr) {
  md_assert(ms_align_back_ptr((void*)10, 16) == (void*)0);
  md_assert(ms_align_back_ptr((void*)17, 16) == (void*)16);
  md_assert(ms_align_back_ptr((void*)16, 16) == (void*)16);
}

MD_CASE(choose) {
  md_assert(ms_choose(1, 2, true) == 1);
  md_assert(ms_choose(1, 2, false) == 2);
}

MD_CASE(min) {
  md_assert(ms_min(1, 2) == 1);
  md_assert(ms_min(1, 1) == 1);
  md_assert(ms_min(2, 1) == 1);
}

MD_CASE(min_signed) {
  md_assert(ms_min_signed(1, 2) == 1);
  md_assert(ms_min_signed(1, 1) == 1);
  md_assert(ms_min_signed(2, 1) == 1);

  md_assert(ms_min_signed(-1, 2) == -1);
  md_assert(ms_min_signed(1, -1) == -1);
}

MD_CASE(max) {
  md_assert(ms_max(1, 2) == 2);
  md_assert(ms_max(1, 1) == 1);
  md_assert(ms_max(2, 1) == 2);
}

MD_CASE(max_signed) {
  md_assert(ms_max_signed(1, 2) == 2);
  md_assert(ms_max_signed(1, 1) == 1);
  md_assert(ms_max_signed(2, 1) == 2);

  md_assert(ms_max_signed(1, -2) == 1);
  md_assert(ms_max_signed(-1, 1) == 1);
}

MD_CASE(test) {
  md_assert(ms_test(2, 1) == 0);
  md_assert(ms_test(2, 2) != 0);
  md_assert(ms_test(2, 3) == 0);
  md_assert(ms_test(2, 4) == 0);
}

int main(int argc, char** argv) {
  md_suite suite = md_suite_create();

  md_add(&suite, is_power2);
  md_add(&suite, is_multiple);
  md_add(&suite, align_sz);
  md_add(&suite, align_ptr);
  md_add(&suite, align_back_sz);
  md_add(&suite, align_back_ptr);
  md_add(&suite, choose);
  md_add(&suite, min);
  md_add(&suite, max);
  md_add(&suite, min_signed);
  md_add(&suite, max_signed);
  md_add(&suite, test);

  return md_run(argc, argv, &suite);
}
