#include <moondance/test.h>
#include <moonsugar/api.h>

MD_CASE(types) {
  md_assert(sizeof(ms_u8) == 1);
  md_assert(sizeof(ms_i8) == 1);

  md_assert(sizeof(ms_u16) == 2);
  md_assert(sizeof(ms_i16) == 2);

  md_assert(sizeof(ms_u32) == 4);
  md_assert(sizeof(ms_i32) == 4);

  md_assert(sizeof(ms_u64) == 8);
  md_assert(sizeof(ms_i64) == 8);

  md_assert(sizeof(ms_f32) == 4);
  md_assert(sizeof(ms_f64) == 8);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  md_add(&suite, types);

  return md_run(argc, argv, &suite);
}
