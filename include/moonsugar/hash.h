/**
 * @file
 *
 * Hash functions interface.
 */
#ifndef MS_HASH_H
#define MS_HASH_H

#include <moonsugar/api.h>

#define ms_hash_handle ms_hash_u32

typedef uint64_t (*ms_hash_clbk)(void const *x);

/**
 * @def uint64_t_h
 *
 * Handle hashing function.
 */
MSUSERET uint64_t MSAPI ms_hash(void const *const data, uint64_t const size);

MSUSERET uint64_t MSAPI ms_hash_u64(void const * const value);
MSUSERET uint64_t MSAPI ms_hash_u32(void const * const value);
MSUSERET uint64_t MSAPI ms_hash_cstr(void const * const s);

#endif // MS_HASH_H


