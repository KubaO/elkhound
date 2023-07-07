// emitcode.cc            see license.txt for copyright and terms of use
// code for emitcode.h

#include <algorithm>       // std::count
#include "emitcode.h"      // this module
#include "syserr.h"        // xsyserror
#include "trace.h"         // tracingSys
#include "fmt/format.h"    // fmt::format

EmitCode::EmitCode(string_view f)
  : file(NULL),
    fname(f),
    line(1)
{
  file = ::fopen(fname.c_str(), "w");
  if (!file) {
    xsyserror("open", fname);
  }
}

EmitCode::~EmitCode()
{
  flush();
  if (file)
    ::fclose(file);
}


int EmitCode::getLine()
{
  flush();
  return line;
}


void EmitCode::flush()
{
  // count newlines
  line += std::count(begin(), end(), '\n');
  // write out the data
  if (::fwrite(data(), 1, size(), file) != size())
    xsyserror("fwrite", fname);
  clear();
}


static string_view hashLine()
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

  fmt::memory_buffer buf;
  for (const char* p = fname; *p; p++)
  {
    char c = *p;
    if (c == '\\') {
      buf.push_back('\\');
      buf.push_back('\\');
    }
    else
      buf.push_back(c);
  }

  return fmt::format("{}{} \"{}\"\n", hashLine(), line, buf);
}

void EmitCode::restoreLine()
{
  // +1 because we specify what line will be *next*
  int line = getLine()+1;

  fmt::format_to(std::back_inserter(*this), "{}{} \"{}\"\n",
    hashLine(), line, fname);
}
