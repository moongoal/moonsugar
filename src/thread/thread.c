#include <string.h>
#include <moonsugar/thread-internal.h>

#define MAX_DESCRIPTORS (128u)

MS_THREAD_LOCAL char current_thread_name[MS_THREAD_NAME_MAX_LEN];

MS_ATOMIC(unsigned) current_descriptor_index;
static ms_thread_start_descriptor thread_start_descriptors[MAX_DESCRIPTORS];

ms_thread_start_descriptor * ms_thread_acquire_start_descriptor(void) {
  ms_thread_start_descriptor *descriptor;

  for(
    bool in_use = true;
    in_use;
    in_use = ms_atomic_test_and_set(&descriptor->in_use, MS_MEMORY_ORDER_SEQ_CST)
  ) {
    unsigned const descriptor_index = ms_atomic_fetch_add(
      &current_descriptor_index,
      1,
      MS_MEMORY_ORDER_SEQ_CST
    );

    descriptor = &thread_start_descriptors[descriptor_index];
  }

  return descriptor;
}

void ms_thread_release_start_descriptor(ms_thread_start_descriptor * const d) {
  ms_atomic_clear(&d->in_use, MS_MEMORY_ORDER_SEQ_CST);
}

char const *ms_get_current_thread_name(void) {
  return current_thread_name;
}

void ms_set_current_thread_name(char const * const name) {
  unsigned len = strlen(name);

  if(len > MS_THREAD_NAME_MAX_LEN) {
    len = MS_THREAD_NAME_MAX_LEN - 1;
  }

  memcpy(current_thread_name, name, sizeof(char) * len);
  current_thread_name[MS_THREAD_NAME_MAX_LEN - 1] = '\0';
}
