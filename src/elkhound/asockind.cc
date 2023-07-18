// asockind.cc            see license.txt for copyright and terms of use
// code for asockind.h

#include "asockind.h"    // this module
#include "xassert.h"     // xassert

string toString(AssocKind k)
{
  static char const * const arr[] = {
    "AK_LEFT",
    "AK_RIGHT",
    "AK_NONASSOC",
    "AK_NEVERASSOC",
    "AK_SPLIT"
  };
  static_assert(TABLESIZE(arr) == NUM_ASSOC_KINDS,
                "arr has wrong size relative to the AssocKind enum");
  xassert((unsigned)k < NUM_ASSOC_KINDS);
  return string(arr[k]);
}
