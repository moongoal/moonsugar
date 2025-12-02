/**
 * @file
 *
 * Test utilities.
 */
#ifndef MS_TEST_H
#define MS_TEST_H

#ifndef MS_TEST_SUITE
  #error This header must only be includer in unit tests.
#endif

#include <moondance/test.h>
#include <moonsugar/api.h>
#include <moonsugar/memory.h>

static ms_heap g_heap;
static ms_allocator g_allocator;

#define MST_HEAP_SIZE (1024U * 1024U)
#define MST_HEAP_PAGE_SIZE (1024U)

#define MST_MEMORY_INIT() \
  do { \
    ms_heap_construct(&g_heap, MST_HEAP_SIZE, MST_HEAP_PAGE_SIZE); \
    g_allocator = MS_ALLOCATOR_DEF_HEAP(g_heap); \
  } while(false)

#define MST_MEMORY_DESTROY() ms_heap_destroy(&g_heap)

#endif // MS_TEST_H

