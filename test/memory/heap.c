#include <moondance/test.h>
#include <moonsugar/memory.h>

static ms_heap heap;

#define HEAP_SIZE (3llu * 1024 * 1024)
#define PAGE_SIZE (1024llu)

void each_setup(void *ctx) {
  ((void)ctx);
  ms_heap_construct(&heap, HEAP_SIZE, PAGE_SIZE);
}

void each_cleanup(void *ctx) {
  ((void)ctx);
  ms_heap_destroy(&heap);
}

MD_CASE(ctor) {
  md_assert(heap.base != NULL);
  md_assert(heap.committed_size == PAGE_SIZE);
  md_assert(heap.size == HEAP_SIZE);
  md_assert(heap.free_list.first != NULL);

  ms_free_list_node * const chunk = heap.free_list.first;

  md_assert(chunk->prev == NULL);
  md_assert(chunk->next == NULL);
  md_assert(chunk->size == HEAP_SIZE);
}

MD_CASE(allocate) {
  void * const ptr = ms_heap_malloc(&heap, PAGE_SIZE - 1);
  const ms_header * const hdr = ms_heap_get_header(ptr);

  md_assert(ptr != NULL);
  md_assert(hdr->size >= PAGE_SIZE - 1);
  md_assert(hdr->alignment == MS_DEFAULT_ALIGNMENT);
}

MD_CASE(free) {
  void * const ptr_before = ms_heap_malloc(&heap, PAGE_SIZE);
  ms_heap_free(&heap, ptr_before);

  void * const ptr_after = ms_heap_malloc(&heap, PAGE_SIZE);
  md_assert(ptr_before == ptr_after);
}

MD_CASE(allocate_zero) {
  void * const ptr = ms_heap_malloc(&heap, 0);

  md_assert(ptr == NULL);
}

MD_CASE(free_zero) {
  ms_heap_free(&heap, NULL);
}

MD_CASE(realloc_less) {
  void * const ptr_before = ms_heap_malloc(&heap, PAGE_SIZE);
  md_assert(ptr_before != NULL);
  const ms_header * hdr = ms_heap_get_header(ptr_before);
  size_t const size_before = hdr->size;

  void * const ptr_after = ms_heap_realloc(&heap, ptr_before, PAGE_SIZE - 1);
  md_assert(ptr_after == ptr_before);

  hdr = ms_heap_get_header(ptr_after);

  md_assert(hdr->size == size_before);
}

MD_CASE(realloc_more_within_page_boundary) {
  void * const ptr_before = ms_heap_malloc(&heap, 10);
  md_assert(ptr_before != NULL);
  const ms_header * hdr = ms_heap_get_header(ptr_before);
  size_t const size_before = hdr->size;

  void * const ptr_after = ms_heap_realloc(&heap, ptr_before, 2);
  md_assert(ptr_after == ptr_before);

  void * const ptr_after2 = ms_heap_realloc(&heap, ptr_before, 9);
  md_assert(ptr_after == ptr_before);

  hdr = ms_heap_get_header(ptr_after2);

  md_assert(hdr->size == size_before);
}

MD_CASE(realloc_more_cross_page_boundary) {
  void * const ptr_before = ms_heap_malloc(&heap, 1);
  md_assert(ptr_before != NULL);
  const ms_header * hdr = ms_heap_get_header(ptr_before);
  size_t const size_before = hdr->size;

  void * const ptr_after = ms_heap_realloc(&heap, ptr_before, 1024);
  md_assert(ptr_after != NULL);
  md_assert(ptr_after != ptr_before);

  hdr = ms_heap_get_header(ptr_after);

  md_assert(hdr->size > size_before);
}

MD_CASE(realloc_same) {
  void * const ptr_before = ms_heap_malloc(&heap, PAGE_SIZE);
  md_assert(ptr_before != NULL);

  void * const ptr_after = ms_heap_realloc(&heap, ptr_before, PAGE_SIZE);
  md_assert(ptr_after == ptr_before);
}

MD_CASE(realloc_zero) {
  void * const ptr_before = ms_heap_malloc(&heap, 1);
  md_assert(ptr_before != NULL);

  void * const ptr_after = ms_heap_realloc(&heap, ptr_before, 0);
  md_assert(ptr_after == NULL);
}

// Allocate two blocks and free them in allocation order
// The allocator must fall back into its IC
MD_CASE(inverse_free) {
  void * const ptr1 = ms_heap_malloc(&heap, 1);
  void * const ptr2 = ms_heap_malloc(&heap, 1);

  md_assert(ptr1 != NULL);
  md_assert(ptr2 != NULL);
  md_assert(ptr1 != ptr2);

  ms_heap_free(&heap, ptr1);
  ms_heap_free(&heap, ptr2);

  md_assert(heap.free_list.first != NULL);
  md_assert(heap.free_list.first->prev == NULL);
  md_assert(heap.free_list.first->next == NULL);
  md_assert(heap.free_list.first->size == HEAP_SIZE);
}

MD_CASE(static_constraints) {
  md_assert((MS_HEAP_DEALLOC_THR) >= sizeof(ms_free_list_node));
}

int main(int argc, char** argv) {
  md_suite suite = md_suite_create();

  suite.each_setup = each_setup;
  suite.each_cleanup = each_cleanup;

  md_add(&suite, ctor);
  md_add(&suite, allocate);
  md_add(&suite, allocate_zero);
  md_add(&suite, free);
  md_add(&suite, free_zero);
  md_add(&suite, realloc_less);
  md_add(&suite, realloc_same);
  md_add(&suite, realloc_more_within_page_boundary);
  md_add(&suite, realloc_more_cross_page_boundary);
  md_add(&suite, realloc_zero);
  md_add(&suite, inverse_free);
  md_add(&suite, static_constraints);

  return md_run(argc, argv, &suite);
}
