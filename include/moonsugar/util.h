/**
 * Utilities.
 */
#ifndef MS_INTEGER_H
#define MS_INTEGER_H

#include <moonsugar/api.h>

#define MS_IS_POWER2_STATIC(n) ((n) > 0) & ((((n) & ((n) - 1)) == 0) != 0)
#define MS_IS_MULTIPLE_STATIC(a, b) ((a) % (b) == 0)
#define MS_ALIGN_SZ_STATIC(value, boundary) ((value) + ((-(value)) & ((boundary) - 1)))

/**
 * Test whether a number is a power of two.
 *
 * @param n The number to test.
 *
 * @return True if n is a power of two, false if not.
 */
MSINLINE MSUSERET inline static bool ms_is_power2(uint64_t const n) {
  return __builtin_popcountll(n) == 1;
}

/**
 * Test whether a number is a multiple of another.
 *
 * @param a The number expected to be a multiple of b.
 * @param b The number a is expected to be a multiple of.
 *
 * @return True if a is a multiple of b, false if not.
 */
MSINLINE MSUSERET inline static bool ms_is_multiple(uint64_t const a, uint64_t const b) {
  return MS_IS_MULTIPLE_STATIC(a, b);
}

/**
 * Choose the first value if the condition is true, the second
 * value otherwise. The selection is made without branching.
 *
 * @param a The value to choose if the condition is true.
 * @param b The value to choose if the conition is false.
 * @param cond The condition to evaluate.
 *
 * @return Either `a` or `b` depending on the value of the condition.
 */
MSINLINE MSUSERET inline static uint64_t ms_choose(uint64_t a, uint64_t b, bool cond) {
  const uint64_t mask = 0ull - (uint64_t)cond;

  return (a & mask) | (b & ~mask);
}

/**
 * Align a size to a given boundary.
 *
 * @param value The size to align.
 * @param boundary The alignment boundary; this value must be a power of 2.
 *
 * @return The aligned size.
 */
MSINLINE MSUSERET inline static uint64_t ms_align_sz(uint64_t const value, uint64_t const boundary) {
  return MS_ALIGN_SZ_STATIC(value, boundary);
}

/**
 * Align a pointer to a given boundary.
 *
 * @param value The pointer to align.
 * @param boundary The alignment boundary; this value must be a power of 2.
 *
 * @return The aligned pointer.
 */
MSINLINE MSUSERET inline static void* ms_align_ptr(void* const value, uint64_t const boundary) {
  return (void*)ms_align_sz((uint64_t)value, boundary);
}

/**
 * Select the minimum between two values.
 *
 * @param a The first value.
 * @param b The second value.
 *
 * @return The minimum between the two arguments.
 */
MSINLINE MSUSERET inline static uint64_t ms_min(uint64_t a, uint64_t b) {
  return a > b ? b : a;
}

/**
 * Select the minimum between two values.
 *
 * @param a The first value.
 * @param b The second value.
 *
 * @return The minimum between the two arguments.
 */
MSINLINE MSUSERET inline static int64_t ms_min_signed(int64_t a, int64_t b) {
  return a > b ? b : a;
}

/**
 * Select the maximum between two values.
 *
 * @param a The first value.
 * @param b The second value.
 *
 * @return The maximum between the two arguments.
 */
MSINLINE MSUSERET inline static uint64_t ms_max(uint64_t a, uint64_t b) {
  return a > b ? a : b;
}

/**
 * Select the maximum between two values.
 *
 * @param a The first value.
 * @param b The second value.
 *
 * @return The maximum between the two arguments.
 */
MSINLINE MSUSERET inline static int64_t ms_max_signed(int64_t a, int64_t b) {
  return a > b ? a : b;
}

/**
 * Return a value capped between a minimum and a maximum.
 *
 * @param vmin The minimum value.
 * @param vax The maximum value.
 * @param value The value to cap.
 *
 * @return `value` capped at the minimum and the maximum.
 */
MSINLINE MSUSERET inline static uint64_t ms_minmax(uint64_t vmin, uint64_t vmax, uint64_t value) {
  return ms_min(
    vmax,
    ms_max(vmin, value)
  );
}

/**
* Return a value capped between a minimum and a maximum.
*
* @param vmin The minimum value.
* @param vax The maximum value.
* @param value The value to cap.
*
* @return `value` capped at the minimum and the maximum.
*/
MSINLINE MSUSERET inline static int64_t ms_minmax_signed(int64_t vmin, int64_t vmax, int64_t value) {
  return ms_min_signed(
    vmax,
    ms_max_signed(vmin, value)
  );
}

/**
 * Test whether an unsigned integer has a specific mask set.
 *
 * @param value The unsigned integer value to test.
 * @param mask The bit mask to test.
 *
 * @return Non-zero if all the bits in the mask aer set,
 *  zero if any bit in the mask is not set.
 */
#define ms_test(value, mask) (((value) & (mask)) == mask)

/**
 * Set a bit.
 *
 * @param value The value to set.
 * @param mask The bit mask to set.
 *
 * @return The value with the indicated bit set.
 */
 #define ms_set(value, mask) ((value) | (mask))

/**
 * Clear a bit.
 *
 * @param value The value to clear.
 * @param mask The bit mask to clear.
 *
 * @return The value with the indicated bit cleared.
 */
#define ms_clear(value, mask) ((value) & ~(mask))

#endif // MS_INTEGER_H

