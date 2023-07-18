// macros.h            see license.txt for copyright and terms of use
// grab-bag of useful macros, stashed here to avoid mucking up
//   other modules with more focus
// (no configuration stuff here!)

#ifndef __MACROS_H
#define __MACROS_H


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


#define ELK_UNUSED(arg) ((void)(arg))


// appended to function declarations to indicate they do not
// return control to their caller; e.g.:
//   void exit(int code) NORETURN;
#ifdef __GNUC__
  #define NORETURN __attribute__((noreturn))
#else
  // just let the warnings roll if we can't suppress them
  #define NORETURN
#endif


// these two are a common idiom in my code for typesafe casts;
// they are essentially a roll-your-own RTTI
#define CAST_MEMBER_FN(destType)                                                \
  destType const &as##destType##C() const;                                      \
  destType &as##destType() { return const_cast<destType&>(as##destType##C()); }

#define CAST_MEMBER_IMPL(inClass, destType)         \
  destType const &inClass::as##destType##C() const  \
  {                                                 \
    xassert(is##destType());                        \
    return (destType const&)(*this);                \
  }


#endif // __MACROS_H
