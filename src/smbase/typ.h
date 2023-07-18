// typ.h            see license.txt for copyright and terms of use
// various types and definitions, some for portability, others for convenience
// Scott McPeak, 1996-2000  This file is public domain.

#ifndef __TYP_H
#define __TYP_H

#include <stdint.h>
#include <stddef.h>         // NULL



// tag for definitions of static member functions; there is no
// compiler in existence for which this is useful, but I like
// to see *something* next to implementations of static members
// saying that they are static, and this seems slightly more
// formal than just a comment
#define STATICDEF /*static*/


// often-useful number-of-entries function
#define TABLESIZE(tbl) ((int)(sizeof(tbl)/sizeof((tbl)[0])))


// concise way to loop on an integer range
#define loopi(end) for(int i=0; i<(int)(end); i++)
#define loopj(end) for(int j=0; j<(int)(end); j++)
#define loopk(end) for(int k=0; k<(int)(end); k++)


// for using selfCheck methods
// to explicitly check invariants in debug mode
//
// dsw: debugging *weakly* implies selfchecking: if we are debugging,
// do selfcheck unless otherwise specified
#ifndef NDEBUG
  #ifndef DO_SELFCHECK
    #define DO_SELFCHECK 1
  #endif
#endif
// dsw: selfcheck *bidirectionally* configurable from the command line: it
// may be turned on *or* off: any definition other than '0' counts as
// true, such as -DDO_SELFCHECK=1 or just -DDO_SELFCHECK
#ifndef DO_SELFCHECK
  #define DO_SELFCHECK 0
#endif
#if DO_SELFCHECK != 0
  #define SELFCHECK() selfCheck()
#else
  #define SELFCHECK() ((void)0)
#endif


#endif // __TYP_H
