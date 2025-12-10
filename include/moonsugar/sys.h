/**
 * @file
 *
 * System information querying.
 */
#ifndef MS_SYS_H
#define MS_SYS_H

#include <moonsugar/api.h>

#define MS_CPU_FEATURE_RSVD0_BIT (1ULL << 4) // Internal - do not use
#define MS_CPU_FEATURE_POPCNT_BIT (1ULL << 8)

#ifdef __x86_64__
  #define MS_CPU_FEATURE_SSE3_BIT (1ULL << 0)
  #define MS_CPU_FEATURE_SSSE3_BIT (1ULL << 1)
  #define MS_CPU_FEATURE_FMA_BIT (1ULL << 2)
  #define MS_CPU_FEATURE_CMPXCHG16B_BIT (1ULL << 3)
  #define MS_CPU_FEATURE_SSE4_1_BIT (1ULL << 5)
  #define MS_CPU_FEATURE_SSE4_2_BIT (1ULL << 6)
  #define MS_CPU_FEATURE_MOVBE_BIT (1ULL << 7)
  #define MS_CPU_FEATURE_AES_BIT (1ULL << 9)
  #define MS_CPU_FEATURE_AVX_BIT (1ULL << 10)
  #define MS_CPU_FEATURE_F16C_BIT (1ULL << 11)
  #define MS_CPU_FEATURE_RDRAND_BIT (1ULL << 12)
  #define MS_CPU_FEATURE_CMOV_BIT (1ULL << 13)
  // #define MS_CPU_FEATURE_PSN_BIT (1ULL << 14)
  #define MS_CPU_FEATURE_CLFLUSH_BIT (1ULL << 15)
  #define MS_CPU_FEATURE_MMX_BIT (1ULL << 16)
  #define MS_CPU_FEATURE_SSE_BIT (1ULL << 17)
  #define MS_CPU_FEATURE_SSE2_BIT (1ULL << 18)
#elif __arm64__
  #define MS_CPU_FEATURE_ATOMICS_BIT (1ULL << 0)
  #define MS_CPU_FEATURE_NEON_BIT (1ULL << 1)
#else
  #error Unsupported processor architecture.
#endif

typedef uint64_t ms_cpu_feature_flags;

typedef struct {
  /**
   * Enabled CPU features.
   */
  ms_cpu_feature_flags cpu_features;

  /**
   * Total physical memory size, in bytes.
   *
   * This value is UINT64_MAX when the amount of installed memory
   * is unavailable.
   */
  uint64_t memory_size;

  /**
   * Actual cache line size.
   */
  uint32_t cache_line_size;

  /**
   * OS memory allocation page size.
   */
  uint32_t page_size;

  /**
   * Minimum address alignment for OS allocated memroy.
   */
  uint32_t alloc_granularity;

  /**
   * Number of logical processors.
   */
  uint32_t proc_count;
} ms_sys_info;

/**
 * Return the system information.
 *
 * @return A pointer to the system information structure.
 */
MSAPI const ms_sys_info *ms_get_sys_info(void);

#ifdef MSLIB
/**
 * Update the sys info structure with CPUID information.
 *
 * @param result The structure to update.
 */
extern void ms_sys_update_with_cpuid(ms_sys_info *restrict const result);

/**
 * Update the sys info structure with OS information.
 *
 * @param result The structure to update.
 */
extern void ms_sys_update_with_os(ms_sys_info *restrict const result);
#endif // MSLIB

#endif // MS_SYS_H
