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

class Flatten;           // flatten.h


// ------------------------- string ---------------------
// This is used when I want to call a function in smbase::string
// that does not exist or has different semantics in std::string.
// That way for now I can keep using the function, but it is
// marked as incompatible.
enum SmbaseStringFunc { SMBASE_STRING_FUNC };

class string {
protected:     // data
  // 10/12/00: switching to never letting s be NULL
  char *s;                             // string contents; never NULL
  static char * const emptyString;     // a global ""; should never be modified

protected:     // funcs
  void dup(char const *source);        // copies, doesn't dealloc first
  void kill();                         // dealloc if str != 0

public:        // funcs
  string(string const &src) { dup(src.s); }
  string(char const *src) { dup(src); }
  string() { s=emptyString; }
  ~string() { kill(); }

  // for this one, use ::substring instead
  string(char const *src, int length, SmbaseStringFunc);

  // for this one, there are two alternatives:
  //   - stringBuilder has nearly the same constructor interface
  //     as string had, but cannot export a char* for writing
  //     (for the same reason string can't anymore); operator[] must
  //     be used
  //   - Array<char> is very flexible, but remember to add 1 to
  //     the length passed to its constructor!
  string(int length, SmbaseStringFunc) { s=emptyString; setlength(length); }

  string(Flatten&) = delete;
  void xfer(Flatten &flat);

  // simple queries
  int length() const;           // returns number of non-null chars in the string; length of "" is 0
  bool empty() const { return s[0] == 0; }
  bool contains(char c) const;

  // array-like access
  char& operator[] (int i) { return s[i]; }
  char operator[] (int i) const { return s[i]; }

  // substring
  string substring(int startIndex, int length) const;

  // conversions
  char const *c_str() const { return s; }

  // assignment
  string& operator=(string const &src)
    { if (&src != this) { kill(); dup(src.s); } return *this; }
  string& operator=(char const *src)
    { if (src != s) { kill(); dup(src); } return *this; }

  // allocate 'newlen' + 1 bytes (for null); initial contents is ""
  string& setlength(int newlen);

  // comparison; return value has same meaning as strcmp's return value:
  //   <0   if   *this < src
  //   0    if   *this == src
  //   >0   if   *this > src
  int compareTo(string const &src) const;
  int compareTo(char const *src) const;
  bool equals(char const *src) const { return compareTo(src) == 0; }
  bool equals(string const &src) const { return compareTo(src) == 0; }

  #define MAKEOP(op)                                                             \
    bool operator op (string const &src) const { return compareTo(src) op 0; }   \
    /*bool operator op (const char *src) const { return compareTo(src) op 0; }*/ \
    /* killed stuff with char* because compilers are too flaky; use compareTo */
  MAKEOP(==)  MAKEOP(!=)
  MAKEOP(>=)  MAKEOP(>)
  MAKEOP(<=)  MAKEOP(<)
  #undef MAKEOP

  // concatenation (properly handles string growth)
  // uses '&' instead of '+' to avoid char* coercion problems
  string operator& (string const &tail) const;

  // input/output
  friend std::ostream& operator<< (std::ostream& os, string const& obj) { return os << obj.s; }

  // debugging
  void selfCheck() const;
    // fail an assertion if there is a problem
};

std::istream& operator>> (std::istream& is, string& obj) = delete;

// ------------------------ rostring ----------------------
// My plan is to use this in places I currently use 'char const *'.
typedef string const &rostring;

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

// construct a string out of characters from 'p' up to 'p+n-1',
// inclusive; resulting string length is 'n'
string substring(char const *p, int n);
inline string substring(rostring p, int n) { return p.substring(0, n); }


// --------------------- stringBuilder --------------------
// this class is specifically for appending lots of things
class stringBuilder : public string {
protected:
  enum { EXTRA_SPACE = 30 };    // extra space allocated in some situations
  char *end;          // current end of the string (points to the NUL character)
  int size;           // amount of space (in bytes) allocated starting at 's'

protected:
  void init(int initSize);
  void dup(char const *src);

public:
  stringBuilder(int length=0);    // creates an empty string
  stringBuilder(char const *str);
  stringBuilder(char const *str, int length);
  stringBuilder(string const &str) { dup(str.c_str()); }
  stringBuilder(stringBuilder const &obj) { dup(obj.c_str()); }
  ~stringBuilder() {}

  stringBuilder& operator= (char const *src);
  stringBuilder& operator= (string const &s) { return operator= (s.c_str()); }
  stringBuilder& operator= (stringBuilder const &s) { return operator= (s.c_str()); }

  int length() const { return end-s; }
  bool empty() const { return length()==0; }

  // unlike 'string' above, I will allow stringBuilder to convert to
  // char const * so I can continue to use 'stringc' to build strings
  // for functions that accept char const *; this should not conflict
  // with std::string, since I am explicitly using a different class
  // (namely stringBuilder) when I use this functionality
  operator char const * () const { return c_str(); }

private:
  using string::setlength; // hide it
public:

  // make sure we can store 'someLength' non-null chars; grow if necessary
  void ensure(int someLength) { if (someLength >= size) { grow(someLength); } }

  // grow the string's length (retaining data); make sure it can hold at least
  // 'newMinLength' non-null chars
  void grow(int newMinLength);

  // this can be useful if you modify the string contents directly..
  // it's not really the intent of this class, though
  void adjustend(char* newend);

  // make the string be the empty string, but don't change the
  // allocated space
  void clear() { adjustend(s); }

  // useful for appending substrings or strings with NUL in them
  void append(char const *tail, int length);

  // append a given number of spaces; meant for contexts where we're
  // building a multi-line string; returns '*this'
  stringBuilder& indent(int amt);

  enum class _disabled1 {};
  enum class _disabled2 {};

  // sort of a mixture of Java compositing and C++ i/o strstream
  stringBuilder& operator << (rostring text) { append(text.c_str(), text.length()); return *this;  }
  stringBuilder& operator << (char const* text) { append(text, strlen(text)); return *this;  }
  stringBuilder& operator << (char c);
  stringBuilder& operator << (unsigned char c) { return operator<<((char)c); }
  stringBuilder& operator << (long i);
  stringBuilder& operator << (unsigned long i);
  stringBuilder& operator << (std::conditional_t <sizeof(intptr_t) == 8, intptr_t, _disabled1> i);
  stringBuilder& operator << (std::conditional_t <sizeof(uintptr_t) == 8, uintptr_t, _disabled2> i);
  stringBuilder& operator << (int i) { return operator<<((long)i); }
  stringBuilder& operator << (unsigned i) { return operator<<((unsigned long)i); }
  stringBuilder& operator << (short i) { return operator<<((long)i); }
  stringBuilder& operator << (unsigned short i) { return operator<<((long)i); }
  stringBuilder& operator << (double d);
  stringBuilder& operator << (void *ptr);     // inserts address in hex
  stringBuilder& operator << (bool b) { return operator<<((long)b); }

  // useful in places where long << expressions make it hard to
  // know when arguments will be evaluated, but order does matter
  typedef stringBuilder& (*Manipulator)(stringBuilder &sb);
  stringBuilder& operator<< (Manipulator manip);

  void readdelim(std::istream &is, char const *delim);

  // an experiment: hex formatting (something I've sometimes done by resorting
  // to sprintf in the past)
  class Hex {
  public:
    unsigned long value;

    Hex(unsigned long v) : value(v) {}
    Hex(Hex const &obj) : value(obj.value) {}
  };
  stringBuilder& operator<< (Hex const &h);
  #define SBHex stringBuilder::Hex
};

std::istream& operator>> (std::istream& is, stringBuilder& sb) = delete;


// ---------------------- misc utils ------------------------
// the real strength of this entire module: construct strings in-place
// using the same syntax as C++ iostreams.  e.g.:
//   puts(stringb("x=" << x << ", y=" << y));
#define stringb(expr) (stringBuilder() << expr)

// experimenting with dropping the () in favor of <<
// (the "c" can be interpreted as "constructor", or maybe just
// the successor to "b" above)
#define stringc stringBuilder()


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
