#include <sys/mman.h>
#include <moonsugar/memory.h>
#include <moonsugar/util.h>
#include <moonsugar/assert.h>

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

bool ms_commit(void * const ptr, size_t count) {
  count = ms_align_sz(count, ms_get_sys_info()->page_size);

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
