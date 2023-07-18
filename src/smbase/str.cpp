// str.cpp            see license.txt for copyright and terms of use
// code for str.h
// Scott McPeak, 1995-2000  This file is public domain.

#include "str.h"            // this module

#include <istream>          // std::istream
#include <stdio.h>          // sprintf
#include <ctype.h>          // isspace
#include <inttypes.h>       // string printf formats

#include <assert.h>         // assert
#include "xassert.h"        // xassert
#include "ckheap.h"         // checkHeapNode
#include "flatten.h"        // Flatten
#include "nonport.h"        // vnprintf


// ----------------------- string ---------------------

std::string operator& (const std::string& head, const std::string& tail)
{
  std::string ret;
  ret.reserve(head.length() + tail.length() + 1);
  ret.append(head);
  ret.append(tail);
  return ret;
}

// --------------------- stringBuilder ------------------


string& operator<< (string& str, void* ptr)
{
  return str << SBHex(intptr_t(ptr));
}


string& operator<< (string& str, SBHex h)
{
  char buf[32];        // should only need 19 for 64-bit word..
  size_t len = sprintf(buf, "0x%" PRIXPTR, h.value);
  assert(len <= sizeof(buf));
  return str << buf;
}


// slow but reliable
string readdelim(std::istream &is, char const *delim)
{
  string ret;
  char c;
  is.get(c);
  while (!is.eof() &&
         (!delim || !strchr(delim, c))) {
    ret.push_back(c);
    is.get(c);
  }
  return ret;
}


// ---------------------- toString ---------------------
#define TOSTRING(type)             \
  string toString(type val)        \
  {                                \
    return std::to_string((val));  \
  }

TOSTRING(int)
TOSTRING(unsigned)
TOSTRING(char)
TOSTRING(long)
TOSTRING(float)

#undef TOSTRING

// this one is more liberal than 'stringc << null' because it gets
// used by the PRINT_GENERIC macro in my astgen tool
string toString(char const *str)
{
  if (!str) {
    return string("(null)");
  }
  else {
    return string(str);
  }
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
  string ret(est + 1, '\0');

  // render the string
  int len = vsprintf(&ret[0], format, args);

  // check the estimate, and fail *hard* if it was low, to avoid any
  // possibility that this might become exploitable in some context
  // (do *not* turn this check off in an NDEGUG build)
  if (len > est) {
    // don't go through fprintf, etc., because the state of memory
    // makes that risky
    static char const msg[] =
      "fatal error: vnprintf failed to provide a conservative estimate,\n"
      "memory is most likely corrupted\n";
    fprintf(stderr, "%s", msg);
    fflush(stderr);
    abort();
  }

  // happy
  ret.resize(len);
  return ret;
}


// ------------------ test code --------------------
#ifdef TEST_STR

#include <iostream>    // std::cout

void test(unsigned long val)
{
  //std::cout << stringb(val << " in hex: 0x" << stringBuilder::Hex(val)) << std::endl;

  std::cout << (stringc << val << " in hex: " << SBHex(val)) << std::endl;
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
