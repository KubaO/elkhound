// emitcode.h            see license.txt for copyright and terms of use
// track state of emitted code so I can emit #line too

#ifndef EMITCODE_H
#define EMITCODE_H

#include <cstdio>         // FILE*
#include "str.h"          // string, string_view
#include "srcloc.h"       // SourceLoc

class EmitCode : public string {
  // This is a bit of an unnecessary hack.
  // Data to be emitted is buffered in the string.
  // Then, any time a line number is needed, the string is
  // scanned for newlines, and then flushed to output stream.
  // This adds a 2nd layer of buffering on top of what the
  // C runtime provides. In the long run, something simpler
  // or with less overhead should be used instead.

private:     // data
  FILE* file;          // stream to write to
  string fname;        // filename for emitting #line
  int line;            // current line number

public:      // funcs
  EmitCode(string_view fname);
  ~EmitCode();

  // get current line number; flushes internally
  int getLine();

  // flush data in stringBuffer to 'os'
  void flush();

  // emit a #line directive to restore reporting to the
  // EmitCode file itself
  void restoreLine();
};


// return a #line directive for the given location
string lineDirective(SourceLoc loc);

#endif // EMITCODE_H
