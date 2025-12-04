/**
 *
 * @file
 *
 * Functional utilities.
 */
#ifndef MS_FUNCTIONAL_H
#define MS_FUNCTIONAL_H

#include <moonsugar/api.h>

typedef bool (*ms_equals_clbk)(void const * a, void const * b);
typedef bool (*ms_none_test_clbk)(void const * a);
typedef void (*ms_none_set_clbk)(void * a);

MSUSERET MSAPI bool ms_equals_u64(void const * a, void const * b);
MSUSERET MSAPI bool ms_equals_u32(void const * a, void const * b);
MSUSERET MSAPI bool ms_equals_cstr(void const * a, void const * b);
MSUSERET MSAPI bool ms_equals_handle(void const * a, void const * b);

MSUSERET MSAPI bool ms_none_test_max_u64(void const * x);
MSUSERET MSAPI bool ms_none_test_max_u32(void const * x);
MSUSERET MSAPI bool ms_none_test_ptr(void const * x);
MSUSERET MSAPI bool ms_none_test_handle(void const * x);

MSAPI void ms_none_set_max_u64(void * x);
MSAPI void ms_none_set_max_u32(void * x);
MSAPI void ms_none_set_ptr(void * x);
MSAPI void ms_none_set_handle(void * x);

#endif // MS_FUNCTIONAL_H

