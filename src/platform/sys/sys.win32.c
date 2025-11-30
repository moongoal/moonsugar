#include <Windows.h>
#include <moonsugar/path.h>
#include <moonsugar/util.h>
#include <moonsugar/platform.h>

void ms_sys_update_with_os(ms_sys_info *const result) {
  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);

  result->page_size = sys_info.dwPageSize;
  result->proc_count = sys_info.dwNumberOfProcessors;
  result->alloc_granularity = sys_info.dwAllocationGranularity;

  ULONGLONG installed_memory;
  if(GetPhysicallyInstalledSystemMemory(&installed_memory)) {
    result->memory_size = installed_memory * 1024;
  } else {
    result->memory_size = UINT64_MAX;
  }
}

void ms_sys_query_engine_dir(char * const out_dir, unsigned const out_dir_len) {
#define BUFF_LEN 1024
  char buff[BUFF_LEN];

  if(GetModuleFileName(NULL, out_dir, out_dir_len) < out_dir_len) {
    // Get path of the $ENGINE/bin/ directory
    if(ms_path_get_parent(out_dir, buff, BUFF_LEN) == MS_RESULT_SUCCESS) {
      // Get pato of the $ENGINE/ directory
      if(ms_path_get_parent(buff, out_dir, out_dir_len) == MS_RESULT_SUCCESS) {
        return;
      }
    }
  }
  
  out_dir[0] = '\0';

#undef BUFF_LEN
}

