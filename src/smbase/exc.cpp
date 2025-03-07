// exc.cc            see license.txt for copyright and terms of use
// code for exc.h
// Scott McPeak, 1996-1998  This file is public domain.

#include "exc.h"          // this module

#include <string.h>       // strlen, strcpy
#include <iostream>       // std::clog
#include <stdarg.h>       // va_xxx
#include <ctype.h>        // toupper, tolower


// ------------------------- xBase -----------------
bool xBase::logExceptions = true;
int xBase::creationCount = 0;


xBase::xBase(rostring m)
  : msg(m)
{
  if (logExceptions) {
    std::clog << "Exception thrown: " << m << std::endl;
  }

  // done at very end when we know this object will
  // successfully be created
  creationCount++;
}


xBase::xBase(xBase const &obj)
  : msg(obj.msg)
{
  creationCount++;
}


xBase::~xBase()
{
  creationCount--;
}


// this is obviously not perfect, since exception objects can be
// created and not thrown; I heard the C++ standard is going to,
// or already does, include (by this name?) a function that does this
// correctly; until then, this will serve as a close approximation
// (this kind of test is, IMO, not a good way to handle the underlying
// problem, but it does reasonably handle 70-90% of the cases that
// arise in practice, so I will endorse it for now)
bool unwinding()
{
  return xBase::creationCount != 0;
}


// tweaked version
bool unwinding_other(xBase const &)
{
  // we know the passed xBase exists.. any others?
  return xBase::creationCount > 1;
}


void xBase::insert(std::ostream &os) const
{
  os << why();
}


void xbase(rostring msg)
{
  xBase x(msg);
  THROW(x);
}


void xBase::addContext(rostring context)
{
  // for now, fairly simple
  msg = fmt::format("while {},\n{}", context, msg);
}


// ------------------- x_assert -----------------
x_assert::x_assert(rostring cond, rostring fname, int line)
  : xBase(fmt::format(
      "Assertion failed: {}, file {} line {}",
      cond, fname, line)),
    condition(cond),
    filename(fname),
    lineno(line)
{}

x_assert::x_assert(x_assert const &obj)
  : xBase(obj),
    condition(obj.condition),
    filename(obj.filename),
    lineno(obj.lineno)
{}

x_assert::~x_assert()
{}


// failure function, declared in xassert.h
void x_assert_fail(char const* file, int line, char const *cond)
{
  THROW(x_assert(cond, file, line));
}


// --------------- xFormat ------------------
xFormat::xFormat(rostring cond)
  : xBase(fmt::format("Formatting error: {}", cond)),
    condition(cond)
{}

xFormat::xFormat(xFormat const &obj)
  : xBase(obj),
    condition(obj.condition)
{}

xFormat::~xFormat()
{}


void xformat(rostring condition)
{
  xFormat x(condition);
  THROW(x);
}

void formatAssert_fail(char const *cond, char const *file, int line)
{
  xFormat x(fmt::format("format assertion failed, {}:{}: {}",
                        file, line, cond));
  THROW(x);
}


// -------------------- XOpen -------------------
XOpen::XOpen(rostring fname)
  : xBase(fmt::format("failed to open file: {}", fname)),
    filename(fname)
{}

XOpen::XOpen(XOpen const &obj)
  : xBase(obj),
    filename(obj.filename)
{}

XOpen::~XOpen()
{}


void throw_XOpen(rostring fname)
{
  XOpen x(fname);
  THROW(x);
}


// -------------------- XOpenEx ---------------------
XOpenEx::XOpenEx(rostring fname, rostring m, rostring c)
  : XOpen(fname),
    mode(m),
    cause(c)
{
  msg = fmt::format("failed to open file\"{}\" for {}: {}",
                    fname, interpretMode(mode), cause);
}

XOpenEx::XOpenEx(XOpenEx const &obj)
  : XOpen(obj),
    mode(obj.mode),
    cause(obj.cause)
{}

XOpenEx::~XOpenEx()
{}


STATICDEF string XOpenEx::interpretMode(rostring mode)
{
  if (mode[0]=='r') {
    if (mode[1]=='+') {
      return "reading and writing";
    }
    else {
      return "reading";
    }
  }

  if (mode[0]=='w') {
    if (mode[1]=='+') {
      return "reading and writing";
    }
    else {
      return "writing";
    }
  }

  if (mode[0]=='a') {
    if (mode[1]=='+') {
      return "reading and appending";
    }
    else {
      return "appending";
    }
  }

  return fmt::format("(unknown action mode \"{}\")", mode);
}


void throw_XOpenEx(rostring fname, rostring mode, rostring cause)
{
  XOpenEx x(fname, mode, cause);
  THROW(x);
}


// -------------------- XUnimp -------------------
XUnimp::XUnimp(rostring msg)
  : xBase(fmt::format("unimplemented: {}", msg))
{}

XUnimp::XUnimp(XUnimp const &obj)
  : xBase(obj)
{}

XUnimp::~XUnimp()
{}


void throw_XUnimp(rostring msg)
{
  XUnimp x(msg);
  THROW(x);
}


void throw_XUnimp(char const *msg, char const *file, int line)
{
  throw_XUnimp(fmt::format("{}:{}: {}", file, line, msg));
}


// -------------------- XFatal -------------------
// That this error is "fatal" need not be stated in the error message
// itself.  Doing so would unnecessarily alarm novice users, and the
// fatal-ness is sufficiently expressed by the fact that an exception
// is thrown, as opposed to simply printing the message and continuing.
XFatal::XFatal(rostring msg)
  : xBase(fmt::format("error: {}", msg))
{}

XFatal::XFatal(XFatal const &obj)
  : xBase(obj)
{}

XFatal::~XFatal()
{}


void throw_XFatal(rostring msg)
{
  XFatal x(msg);
  THROW(x);
}


// ---------------- test code ------------------
#ifdef TEST_EXC

int main()
{
  xBase x("yadda");
  std::cout << x << std::endl;

  try {
    THROW(x);
  }
  catch (xBase &x) {
    std::cout << "caught xBase: " << x << std::endl;
  }

  return 0;
}

#endif // TEST_EXC
