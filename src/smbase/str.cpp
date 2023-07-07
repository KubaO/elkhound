// str.cpp            see license.txt for copyright and terms of use
// code for str.h
// Scott McPeak, 1995-2000  This file is public domain.

#include "str.h"            // this module

#include <stdlib.h>         // atoi
#include <stdio.h>          // sprintf
#include <ctype.h>          // isspace
#include <string.h>         // strcmp
#include <iostream>         // ostream << char*
#include <inttypes.h>       // string printf formats

#include <assert.h>         // assert
#ifndef _MSC_VER
  #include <unistd.h>         // write
#else
  #include <windows.h>
#endif
#include "xassert.h"        // xassert
#include "ckheap.h"         // checkHeapNode
#include "flatten.h"        // Flatten
#include "nonport.h"        // vnprintf
#include "array.h"          // Array
#include "fmt/core.h"       // fmt::format

// ----------------------- string ---------------------


string operator&(string const& head, string const &tail)
{
  string dest(head);
  dest &= tail;
  return dest;
}

string& operator&=(string& head, string const &tail)
{
  int headLen = head.length();
  int tailLen = tail.length();
  head.reserve(headLen + tailLen + 1);
  head.append(tail);
  head.push_back('\0');
  head.pop_back();
  return head;
}


void readall(std::istream& is, string &into)
{
  const int block = 4096;
  into.clear();
  while (!is.fail())
  {
    int head = into.size();
    into.resize(head + block);
    is.read(&into[head], block);
    int read = is.gcount();
    xassert(read >= 0 && read <= block);
    if (read < block)
      into.resize(head + read);
  }
}


// ----------------------- rostring ---------------------
int strcmp(rostring s1, rostring s2)
  { return strcmp(s1.c_str(), s2.c_str()); }
int strcmp(rostring s1, char const *s2)
  { return strcmp(s1.c_str(), s2); }
int strcmp(char const *s1, rostring s2)
  { return strcmp(s1, s2.c_str()); }


char const *strstr(rostring haystack, char const *needle)
{
  return strstr(haystack.c_str(), needle);
}


int atoi(rostring s)
{
  return atoi(toCStr(s));
}

string substring(char const *p, int n)
{
  return string(p, n);
}


// --------------------- stringBuilder ------------------
stringBuilder::stringBuilder(int len)
{
  init(len);
}

void stringBuilder::init(int initSize)
{
  int size = initSize + EXTRA_SPACE + 1;     // +1 to be like string::setlength
  clear();
  reserve(size);
  resize(initSize + 1);
  pop_back();
}


void stringBuilder::dup(char const *str)
{
  assign(str);
}


stringBuilder::stringBuilder(char const *str)
{
  dup(str);
}


stringBuilder::stringBuilder(char const *str, int len)
{
  assign(str, len);
}


stringBuilder& stringBuilder::setlength(int newlen)
{
  clear();
  init(newlen);
  return *this;
}


void stringBuilder::adjustend(char* newend)
{
  char* s = &operator[](0);
  xassert(s <= newend  &&  newend < s + capacity());
  int newSize = newend - s;

  resize(newSize);
  push_back('\0');     // sm 9/29/00: maintain invariant
  pop_back();
}


void stringBuilder::truncate(int newLength)
{
  xassert(0 <= newLength && newLength <= length());
  adjustend(&operator[](0) + newLength);
}


stringBuilder& stringBuilder::operator&= (char const *tail)
{
  append(tail, strlen(tail));
  return *this;
}

void stringBuilder::append(char const *tail, int len)
{
  string::append(tail, len);
  push_back('\0');
  pop_back();
}


stringBuilder& stringBuilder::indent(int amt)
{
  xassert(amt >= 0);
  string::append(amt, ' ');
  push_back('\0');
  pop_back();
  return *this;
}


void stringBuilder::grow(int newMinLength)
{
  // I want at least EXTRA_SPACE extra
  int newMinSize = newMinLength + EXTRA_SPACE + 1;         // compute resulting allocated size

  // I want to grow at the rate of at least 50% each time
  int suggest = capacity() * 3 / 2;

  // see which is bigger
  newMinSize = max(newMinSize, suggest);

  reserve(newMinSize);
}


stringBuilder& stringBuilder::operator<< (char c)
{
  push_back(c);
  push_back('\0');
  pop_back();
  return *this;
}


#define MAKE_LSHIFT(Argtype, fmt)                        \
  stringBuilder& stringBuilder::operator<< (Argtype arg) \
  {                                                      \
    char buf[60];      /* big enough for all types */    \
    int len = sprintf(buf, fmt, arg);                    \
    if (len >= 60) {                                     \
      abort();    /* too big */                          \
    }                                                    \
    return *this << buf;                                 \
  }

using intptr_type = std::conditional_t <sizeof(intptr_t) == 8, intptr_t, stringBuilder::_disabled1>;
using uintptr_type = std::conditional_t <sizeof(uintptr_t) == 8, uintptr_t, stringBuilder::_disabled2>;

MAKE_LSHIFT(long, "%ld")
MAKE_LSHIFT(unsigned long, "%lu")
MAKE_LSHIFT(double, "%g")
MAKE_LSHIFT(void*, "%p")
MAKE_LSHIFT(intptr_type, PRIiPTR)
MAKE_LSHIFT(uintptr_type, PRIuPTR)

#undef MAKE_LSHIFT


stringBuilder& stringBuilder::operator<< (
  stringBuilder::Hex const &h)
{
  char buf[32];        // should only need 19 for 64-bit word..
  int len = sprintf(buf, "0x%lX", h.value);
  if (len >= 20) {
    abort();
  }
  return *this << buf;

  // the length check above isn't perfect because we only find out there is
  // a problem *after* trashing the environment.  it is for this reason I
  // use 'assert' instead of 'xassert' -- the former calls abort(), while the
  // latter throws an exception in anticipation of recoverability
}


stringBuilder& stringBuilder::operator<< (Manipulator manip)
{
  return manip(*this);
}


// ---------------------- toString ---------------------
#ifndef FMT_VERSION
  #define TOSTRING(type)          \
    string toString(type val)     \
    {                             \
      return std::to_string(val); \
    }

  TOSTRING(int)
  TOSTRING(unsigned)
  TOSTRING(char)
  TOSTRING(long)
  TOSTRING(float)

  #undef TOSTRING
#endif // FMT_VERSION

// this one is more liberal than 'stringc << null' because it gets
// used by the PRINT_GENERIC macro in my astgen tool
string toString(char const *str)
{
  return str ? str : "(null)";
}


// ------------------- stringf -----------------
string stringf(char const *format, ...)
{
  va_list args;
  va_start(args, format);
  string ret = vstringf(format, args);
  va_end(args);
  return ret;
}


// this should eventually be put someplace more general...
#ifndef va_copy
  #ifdef __va_copy
    #define va_copy(a,b) __va_copy(a,b)
  #else
    #define va_copy(a,b) (a)=(b)
  #endif
#endif


string vstringf(char const *format, va_list args)
{
  // estimate string length
  va_list args2;
  va_copy(args2, args);
  int est = vnprintf(format, args2);
  va_end(args2);

  // allocate space
  Array<char> buf(est+1);

  // render the string
  int len = vsprintf(buf, format, args);

  // check the estimate, and fail *hard* if it was low, to avoid any
  // possibility that this might become exploitable in some context
  // (do *not* turn this check off in an NDEGUG build)
  if (len > est) {
    // don't go through fprintf, etc., because the state of memory
    // makes that risky
    static char const msg[] =
      "fatal error: vnprintf failed to provide a conservative estimate,\n"
      "memory is most likely corrupted\n";
	#ifdef _MSC_VER
	  HANDLE stderrH = GetStdHandle(STD_ERROR_HANDLE);
	  DWORD dontCare1;
	  WriteFile(stderrH, msg, strlen(msg), &dontCare1, NULL);
	#else
      write(2 /*stderr*/, msg, strlen(msg));
	#endif
    abort();
  }

  // happy
  return string(buf);
}


// ------------------ test code --------------------
#ifdef TEST_STR

#include <iostream>    // std::cout

void test(unsigned long val)
{
  //std::cout << stringb(val << " in hex: 0x" << stringBuilder::Hex(val)) << std::endl;

  std::cout << fmt::format("{} in hex: 0x{:X}", val, val) << std::endl;
}

int main()
{
  // for the moment I just want to test the hex formatting
  test(64);
  test(0xFFFFFFFF);
  test(0);
  test((unsigned long)(-1));
  test(1);

  std::cout << "stringf: " << stringf("int=%d hex=%X str=%s char=%c float=%f",
                                 5, 0xAA, "hi", 'f', 3.4) << std::endl;

  std::cout << "tests passed\n";

  return 0;
}

#endif // TEST_STR
