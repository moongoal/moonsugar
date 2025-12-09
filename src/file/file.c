#include <moonsugar/file.h>

#ifndef MS_MAX_FILES
  #define MS_MAX_FILES (1024u)
#endif

typedef struct ms_node ms_node;

struct ms_node {
  ms_file file;
  ms_node* next;
};

ms_node g_nodes[MS_MAX_FILES];
MS_ATOMIC(ms_node*) g_next;

void ms_file_initialize(void) {
  for(uint32_t i = 0; i < MS_MAX_FILES; ++i) {
    ms_node * const restrict node = &g_nodes[i];

    node->next = &g_nodes[i + 1];
  }

  g_nodes[MS_MAX_FILES - 1].next = NULL;
  g_next = &g_nodes[0];
}

static void release_file(ms_file * const restrict f) {
  ms_node * const prev = (ms_node*)f;
  ms_node *node = ms_atomic_load(&g_next, MS_MEMORY_ORDER_ACQUIRE);

  do {
    prev->next = node;
  } while(
    !ms_atomic_compare_exchange_weak(
      &g_next,
      &node,
      prev,
      MS_MEMORY_ORDER_RELEASE,
      MS_MEMORY_ORDER_ACQUIRE
    )
  );
}

ms_result ms_fopen(char const * const path, ms_fmode_flags const flags, ms_file ** const restrict out_file) {
  ms_node *node = ms_atomic_load(&g_next, MS_MEMORY_ORDER_ACQUIRE);
  ms_node *next;

  do {
    if(node == NULL) {
      return MS_RESULT_RESOURCE_LIMIT;
    }
      
    next = node->next;
  } while(
    !ms_atomic_compare_exchange_weak(
      &g_next,
      &node,
      next,
      MS_MEMORY_ORDER_RELEASE,
      MS_MEMORY_ORDER_ACQUIRE
    )
  );

  *out_file = &node->file;

  ms_result const result = ms_file_backend_open(path, &node->file, flags);

  if(result != MS_RESULT_SUCCESS) {
    release_file(*out_file);
  }

  return result;
}

void ms_fclose(ms_file * const restrict f) {
  ms_file_backend_close(f);
  release_file(f);
}

