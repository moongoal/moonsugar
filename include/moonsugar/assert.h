/**
 * @file
 *
 * Assertions.
 */
#ifndef MS_ASSERT_H
#define MS_ASSERT_H

#include <stdio.h>
#include <moonsugar/api.h>

#define MS_ASSERT(cond) MS_ASSERT2(cond, MS_QUOTE(cond))

#ifdef MS_FEAT_ASSERT
  #define MS_ASSERT2(cond, msg) \
    do { \
      if(!(cond)) { \
        fputs(msg, stderr); \
        ms_break(); \
      } \
    } while(false)
#else // MS_FEAT_ASSERT
  #define MS_ASSERT2(cond, msg) ((void)(cond))
#endif // MS_FEAT_ASSERT

#endif // MS_ASSERT_H
