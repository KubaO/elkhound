// str.h            see license.txt for copyright and terms of use
// a string class
// the representation uses just one char*, so that a smart compiler
//   can pass the entire object as a single word
// Scott McPeak, 1995-2000  This file is public domain.

// 2005-03-01: See string.txt.  The plan is to evolve the class
// towards compatibility with std::string, such that eventually
// they will be interchangeable.  So far I have converted only
// the most problematic constructs, those involving construction,
// conversion, and internal pointers.

#ifndef STR_H
#define STR_H

#include "typ.h"         // bool
#include <iostream>      // istream, ostream
#include <stdarg.h>      // va_list
#include <string.h>      // strcmp, etc.

#include <nonstd/string_view.hpp>
#include <string>
#include <type_traits>



// ------------------------- string ---------------------

using string = std::string;

// concatenation (properly handles string growth)
// uses '&' instead of '+' to avoid char* coercion problems
string operator&(string const& head, string const& tail);
string& operator&=(string& head, string const& tail);

// read all remaining chars of is into the output string, clearing it first
void readall(std::istream& is, string& into);


// ------------------------ rostring ----------------------
// My plan is to use this in places I currently use 'char const *'.
typedef string const &rostring;

// I have the modest hope that the transition to 'rostring' might be
// reversible, so this function converts to 'char const *' but with a
// syntax that could just as easily apply to 'char const *' itself
// (and in that case would be the identity function).
inline char const *toCStr(rostring s) { return s.c_str(); }

void toCStr(char const* s) = delete;

// I need some compatibility functions
inline int strlen(rostring s) { return s.length(); }

int strcmp(rostring s1, rostring s2);
int strcmp(rostring s1, char const *s2);
int strcmp(char const *s1, rostring s2);
// string.h, above, provides:
// int strcmp(char const *s1, char const *s2);

// dsw: this is what we are asking most of the time so let's special
// case it
inline bool streq(rostring s1, rostring s2)       {return strcmp(s1, s2) == 0;}
inline bool streq(rostring s1, char const *s2)    {return strcmp(s1, s2) == 0;}
inline bool streq(char const *s1, rostring s2)    {return strcmp(s1, s2) == 0;}
inline bool streq(char const *s1, char const *s2) {return strcmp(s1, s2) == 0;}

char const *strstr(rostring haystack, char const *needle);

// there is no wrapper for 'strchr'; use string::contains

int atoi(rostring s);

// construct a string out of characters from 'p' up to 'p+n-1',
// inclusive; resulting string length is 'n'
string substring(char const *p, int n);
inline string substring(rostring p, int n)
  { return substring(p.c_str(), n); }


// --------------------- stringBuilder --------------------
// this class is specifically for appending lots of things
class stringBuilder : public string {
protected:
  enum { EXTRA_SPACE = 30 };    // extra space allocated in some situations

protected:
  void init(int initSize);
  void dup(char const *src);

public:
  explicit stringBuilder(int capacity);

  stringBuilder() { init(0); }
  stringBuilder(char const *str);
  stringBuilder(char const *str, int length);
  stringBuilder(string const& str) : string(str) {}
  stringBuilder(stringBuilder const& obj) = default;

  // unlike 'string' above, I will allow stringBuilder to convert to
  // char const * so I can continue to use 'stringc' to build strings
  // for functions that accept char const *; this should not conflict
  // with std::string, since I am explicitly using a different class
  // (namely stringBuilder) when I use this functionality
  operator char const * () const { return c_str(); }

  stringBuilder& setlength(int newlen);    // change length, forget current data

  // make sure we can store 'someLength' non-null chars; grow if necessary
  void ensure(int someLength) { if (someLength >= capacity()) { grow(someLength); } }

  // grow the string's length (retaining data); make sure it can hold at least
  // 'newMinLength' non-null chars
  void grow(int newMinLength);

  // this can be useful if you modify the string contents directly..
  // it's not really the intent of this class, though
  void adjustend(char* newend);

  // remove characters from the end of the string; 'newLength' must
  // be at least 0, and less than or equal to current length
  void truncate(int newLength);

  // make the string be the empty string, but don't change the
  // allocated space
  void clear() { string::clear(); push_back('\0'); pop_back(); }

  // concatenation, which is the purpose of this class
  stringBuilder& operator&= (char const *tail);

  // useful for appending substrings or strings with NUL in them
  void append(char const *tail, int length);
  using string::append;

  // append a given number of spaces; meant for contexts where we're
  // building a multi-line string; returns '*this'
  stringBuilder& indent(int amt);

  enum class _disabled1 {};
  enum class _disabled2 {};

  // sort of a mixture of Java compositing and C++ i/o strstream
  stringBuilder& operator << (rostring text) { return operator&=(text.c_str()); }
  stringBuilder& operator << (char const *text) { return operator&=(text); }
  stringBuilder& operator << (char c);
  stringBuilder& operator << (unsigned char c) { return operator<<((char)c); }
  stringBuilder& operator << (long i);
  stringBuilder& operator << (unsigned long i);
  stringBuilder& operator << (std::conditional_t <sizeof(intptr_t) == 8, intptr_t, _disabled1> i);
  stringBuilder& operator << (std::conditional_t <sizeof(uintptr_t) == 8, uintptr_t, _disabled2> i);
  stringBuilder& operator << (int i) { return operator<<((long)i); }
  stringBuilder& operator << (unsigned i) { return operator<<((unsigned long)i); }
  stringBuilder& operator << (short i) { return operator<<((long)i); }
  stringBuilder& operator << (unsigned short i) { return operator<<((long)i); }
  stringBuilder& operator << (double d);
  stringBuilder& operator << (void *ptr);     // inserts address in hex
  #ifndef LACKS_BOOL
    stringBuilder& operator << (bool b) { return operator<<((long)b); }
  #endif // LACKS_BOOL

  // useful in places where long << expressions make it hard to
  // know when arguments will be evaluated, but order does matter
  typedef stringBuilder& (*Manipulator)(stringBuilder &sb);
  stringBuilder& operator<< (Manipulator manip);

  // an experiment: hex formatting (something I've sometimes done by resorting
  // to sprintf in the past)
  class Hex {
  public:
    unsigned long value;

    Hex(unsigned long v) : value(v) {}
    Hex(Hex const &obj) : value(obj.value) {}
  };
  stringBuilder& operator<< (Hex const &h);
};
using SBHex = stringBuilder::Hex;

std::istream& operator>> (std::istream& is, stringBuilder& sb) = delete;

// ---------------------- misc utils ------------------------
// the real strength of this entire module: construct strings in-place
// using the same syntax as C++ iostreams.  e.g.:
//   puts(stringb("x=" << x << ", y=" << y));
#define stringb(expr) (stringBuilder() << expr)

// experimenting with dropping the () in favor of <<
// (the "c" can be interpreted as "constructor", or maybe just
// the successor to "b" above)
#define stringc stringBuilder()


// experimenting with using toString as a general method for datatypes
string toString(int i);
string toString(unsigned i);
string toString(char c);
string toString(long i);
string toString(char const *str);
string toString(float f);


// printf-like construction of a string; often very convenient, since
// you can use any of the formatting characters (like %X) that your
// libc's sprintf knows about
string stringf(char const *format, ...);
string vstringf(char const *format, va_list args);


#endif // STR_H
