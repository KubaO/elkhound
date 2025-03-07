// emitcode.cc            see license.txt for copyright and terms of use
// code for emitcode.h

#include "emitcode.h"      // this module
#include "syserr.h"        // xsyserror
#include "srcloc.h"        // SourceLoc
#include "trace.h"         // tracingSys
#include <string.h>        // memcpy

EmitCode::EmitCode(rostring f)
  : os(f),
    fname(f),
    line(1)
{
  if (!os) {
    xsyserror("open", fname);
  }
}

EmitCode::~EmitCode()
{
  flush();
}


int EmitCode::getLine()
{
  flush();
  return line;
}


void EmitCode::flush()
{
  // count newlines
  char const *p = c_str();
  while (*p) {
    if (*p == '\n') {
      line++;
    }
    p++;
  }

  #if 0
    // this is the original code
    os << *this;
  #else
    // 2005-06-28: There is a bug in the cygwin implementation of
    // std::ofstream::operator<< that causes a stack overflow segfault
    // when writing strings longer than about 2MB.  So, I will
    // manually break up the string into little chunks to write it.

    // how long is the string?
    int len = p - c_str();

    enum { SZ = 0x1000 };       // write in 4k chunks
    p = c_str();

    while (len >= SZ) {
      char buf[SZ+1];
      memcpy(buf, p, SZ);
      buf[SZ] = 0;

      os << buf;

      p += SZ;
      len -= SZ;
    }

    os << p;
  #endif

  clear();
}


char const *hashLine()
{
  if (tracingSys("nolines")) {
    // emit with comment to disable its effect
    return "// #line ";
  }
  else {
    return "#line ";
  }
}


// note that #line must be preceeded by a newline
string lineDirective(SourceLoc loc)
{
  char const *fname;
  int line, col;
  SourceLocManager::instance()->decodeLineCol(loc, fname, line, col);

  std::string cfname;
  for (const char* p = fname; *p; p++)
  {
    char c = *p;
    if (c == '\\')
      cfname.append("\\\\");
    else
      cfname.push_back(c);
  }

  return fmt::format("{}{} \"{}\"\n", hashLine(), line, cfname);
}

void EmitCode::restoreLine()
{
  // +1 because we specify what line will be *next*
  int line = getLine()+1;
  *this << hashLine() << line
        << " \"" << getFname() << "\"\n";
}


STATICDEF bool EmitCode::isParamUsed(string_view name, string_view body)
{
  // This is a very simple heuristic. C lexer1 should be used instead
  // to see if a token equal to the name is exists in the body
  return body.find(name) != string_view::npos;
}