#include <sys/types.h>
#include <sys/sysctl.h>
#include <moonsugar/platform.h>

void ms_sys_update_with_os(ms_sys_info *const result) {
  int64_t cache_line_size = 0;
  size_t cache_line_size_len = sizeof(cache_line_size);

  int64_t memory_size = 0;
  size_t memory_size_len = sizeof(memory_size);

  int64_t page_size = 0;
  size_t page_size_len = sizeof(page_size);

  int32_t proc_count = 0;
  size_t proc_count_len = sizeof(page_size);

  sysctlbyname("hw.cachelinesize", &cache_line_size, &cache_line_size_len, NULL, 0);
  sysctlbyname("hw.memsize", &memory_size, &memory_size_len, NULL, 0);
  sysctlbyname("hw.pagesize", &page_size, &page_size_len, NULL, 0);
  sysctlbyname("hw.ncpu", &proc_count, &proc_count_len, NULL, 0);

  result->cache_line_size = cache_line_size;
  result->memory_size = memory_size;
  result->page_size = page_size;
  result->proc_count = proc_count;
  result->alloc_granularity = page_size; // mmap allocates page-aligned
  result->cpu_features = 0;

#ifdef __ARM_FEATURE_ATOMICS
  result->cpu_features |= MS_CPU_FEATURE_ATOMICS_BIT;
#endif // __ARM_FEATURE_ATOMICS

  result->cpu_features |= MS_CPU_FEATURE_POPCNT_BIT;
  result->cpu_features |= MS_CPU_FEATURE_NEON_BIT;
}

