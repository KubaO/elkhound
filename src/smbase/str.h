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
#include "fmt/format.h"  // fmt::string_view, memory_buffer

#include <nonstd/string_view.hpp>
#include <string>
#include <type_traits>


// ------------------------- common declarations ---------------------

using string = std::string;
using string_view = nonstd::string_view;

template<> struct fmt::formatter<fmt::memory_buffer> : formatter<fmt::string_view>
{
  using Base = formatter<fmt::string_view>;
  format_context::iterator format(const memory_buffer &buf, format_context& ctx)
  {
    return Base::format(fmt::string_view(buf.data(), buf.size()), ctx);
  }
};

// ------------------------- string ---------------------

// concatenation
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


// ---------------------- misc utils ------------------------

// experimenting with using toString as a general method for datatypes
#ifdef FMT_VERSION
  template <typename T, class en =
    std::enable_if_t<!std::is_pointer_v<T>>>
  string toString(const T& val)
  {
    return fmt::to_string(val);
  }
#else
  string toString(int i);
  string toString(unsigned i);
  string toString(char c);
  string toString(long i);
  string toString(char const *str);
  string toString(float f);
#endif

string toString(char const* str);


// printf-like construction of a string; often very convenient, since
// you can use any of the formatting characters (like %X) that your
// libc's sprintf knows about
string stringf(char const *format, ...);
string vstringf(char const *format, va_list args);


#endif // STR_H
