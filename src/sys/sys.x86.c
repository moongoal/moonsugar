#include <moonsugar/log.h>
#include <moonsugar/util.h>
#include <moonsugar/sys.h>

#define CPUID_FEATURE_SSE3_BIT (1llu << 0)
#define CPUID_FEATURE_SSSE3_BIT (1llu << 9)
#define CPUID_FEATURE_FMA_BIT (1llu << 12)
#define CPUID_FEATURE_CMPXCHG16B_BIT (1llu << 13)
#define CPUID_FEATURE_SSE4_1_BIT (1llu << 19)
#define CPUID_FEATURE_SSE4_2_BIT (1llu << 20)
#define CPUID_FEATURE_MOVBE_BIT (1llu << 22)
#define CPUID_FEATURE_POPCNT_BIT (1llu << 23)
#define CPUID_FEATURE_AES_BIT (1llu << 25)
#define CPUID_FEATURE_AVX_BIT (1llu << 28)
#define CPUID_FEATURE_F16C_BIT (1llu << 29)
#define CPUID_FEATURE_RDRAND_BIT (1llu << 30)
#define CPUID_FEATURE_CMOV_BIT (1llu << 15)
#define CPUID_FEATURE_CLFLUSH_BIT (1llu << 19)
#define CPUID_FEATURE_MMX_BIT (1llu << 23)
#define CPUID_FEATURE_SSE_BIT (1llu << 25)
#define CPUID_FEATURE_SSE2_BIT (1llu << 26)

static void update_with_cpuid_80000006h(
  ms_sys_info * const restrict out,
  uint32_t const ebx,
  uint32_t const ecx,
  uint32_t const edx
) {
  ((void)ebx);
  ((void)edx);

  out->cache_line_size = ms_choose(
    ecx & 0xffu,
    MS_CACHE_LINE_SIZE,
    ecx != 0
  );
}

static void update_with_cpuid_01h(
  ms_sys_info * const restrict out,
  uint32_t const ebx,
  uint32_t const ecx,
  uint32_t const edx
) {
  ((void)ebx);

  #define ENABLE_FEATURE(reg, cpuid_mask, feature) do { if(ms_test(reg, cpuid_mask)) { out->cpu_features |= feature; } } while(false)

  ENABLE_FEATURE(ecx, CPUID_FEATURE_SSE3_BIT, MS_CPU_FEATURE_SSE3_BIT);
  ENABLE_FEATURE(ecx, CPUID_FEATURE_SSSE3_BIT, MS_CPU_FEATURE_SSSE3_BIT);
  ENABLE_FEATURE(ecx, CPUID_FEATURE_FMA_BIT, MS_CPU_FEATURE_FMA_BIT);
  ENABLE_FEATURE(ecx, CPUID_FEATURE_CMPXCHG16B_BIT, MS_CPU_FEATURE_CMPXCHG16B_BIT);
  ENABLE_FEATURE(ecx, CPUID_FEATURE_SSE4_1_BIT, MS_CPU_FEATURE_SSE4_1_BIT);
  ENABLE_FEATURE(ecx, CPUID_FEATURE_SSE4_2_BIT, MS_CPU_FEATURE_SSE4_2_BIT);
  ENABLE_FEATURE(ecx, CPUID_FEATURE_MOVBE_BIT, MS_CPU_FEATURE_MOVBE_BIT);
  ENABLE_FEATURE(ecx, CPUID_FEATURE_POPCNT_BIT, MS_CPU_FEATURE_POPCNT_BIT);
  ENABLE_FEATURE(ecx, CPUID_FEATURE_AES_BIT, MS_CPU_FEATURE_AES_BIT);
  ENABLE_FEATURE(ecx, CPUID_FEATURE_AVX_BIT, MS_CPU_FEATURE_AVX_BIT);
  ENABLE_FEATURE(ecx, CPUID_FEATURE_F16C_BIT, MS_CPU_FEATURE_F16C_BIT);
  ENABLE_FEATURE(ecx, CPUID_FEATURE_RDRAND_BIT, MS_CPU_FEATURE_RDRAND_BIT);

  ENABLE_FEATURE(edx, CPUID_FEATURE_CMOV_BIT, MS_CPU_FEATURE_CMOV_BIT);
  ENABLE_FEATURE(edx, CPUID_FEATURE_CLFLUSH_BIT, MS_CPU_FEATURE_CLFLUSH_BIT);
  ENABLE_FEATURE(edx, CPUID_FEATURE_MMX_BIT, MS_CPU_FEATURE_MMX_BIT);
  ENABLE_FEATURE(edx, CPUID_FEATURE_SSE_BIT, MS_CPU_FEATURE_SSE_BIT);
  ENABLE_FEATURE(edx, CPUID_FEATURE_SSE2_BIT, MS_CPU_FEATURE_SSE2_BIT);

  #undef ENABLE_FEATURE
}

static void cpuid(uint32_t const query_leaf, uint32_t result[static 4]) {
  __asm__(
    "movl %4, %%eax\n"
    "cpuid\n"
    "movl %%eax, %0\n"
    "movl %%ebx, %1\n"
    "movl %%ecx, %2\n"
    "movl %%edx, %3\n"
    : "=rm"(result[0]), "=rm"(result[1]), "=rm"(result[2]), "=rm"(result[3])
    : "rm"(query_leaf)
    : "eax", "ebx", "ecx", "edx"
  );
}

void ms_sys_update_with_cpuid(ms_sys_info * const restrict result) {
  uint32_t regs[4] = {0}; // eax, ebx, ecx, edx

  cpuid(0, regs);

  if(1 <= regs[0]) { // cpuid(1) supported
    cpuid(1, regs);
    update_with_cpuid_01h(result, regs[1], regs[2], regs[3]);
  } else {
    ms_error("CPUID basic processor information value too low.");
  }

  cpuid(0x80000000, regs);

  if(0x80000006 <= regs[0]) {
    cpuid(0x80000006, regs);
    update_with_cpuid_80000006h(result, regs[1], regs[2], regs[3]);
  } else {
    ms_warnf("CPUID extended processor information does not contain cache line size. Assuming %u bytes.", MS_CACHE_LINE_SIZE);

    result->cache_line_size = MS_CACHE_LINE_SIZE;
  }
}
