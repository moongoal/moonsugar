#include <moonsugar/test.h>
#include <moonsugar/assert.h>
#include <moonsugar/containers/ring.h>

#define RING_LENGTH 16u

static ms_ring ring;

static void suite_setup(md_suite *suite) { ((void)suite); MST_MEMORY_INIT(); }
static void suite_cleanup(md_suite *suite) { ((void)suite); MST_MEMORY_DESTROY(); }

static void each_setup(void *ctx) {
  ((void)ctx);

  ms_result const result = ms_ring_construct(
    &ring,
    &(ms_ring_description){RING_LENGTH, sizeof(uint32_t), g_allocator}
  );

  MS_ASSERT(result == MS_RESULT_SUCCESS);
}

static void each_cleanup(void *ctx) { ((void)ctx); ms_ring_destroy(&ring); }

MD_CASE(construct) {
  md_assert(ring.capacity = RING_LENGTH);
  md_assert(ring.windex == 0);
  md_assert(ring.count == 0);
  md_assert(ring.values != NULL);
}

MD_CASE(enqueue) {
  uint32_t *const wptr = ms_ring_enqueue(&ring);

  *wptr = 0x12345;

  md_assert(ring.count == 1);
  md_assert(ring.windex == 1);
  md_assert(wptr == ring.values);
}

MD_CASE(enqueue__full) {
  for(int i = 0; i < (int)RING_LENGTH; ++i) {
    uint32_t *const wptr = ms_ring_enqueue(&ring);
    *wptr = UINT32_MAX;
  }

  md_assert(ring.count == RING_LENGTH);
  md_assert(ring.windex == RING_LENGTH);

  uint32_t *const wptr = ms_ring_enqueue(&ring);

  md_assert(wptr == NULL);
  md_assert(ring.count == RING_LENGTH);
  md_assert(ring.windex == RING_LENGTH);
}

MD_CASE(enqueue__wrap_around) {
  for(int i = 0; i < (int)RING_LENGTH; ++i) {
    (void)ms_ring_enqueue(&ring);
  }

  (void)ms_ring_dequeue(&ring);

  uint32_t *const wptr = ms_ring_enqueue(&ring);

  md_assert(ring.count == RING_LENGTH);
  md_assert(ring.windex == RING_LENGTH + 1);
  md_assert(wptr == ring.values);
}

MD_CASE(dequeue) {
  uint32_t *const wptr = ms_ring_enqueue(&ring);
  *wptr = 0x123;

  uint32_t *rptr = ms_ring_dequeue(&ring);
  md_assert(rptr == wptr);
  md_assert(ring.count == 0);
  md_assert(ring.windex == 1);

  rptr = ms_ring_dequeue(&ring);
  md_assert(ring.count == 0);
  md_assert(ring.windex == 1);
  md_assert(rptr == NULL);
}

int main(int argc, char **argv) {
  md_suite suite = md_suite_create();

  suite.suite_setup = suite_setup;
  suite.suite_cleanup = suite_cleanup;

  suite.each_setup = each_setup;
  suite.each_cleanup = each_cleanup;

  md_add(&suite, construct);
  md_add(&suite, enqueue);
  md_add(&suite, enqueue__full);
  md_add(&suite, enqueue__wrap_around);
  md_add(&suite, dequeue);

  return md_run(argc, argv, &suite);
}


