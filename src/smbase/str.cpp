// str.cpp            see license.txt for copyright and terms of use
// code for str.h
// Scott McPeak, 1995-2000  This file is public domain.

#include "str.h"            // this module

#include <istream>          // std::istream
#include <ctype.h>          // isspace
#include <fmt/format.h>     // fmt::println, etc.


// --------------------- stringBuilder ------------------


string& operator<< (string& str, void* ptr)
{
  fmt::basic_memory_buffer<char, 32> buf;
  fmt::format_to(fmt::appender(buf), "0x{:02X}", ptr);
  return str << string_view(buf.data(), buf.size());
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

string toString(void const *p)
{
  if (!p) {
    return "(null)";
  }
  else {
    return fmt::format("0x{:08X}", p);
  }
}

string formatStrParenStr(const char* str, uintptr_t val)
{
  return fmt::format("{}({})", str, val);
}



// ------------------ test code --------------------
#ifdef TEST_STR

void test(unsigned long val)
{
  fmt::println("{0} in hex: 0x{0:02X}", val);
}

int main()
{
  // for the moment I just want to test the hex formatting
  test(64);
  test(0xFFFFFFFF);
  test(0);
  test((unsigned long)(-1));
  test(1);

  fmt::println("tests passed");

  return 0;
}

#endif // TEST_STR
