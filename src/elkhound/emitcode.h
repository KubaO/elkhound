// emitcode.h            see license.txt for copyright and terms of use
// track state of emitted code so I can emit #line too
// check if line numbers

#ifndef EMITCODE_H
#define EMITCODE_H

#include <fstream>        // std::ofstream
#include "str.h"          // stringBuffer
#include "srcloc.h"       // SourceLoc

class EmitCode : public stringBuilder {
private:     // data
  std::ofstream os;    // stream to write to
  string fname;        // filename for emitting #line
  int line;            // current line number

public:      // funcs
  EmitCode(rostring fname);
  ~EmitCode();

  string const &getFname() const { return fname; }

  // get current line number; flushes internally
  int getLine();

  // flush data in stringBuffer to 'os'
  void flush();

  // emit a #line directive to restore reporting to the
  void restoreLine();

  // is a parameter used in a given body of a function
  // a heuristic that can be improved
  static bool isParamUsed(string_view name, string_view body);
};


// return a #line directive for the given location
string lineDirective(SourceLoc loc);



#endif // EMITCODE_H
