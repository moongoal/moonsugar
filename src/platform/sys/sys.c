#include <moonsugar/platform.h>

static ms_sys_info info;

ms_sys_info const *ms_get_sys_info(void) {
  // RSVD0 is used to determine whether the structure
  // has been initialised or not.
  if(info.cpu_features & MS_CPU_FEATURE_RSVD0_BIT) {
    return &info;
  }

  // Perform initialisation
  ms_sys_update_with_cpuid(&info);
  ms_sys_update_with_os(&info);

  info.cpu_features |= MS_CPU_FEATURE_RSVD0_BIT;

  return &info;
}
