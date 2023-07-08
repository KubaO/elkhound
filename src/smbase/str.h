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

#include <istream>       // istream
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


// ------------------------ rostring ----------------------
// My plan is to use this in places I currently use 'char const *'.
typedef string const &rostring;

// I need some compatibility functions
inline int strcmp(string_view s1, string_view s2) { return s1.compare(s2); }

inline bool streq(string_view s1, string_view s2) { return s1 == s2; }

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


#endif // STR_H
