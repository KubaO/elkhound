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

#include <iostream>                // istream, ostream
#include <stdarg.h>                // va_list
#include <string.h>                // strcmp, etc.
#include <type_traits>
#include <string>                  // std::string
#include "nonstd/string_view.hpp"  // nonstd::string_view

class Flatten;           // flatten.h


// ------------------------- string ---------------------

// concatenation
// uses '&' instead of '+' to avoid char* coercion problems
std::string operator& (const std::string& head, const std::string& tail);

// ------------------ porting compatibility ------------------

using string = std::string;
using rostring = const std::string&;
using stringBuilder = std::string;
using string_view = nonstd::string_view;

// ------------------------ rostring ----------------------
// My plan is to use this in places I currently use 'char const *'.

// I need some compatibility functions
inline int strcmp(string_view s1, string_view s2) { return s1.compare(s2); }

inline bool streq(string_view s1, string_view s2) { return s1 == s2; }


// --------------------- stringBuilder remnants --------------------

// append a given number of spaces; meant for contexts where we're
// building a multi-line string; returns '*this'
inline std::string& indent(std::string& str, int amt) {
  return str.append(amt, ' ');
}

struct _disabled1 { operator int() { return 0; } };
struct _disabled2 { operator int() { return 0; } };

// not-very-efficient compatibility stand-ins
inline string& operator << (string& str, string_view text) { return str.append(text.data(), text.size());  }
// Do not remove the overload below. It's needed because there are some types that convert to char const *,
// but other - incorrect - overloads will kick in when this one is gone.
inline string& operator << (string& str, char const* text) { return str.append(text); }
inline string& operator << (string& str, char c) { str.push_back(c); return str; }
inline string& operator << (string& str, unsigned char c) { str.push_back(c); return str; }
inline string& operator << (string& str, long i) { return str.append(std::to_string(i)); }
inline string& operator << (string& str, unsigned long i) { return str.append(std::to_string(i)); }
inline string& operator << (string& str, std::conditional_t <sizeof(intptr_t) == 8, intptr_t, _disabled1> i)
  { return str.append(std::to_string(i)); }
inline string& operator << (string& str, std::conditional_t <sizeof(uintptr_t) == 8, uintptr_t, _disabled2> i)
  { return str.append(std::to_string(i)); }
inline string& operator << (string& str, int i) { return str.append(std::to_string(i)); }
inline string& operator << (string& str, unsigned i) { return str.append(std::to_string(i)); }
inline string& operator << (string& str, short i) { return str.append(std::to_string(i)); }
inline string& operator << (string& str, unsigned short i) { return str.append(std::to_string(i)); }
inline string& operator << (string& str, double d) { return str.append(std::to_string(d)); }
       string& operator << (string& str, void* ptr);
inline string& operator << (string& str, bool b) { return str.append(std::to_string((int)b)); }
template <std::size_t N>
string& operator << (string& str, const char (&text)[N]) { return str.append((const char*)text, N - 1); }

class C_Str {};
static constexpr C_Str c_str;
inline const char* operator << (string& str, C_Str) { return str.c_str(); }

string readdelim(std::istream &is, char const *delim);

struct SBHex {
  uintptr_t value;
  SBHex(uintptr_t v) : value(v) {}
};
string& operator<< (string& str, SBHex);


// ---------------------- misc utils ------------------------

// The "c" can be interpreted as "constructor".
#define stringc string()


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
