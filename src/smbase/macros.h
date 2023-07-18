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
#define loopj(end) for(int j=0; j<(int)(end); j++)


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


// member copy in constructor initializer list
#define DMEMB(var) var(obj.var)


// assert something at compile time (must use this inside a function);
// works because compilers won't let us declare negative-length arrays
// (the expression below works with egcs-1.1.2, gcc-2.x, gcc-3.x)
#define STATIC_ASSERT(cond) \
  { (void)((int (*)(char failed_static_assertion[(cond)?1:-1]))0); }


// for silencing variable-not-used warnings
template <class T>
inline void pretendUsedFn(T const &) {}
#define PRETEND_USED(arg) pretendUsedFn(arg) /* user ; */


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


// keep track of a count and a high water mark
#define INC_HIGH_WATER(count, highWater)  \
  count++;                                \
  if (count > highWater) {                \
    highWater = count;                    \
  }


// for a class that maintains allocated-node stats
#define ALLOC_STATS_DECLARE                     \
  static int numAllocd;                         \
  static int maxAllocd;                         \
  static void printAllocStats(bool anyway);

// these would go in a .cc file, whereas above goes in .h file
#define ALLOC_STATS_DEFINE(classname)                      \
  int classname::numAllocd = 0;                            \
  int classname::maxAllocd = 0;                            \
  STATICDEF void classname::printAllocStats(bool anyway)   \
  {                                                        \
    if (anyway || numAllocd != 0) {                        \
      std::cout << #classname << " nodes: " << numAllocd   \
           << ", max  nodes: " << maxAllocd                \
           << std::endl;                                   \
    }                                                      \
  }

#define ALLOC_STATS_IN_CTOR                     \
  INC_HIGH_WATER(numAllocd, maxAllocd);

#define ALLOC_STATS_IN_DTOR                     \
  numAllocd--;


// ----------- automatic data value restorer -------------
// used when a value is to be set to one thing now, but restored
// to its original value on return (even when the return is by
// an exception being thrown)
template <class T>
class Restorer {
  T &variable;
  T prevValue;

public:
  Restorer(T &var, T newValue)
    : variable(var),
      prevValue(var)
  {
    variable = newValue;
  }

  // this one does not set it to a new value, just remembers the current
  Restorer(T &var)
    : variable(var),
      prevValue(var)
  {}

  ~Restorer()
  {
    variable = prevValue;
  }
};


// put at the top of a class for which the default copy ctor
// and operator= are not desired; then don't define these functions
#define NO_OBJECT_COPIES(name)   \
  private:                       \
    name(name&);                 \
    void operator=(name&) /*user ;*/


#endif // __MACROS_H
