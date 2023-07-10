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

#include <iostream>      // istream, ostream
#include <stdarg.h>      // va_list
#include <string.h>      // strcmp, etc.
#include <type_traits>
#include <string>        // std::string

class Flatten;           // flatten.h


// ------------------------- string ---------------------

// concatenation
// uses '&' instead of '+' to avoid char* coercion problems
std::string operator& (const std::string& head, const std::string& tail);

// ------------------ porting compatibility ------------------

using string = std::string;
using rostring = const std::string&;
using stringBuilder = std::string;

// ------------------------ rostring ----------------------
// My plan is to use this in places I currently use 'char const *'.

// I need some compatibility functions
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

// construct a string out of characters from 'p' up to 'p+n-1',
// inclusive; resulting string length is 'n'
string substring(char const *p, int n);
inline string substring(rostring p, int n) { return p.substr(0, n); }

// --------------------- stringBuilder remnants --------------------

// append a given number of spaces; meant for contexts where we're
// building a multi-line string; returns '*this'
inline std::string& indent(std::string& str, int amt) {
  return str.append(amt, ' ');
}

enum class _disabled1 {};
enum class _disabled2 {};

// not-very-efficient compatibility stand-ins
inline string& operator << (string& str, rostring text) { return str.append(text);  }
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
string& operator << (string& str, const char (&text)[N]) { return str.append((const char*)text, n - 1); }

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
// the real strength of this entire module: construct strings in-place
// using the same syntax as C++ iostreams.  e.g.:
//   puts(stringb("x=" << x << ", y=" << y));
#define stringb(expr) (string() << expr)

// experimenting with dropping the () in favor of <<
// (the "c" can be interpreted as "constructor", or maybe just
// the successor to "b" above)
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
