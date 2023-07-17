// trace.cc            see license.txt for copyright and terms of use
// code for trace.h

#include "trace.h"     // this module
#include "str.h"       // string
#include "strtokp.h"   // StrtokParse
#include "nonport.h"   // getMilliseconds()
#include "xassert.h"   // xfailure

#include <unordered_set> // std::unordered_set<string>
#include <fstream>       // std::ofstream
#include <stdlib.h>      // getenv


// list of active tracers, initially empty
std::unordered_set<string> tracers;

// stream connected to /dev/null
std::ofstream devNullObj("/dev/null");
static std::ostream *devNull = &devNullObj;


void traceAddSys(char const *sysName)
{
  tracers.emplace(sysName);
}


void traceRemoveSys(char const *sysName)
{
  auto it = tracers.find(sysName);
  if (it != tracers.end()) {
    tracers.erase(it);
    return;
  }
  xfailure("traceRemoveSys: tried to remove system that isn't there");
}


bool tracingSys(char const *sysName)
{
  return tracers.find(sysName) != tracers.end();
}


void traceRemoveAll()
{
  tracers.clear();
}


std::ostream &trace(char const *sysName)
{
  if (tracingSys(sysName)) {
    std::cout << "%%% " << sysName << ": ";
    return std::cout;
  }
  else {
    return *devNull;
  }
}


void trstr(char const *sysName, char const *traceString)
{
  trace(sysName) << traceString << std::endl;
}


std::ostream &traceProgress(int level)
{
  if ( (level == 1) ||
       (level == 2 && tracingSys("progress2")) ) {
    static long progStart = getMilliseconds();

    return trace("progress") << (getMilliseconds() - progStart) << "ms: ";
  }
  else {
    return *devNull;
  }
}


void traceAddMultiSys(char const *systemNames)
{
  StrtokParse tokens(systemNames, ",");
  for (string_view tok : tokens) {
    if (tok.starts_with('-')) {
      // treat a leading '-' as a signal to *remove*
      // a tracing flag, e.g. from some defaults specified
      // statically
      string_view name = tok.substr(1);
      // tokens are guaranteed to be null-terminated
      if (tracingSys(name.data())) {      // be forgiving here
        traceRemoveSys(name.data());
      }
      else {
        std::cout << "Currently, `" << name << "' is not being traced.\n";
      }
    }
    else {
      // normal behavior: add things to the trace list
      // tokens are guaranteed to be null-terminated
      traceAddSys(tok.data());
    }
  }
}


bool traceProcessArg(int &argc, char **&argv)
{
  traceAddFromEnvVar();

  if (argc >= 3  &&  0==strcmp(argv[1], "-tr")) {
    traceAddMultiSys(argv[2]);
    argc -= 2;
    argv += 2;
    return true;
  }
  else {
    return false;
  }
}


bool ignoreTraceEnvVar = false;

void traceAddFromEnvVar()
{
  if (ignoreTraceEnvVar) {
    return;
  }

  char const *var = getenv("TRACE");
  if (var) {
    traceAddMultiSys(var);
  }

  ignoreTraceEnvVar = true;
}


// EOF
