#include <moonsugar/test.h>
#include <moonsugar/containers.h>

#define PAGE_CAPACITY (256u)
#define ITEM_SIZE (sizeof(int))

ms_pvector vec;

void suite_setup(md_suite * suite) { ((void)suite); MST_MEMORY_INIT(); }
void suite_cleanup(md_suite * suite) { ((void)suite); MST_MEMORY_DESTROY(); }

void each_cleanup(void *ctx) { ((void)ctx); ms_pvector_destroy(&vec); }

MD_CASE(construct) {
  ms_result const result = ms_pvector_construct(&vec, &(ms_pvector_description){PAGE_CAPACITY, ITEM_SIZE, g_allocator});

  md_assert(result == MS_RESULT_SUCCESS);
  md_assert(vec.first == NULL);
  md_assert(vec.last == NULL);
  md_assert(vec.item_size == ITEM_SIZE);
  md_assert(vec.page_capacity == PAGE_CAPACITY);
  md_assert(vec.page_count == 0);
}

MD_CASE(construct__zero_page_capacity) {
  ms_result const result = ms_pvector_construct(&vec, &(ms_pvector_description){0, ITEM_SIZE, g_allocator});
  md_assert(result == MS_RESULT_INVALID_ARGUMENT);
}

MD_CASE(construct__zero_item_size) {
  ms_result const result = ms_pvector_construct(&vec, &(ms_pvector_description){PAGE_CAPACITY, 0, g_allocator});
  md_assert(result == MS_RESULT_INVALID_ARGUMENT);
}

MD_CASE(construct__null_description) {
  ms_result const result = ms_pvector_construct(&vec, NULL);
  md_assert(result == MS_RESULT_INVALID_ARGUMENT);
}

MD_CASE(append_page) {
  ms_pvector_construct(&vec, &(ms_pvector_description){PAGE_CAPACITY, ITEM_SIZE, g_allocator});
  ms_result const app1_result = ms_pvector_append_page(&vec);

  md_assert(app1_result == MS_RESULT_SUCCESS);
  md_assert(vec.first != NULL);
  md_assert(vec.last == vec.first);
  md_assert(vec.page_count == 1);

  md_assert(vec.first->count == 0);
  md_assert(vec.first->next == NULL);

  ms_result const app2_result = ms_pvector_append_page(&vec);

  md_assert(app2_result == MS_RESULT_SUCCESS);
  md_assert(vec.first != vec.last);
  md_assert(vec.last != NULL);
  md_assert(vec.page_count == 2);
}

MD_CASE(page_size_macros) {
  ms_pvector_construct(&vec, &(ms_pvector_description){PAGE_CAPACITY, ITEM_SIZE, g_allocator});

  md_assert(
    MS_PVECTOR_PAGE_SIZE_STATIC(int, PAGE_CAPACITY)
    == sizeof(ms_pvector_page) + ITEM_SIZE * PAGE_CAPACITY
  );

  md_assert(MS_PVECTOR_PAGE_SIZE(&vec) == sizeof(ms_pvector_page) + ITEM_SIZE * PAGE_CAPACITY);
}

MD_CASE(get_ptr__out_of_bounds) {
  ms_pvector_construct(&vec, &(ms_pvector_description){PAGE_CAPACITY, ITEM_SIZE, g_allocator});
  ms_pvector_append_page(&vec);

  void *out_ptr = ms_pvector_get_ptr(&vec, 1);
  md_assert(out_ptr == NULL);
}

MD_CASE(get__append) {
  ms_result append_result;

  ms_pvector_construct(&vec, &(ms_pvector_description){PAGE_CAPACITY, ITEM_SIZE, g_allocator});
  ms_pvector_append_page(&vec);

  for(int i = 0; i < (int)PAGE_CAPACITY; ++i) {
    append_result = ms_pvector_append(&vec, &i);
    md_assert(append_result == MS_RESULT_SUCCESS);
  }

  append_result = ms_pvector_append(&vec, &(int){5});
  md_assert(append_result == MS_RESULT_SUCCESS);
  md_assert(vec.last->count == 1);

  int out_value;
  ms_result get_result;

  for(int i = 0; i < (int)PAGE_CAPACITY; ++i) {
    get_result = ms_pvector_get_value(&vec, i, &out_value);

    md_assert(get_result == MS_RESULT_SUCCESS);
    md_assert(out_value == i);
  }

  get_result = ms_pvector_get_value(&vec, PAGE_CAPACITY, &out_value);

  md_assert(get_result == MS_RESULT_SUCCESS);
  md_assert(out_value == 5);
}

MD_CASE(set) {
  ms_result append_result;

  ms_pvector_construct(&vec, &(ms_pvector_description){PAGE_CAPACITY, ITEM_SIZE, g_allocator});
  ms_pvector_append_page(&vec);

  for(int i = 0; i < (int)PAGE_CAPACITY; ++i) {
    append_result = ms_pvector_append(&vec, &i);

    md_assert(append_result == MS_RESULT_SUCCESS);
  }

  append_result = ms_pvector_set(&vec, 1, &(int){999});

  for(int i = 0; i < (int)PAGE_CAPACITY; ++i) {
    int out_value;
    ms_result const get_result = ms_pvector_get_value(&vec, i, &out_value);

    md_assert(get_result == MS_RESULT_SUCCESS);

    if(i != 1) {
      md_assert(out_value == i);
    } else {
      md_assert(out_value == 999);
    }
  }
}

MD_CASE(set__out_of_bounds) {
  ms_pvector_construct(&vec, &(ms_pvector_description){PAGE_CAPACITY, ITEM_SIZE, g_allocator});
  ms_pvector_append_page(&vec);

  ms_result const get_result = ms_pvector_set(&vec, 1, &(int){5});

  md_assert(get_result == MS_RESULT_INVALID_ARGUMENT);
}

MD_CASE(for_each_page_macro) {
  ms_pvector_construct(&vec, &(ms_pvector_description){PAGE_CAPACITY, ITEM_SIZE, g_allocator});
  ms_pvector_append_page(&vec);

  MS_PVECTOR_FOREACH_PAGE(&vec, md_assert(page == vec.first));
}

MD_CASE(for_each_macro) {
  ms_pvector_construct(&vec, &(ms_pvector_description){PAGE_CAPACITY, ITEM_SIZE, g_allocator});
  ms_pvector_append_page(&vec);

  vec.first->count = 3;
  int *prev_item = NULL;

  MS_PVECTOR_FOREACH(
    &vec,
    int,
    if(prev_item == NULL) { md_assert(item == (void *)page->data); } else {
      md_assert(item == prev_item + 1);
    }

    prev_item = item;
  );
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  suite.suite_setup = suite_setup;
  suite.suite_cleanup = suite_cleanup;
  suite.each_cleanup = each_cleanup;

  md_add(&suite, construct);
  md_add(&suite, construct__zero_item_size);
  md_add(&suite, construct__zero_page_capacity);
  md_add(&suite, construct__null_description);
  md_add(&suite, append_page);
  md_add(&suite, page_size_macros);
  md_add(&suite, get__append);
  md_case *const get_ptr__out_of_bounds_case MSUNUSED
      = md_add(&suite, get_ptr__out_of_bounds);
  md_case *const set__out_of_bounds_case MSUNUSED = md_add(&suite, set__out_of_bounds);
  md_add(&suite, set);
  md_add(&suite, for_each_page_macro);
  md_add(&suite, for_each_macro);

#ifndef MS_FEAT_CHECK_BOUNDS
  get_ptr__out_of_bounds_case->skip = true;
  set__out_of_bounds_case->skip = true;
#endif // MS_FEAT_CHECK_BOUNDS

  return md_run(argc, argv, &suite);
}


