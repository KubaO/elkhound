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

  std::cout << "tests passed\n";

  return 0;
}

#endif // TEST_STR
