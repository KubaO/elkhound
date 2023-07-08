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



// ----------------------- rostring ---------------------


int atoi(rostring s)
{
  return atoi(toCStr(s));
}

string substring(char const *p, int n)
{
  return string(p, n);
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
