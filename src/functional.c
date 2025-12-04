#include <string.h>
#include <moonsugar/functional.h>

bool ms_equals_u64(void const * a, void const * b) {
  return *(uint64_t*)a == *(uint64_t*)b;
}

bool ms_equals_u32(void const * a, void const * b) {
  return *(uint32_t*)a == *(uint32_t*)b;
}

bool ms_equals_cstr(void const * a, void const * b) {
  return strcmp(*(char const **)a, *(char const **)b) == 0;
}

bool ms_none_test_max_u64(void const * x) {
  return *(uint64_t*)x == UINT64_MAX;
}

bool ms_none_test_max_u32(void const * x) {
  return *(uint32_t*)x == UINT32_MAX;
}

bool ms_none_test_ptr(void const * x) {
  return *(void**)x == NULL;
}

void ms_none_set_max_u64(void * x) {
  *(uint64_t*)x = UINT64_MAX;
}

void ms_none_set_max_u32(void * x) {
  *(uint32_t*)x = UINT32_MAX;
}

void ms_none_set_ptr(void * x) {
  *(void**)x = NULL;
}

bool ms_equals_handle(void const * a, void const * b) {
  return *(ms_handle*)a == *(ms_handle*)b;
}

bool ms_none_test_handle(void const * x) {
  return !ms_hraw_is_valid(*(ms_handle*)x);
}

void ms_none_set_handle(void * x) {
  *(ms_handle*)x = MS_HINVALID;
}

