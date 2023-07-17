// cparse.cc            see license.txt for copyright and terms of use
// code for cparse.h

#include <iostream>      // std::cout

#include "cparse.h"      // this module
#include "cc_lang.h"     // CCLang
#include "trace.h"       // trace

ParseEnv::ParseEnv(StringTable &table, CCLang &L)
  : str(table),
    intType(table.add("int")),
    strRefAttr(table.add("attr")),
    types(),
    lang(L)
{}

ParseEnv::~ParseEnv()
{}


static char const *identity(void *data)
{
  return (char const*)data;
}

void ParseEnv::enterScope()
{
  types.emplace(identity);
}

void ParseEnv::leaveScope()
{
  types.pop();
}

void ParseEnv::addType(StringRef type)
{
  StringHash &h = types.top();
  if (h.get(type)) {
    // this happens for C++ code which has both the implicit
    // and explicit typedefs (and/or, explicit 'class Foo' mentions
    // in places)
    //std::cout << "duplicate entry for " << type << " -- will ignore\n";
  }
  else {
    h.add(type, (void*)type);
  }
}

bool ParseEnv::isType(StringRef name)
{
  if (name == intType) {
    return true;
  }

  for (auto const& iter : types) {
    if (iter.get(name)) {
      return true;
    }
  }
  return false;
}


void ParseEnv::declareClassTag(StringRef tagName)
{
  // TYPE/NAME
  if (lang.tagsAreTypes) {
    #ifndef NDEBUG
    trace("cc") << "defined new struct/class tag as type " << tagName << std::endl;
    #endif
    addType(tagName);
  }
}
