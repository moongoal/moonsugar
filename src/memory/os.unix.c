#include <sys/mman.h>
#include <moonsugar/memory.h>
#include <moonsugar/util.h>
#include <moonsugar/assert.h>
#include <moonsugar/sys.h>

void* ms_reserve(size_t count) {
  count = ms_align_sz(count, ms_get_sys_info()->page_size);

  void * const result = mmap(
    NULL, // addr
    count, // length
    PROT_NONE, // prot
    MAP_PRIVATE | MAP_ANONYMOUS, // flags
    -1, // fd
    0 // offset
  );

   MS_ASSERT(result != MAP_FAILED);

   return result;
}

void ms_release(void * const ptr, size_t count) {
  count = ms_align_sz(count, ms_get_sys_info()->page_size);

  munmap(ptr, count);
}

bool ms_commit(void * ptr, size_t count) {
  uint64_t const page_size = ms_get_sys_info()->page_size;
  uint8_t* const ptr_end = ms_align_ptr((uint8_t*)ptr + count, page_size);

  ptr = ms_align_back_ptr(ptr, page_size);
  count = ptr_end - (uint8_t*)ptr;

#ifdef HAS_REMAP
  // TODO: Implement this correctly - good luck
  void * const result = mremap(
    ptr, // addr
    count, // length
    PROT_READ | PROT_WRITE, // prot
    MAP_PRIVATE | MAP_ANONYMOUS, // flags
    -1, // fd
    0 // offset
  );
#else
  munmap(ptr, count);

  void * const result = mmap(
    ptr, // addr
    count, // length
    PROT_READ | PROT_WRITE, // prot
    MAP_PRIVATE | MAP_ANONYMOUS, // flags
    -1, // fd
    0 // offset
  );
#endif // HAS_REMAP

  return result != MAP_FAILED;
}

void ms_decommit(void * const ptr, size_t count) {
  count = ms_align_sz(count, ms_get_sys_info()->page_size);

#ifdef HAS_REMAP
  // TODO: Implement this correctly - good luck
  void * const result = mremap(
    ptr, // addr
    count, // length
    PROT_NONE, // prot
    MAP_PRIVATE | MAP_ANONYMOUS, // flags
    -1, // fd
    0 // offset
  );
#else
  munmap(ptr, count);

  void * const result = mmap(
    ptr, // addr
    count, // length
    PROT_NONE, // prot
    MAP_PRIVATE | MAP_ANONYMOUS, // flags
    -1, // fd
    0 // offset
  );
#endif // HAS_REMAP

  MS_ASSERT(result != MAP_FAILED);
}
