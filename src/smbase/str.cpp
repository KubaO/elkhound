// str.cpp            see license.txt for copyright and terms of use
// code for str.h
// Scott McPeak, 1995-2000  This file is public domain.

#include "str.h"            // this module

#include <stdlib.h>         // atoi
#include <iostream>         // ostream << char*


// ----------------------- rostring ---------------------


int atoi(rostring s)
{
  return atoi(s.c_str());
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

  std::cout << "tests passed\n";

  return 0;
}

#endif // TEST_STR
