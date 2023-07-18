// strtable.cc            see license.txt for copyright and terms of use
// code for strtable.h

#include "strtable.h"    // this module
#include "algo.h"        // sm::...
#include "xassert.h"     // xassert
#include "flatten.h"     // Flatten

#include <string.h>      // strlen


StringTable *flattenStrTable = NULL;


StringTable::StringTable()
  : racks(NULL),
    longStrings(NULL)
{}


StringTable::~StringTable()
{
  clear();
}

void StringTable::clear()
{
  hash.clear();

  while (racks != NULL) {
    Rack *temp = racks;
    racks = racks->next;
    delete temp;
  }

  while (longStrings != NULL) {
    LongString *temp = longStrings;
    longStrings = longStrings->next;
    free(temp);
  }
}


StringRef StringTable::add(string_view src)
{
  // see if it's already here
  auto it = hash.find(src);
  if (it != hash.end()) {
    return it->data();
  }

  string_view ret;
  int len = src.size()+1;     // include null terminator

  // is it a long string?
  if (len >= longThreshold) {
    // allocate a long string with a variable-sized tail
    LongString *ls = static_cast<LongString*>(malloc(sizeof(LongString) + src.size()));

    ls->next = longStrings;
    longStrings = ls;

    ret = string_view(ls->data, src.size());
  }

  else {
    // see if we need a new rack
    if (!racks || len > racks->availBytes()) {
      // need a new rack
      xassert(len <= rackSize);
      racks = new Rack(racks);     // prepend new rack
    }

    // add the string to the last rack
    ret = string_view(racks->nextByte(), src.size());
    racks->usedBytes += len;
  }

  // copy the string to the destination
  char* dst = const_cast<char*>(ret.data());
  src.copy(dst, ret.size());
  dst[ret.size()] = '\0';

  // enter the intended location into the indexing structures
  hash.insert(ret);

  return ret.data();
}


StringRef StringTable::get(string_view src) const
{
  auto it = hash.find(src);
  if (it != hash.end()) {
    return it->data();
  }
  return nullptr;
}


// for now, just store each instance separately ...
void StringTable::xfer(Flatten &flat, StringRef &ref)
{
  if (flat.reading()) {
    // read the string
    char *str;
    flat.xferCharString(str);

    if (str) {
      // add to table
      ref = add(str);

      // delete string's storage
      delete str;
    }
    else {
      // was NULL
      ref = NULL;
    }
  }

  else {
    // copy it to disk
    // since we're writing, the cast is ok
    flat.xferCharString(const_cast<char*&>(ref));
  }
}
