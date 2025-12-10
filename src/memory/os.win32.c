#include <Windows.h>
#include <moonsugar/util.h>
#include <moonsugar/memory.h>
#include <moonsugar/sys.h>

void* ms_reserve(size_t count) {
  count = ms_align_sz(count, ms_get_sys_info()->page_size);

  return VirtualAlloc(
    NULL,
    count,
    MEM_RESERVE,
    PAGE_READWRITE
  );
}

void ms_release(void * ptr, size_t count) {
  uint64_t const page_size = ms_get_sys_info()->page_size;
  uint8_t* const ptr_end = ms_align_ptr((uint8_t*)ptr + count, page_size);

  ptr = ms_align_back_ptr(ptr, page_size);
  count = ptr_end - (uint8_t*)ptr;

  VirtualFree(ptr, (SIZE_T)count, MEM_RELEASE);
}

bool ms_commit(void * ptr, size_t count) {
  uint64_t const page_size = ms_get_sys_info()->page_size;
  uint8_t* const ptr_end = ms_align_ptr((uint8_t*)ptr + count, page_size);

  ptr = ms_align_back_ptr(ptr, page_size);
  count = ptr_end - (uint8_t*)ptr;

  return VirtualAlloc(
    ptr,
    count,
    MEM_COMMIT,
    PAGE_READWRITE
  ) != NULL;
}

void ms_decommit(void * ptr, size_t count) {
  uint64_t const page_size = ms_get_sys_info()->page_size;
  uint8_t* const ptr_end = ms_align_ptr((uint8_t*)ptr + count, page_size);

  ptr = ms_align_back_ptr(ptr, page_size);
  count = ptr_end - (uint8_t*)ptr;

  VirtualFree(ptr, count, MEM_DECOMMIT);
}
