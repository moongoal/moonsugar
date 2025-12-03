#include <moonsugar/test.h>
#include <moonsugar/memory.h>

static ms_arena arena;

#define ARENA_BASE_SIZE (1024ull)

void suite_setup(md_suite *suite) {
  ((void)suite);

  MST_MEMORY_INIT();
}

void suite_cleanup(md_suite *suite) {
  ((void)suite);

  MST_MEMORY_DESTROY();
}

void each_setup(void *ctx) {
  ((void)ctx);
  ms_arena_description const description = {
    ARENA_BASE_SIZE,
    MS_ALLOCATOR_DEF_HEAP(g_heap)
  };

  ms_arena_construct(&arena, &description);
}

void each_cleanup(void *ctx) {
  ((void)ctx);
  ms_arena_destroy(&arena);
}

MD_CASE(ctor) {
  md_assert(arena.base_size == ARENA_BASE_SIZE);
  md_assert(arena.first == NULL);
}

MD_CASE(allocate) {
  void * const ptr = ms_arena_malloc(&arena, ARENA_BASE_SIZE - 1, MS_DEFAULT_ALIGNMENT);
  const ms_header * const hdr = ms_arena_get_header(ptr);

  md_assert(ptr != NULL);
  md_assert(hdr->size >= ARENA_BASE_SIZE - 1);
  md_assert(hdr->alignment == MS_DEFAULT_ALIGNMENT);
}

MD_CASE(free) {
  void * const ptr_before = ms_arena_malloc(&arena, ARENA_BASE_SIZE, MS_DEFAULT_ALIGNMENT);
  ms_arena_free(&arena, ptr_before);

  void * const ptr_after = ms_arena_malloc(&arena, ARENA_BASE_SIZE, MS_DEFAULT_ALIGNMENT);
  md_assert(ptr_before == ptr_after);
}

MD_CASE(allocate_zero) {
  void * const ptr = ms_arena_malloc(&arena, 0, MS_DEFAULT_ALIGNMENT);

  md_assert(ptr == NULL);
}

MD_CASE(free_zero) {
  ms_arena_free(&arena, NULL);
}

MD_CASE(realloc_less) {
  void * const ptr_before = ms_arena_malloc(&arena, ARENA_BASE_SIZE, MS_DEFAULT_ALIGNMENT);
  md_assert(ptr_before != NULL);
  const ms_header * hdr = ms_arena_get_header(ptr_before);
  size_t const size_before = hdr->size;

  void * const ptr_after = ms_arena_realloc(&arena, ptr_before, ARENA_BASE_SIZE - 1);
  md_assert(ptr_after == ptr_before);

  hdr = ms_arena_get_header(ptr_after);

  md_assert(hdr->size == size_before);
}

MD_CASE(realloc_more_within_page_boundary) {
  void * const ptr_before = ms_arena_malloc(&arena, 10, MS_DEFAULT_ALIGNMENT);
  md_assert(ptr_before != NULL);
  const ms_header * hdr = ms_arena_get_header(ptr_before);
  size_t const size_before = hdr->size;

  void * const ptr_after = ms_arena_realloc(&arena, ptr_before, 2);
  md_assert(ptr_after == ptr_before);

  void * const ptr_after2 = ms_arena_realloc(&arena, ptr_before, 9);
  md_assert(ptr_after == ptr_before);

  hdr = ms_arena_get_header(ptr_after2);

  md_assert(hdr->size == size_before);
}

MD_CASE(realloc_more_cross_page_boundary) {
  void * const ptr_before = ms_arena_malloc(&arena, 1, MS_DEFAULT_ALIGNMENT);
  md_assert(ptr_before != NULL);
  const ms_header * hdr = ms_arena_get_header(ptr_before);
  size_t const size_before = hdr->size;

  void * const ptr_after = ms_arena_realloc(&arena, ptr_before, 1024);
  md_assert(ptr_after != NULL);
  md_assert(ptr_after != ptr_before);

  hdr = ms_arena_get_header(ptr_after);

  md_assert(hdr->size > size_before);
}

MD_CASE(realloc_same) {
  void * const ptr_before = ms_arena_malloc(&arena, ARENA_BASE_SIZE, MS_DEFAULT_ALIGNMENT);
  md_assert(ptr_before != NULL);

  void * const ptr_after = ms_arena_realloc(&arena, ptr_before, ARENA_BASE_SIZE);
  md_assert(ptr_after == ptr_before);
}

MD_CASE(realloc_zero) {
  void * const ptr_before = ms_arena_malloc(&arena, 1, MS_DEFAULT_ALIGNMENT);
  md_assert(ptr_before != NULL);

  void * const ptr_after = ms_arena_realloc(&arena, ptr_before, 0);
  md_assert(ptr_after == NULL);
}

// Allocate two blocks and free them in allocation order
// The allocator must fall back into its IC
MD_CASE(inverse_free) {
  void * const ptr1 = ms_arena_malloc(&arena, 1, MS_DEFAULT_ALIGNMENT);
  void * const ptr2 = ms_arena_malloc(&arena, 1, MS_DEFAULT_ALIGNMENT);

  md_assert(ptr1 != NULL);
  md_assert(ptr2 != NULL);
  md_assert(ptr1 != ptr2);

  ms_arena_free(&arena, ptr1);
  ms_arena_free(&arena, ptr2);

  md_assert(arena.first == NULL);
}

MD_CASE(static_constraints) {
  md_assert((MS_HEAP_DEALLOC_THR) >= sizeof(ms_free_list_node));
}

int main(int argc, char** argv) {
  md_suite suite = md_suite_create();

  suite.suite_setup = suite_setup;
  suite.suite_cleanup = suite_cleanup;

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

