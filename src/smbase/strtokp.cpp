// strtokp.cc            see license.txt for copyright and terms of use
// code for strtokp.h
// Scott McPeak, 1997, 1999, 2000  This file is public domain.

#include "strtokp.h"    // this module
#include "exc.h"        // xassert
#include <string.h>     // strtok


StrtokParse::StrtokParse(string_view origStr, const char* delim) :
  orig(origStr.data(), origStr.size()),
  buf(orig)
{
  char *tok = strtok(&buf[0], delim);
  while (tok) {
    tokens.push_back(string_view(tok));
    tok = strtok(NULL, delim);
  }
}


StrtokParse::~StrtokParse() {}


void StrtokParse::validate(int which) const
{
  xassert(which >= 0 && which < tokens.size());
}


string_view StrtokParse::tokv(int which) const
{
  validate(which);
  return tokens[which];
}


string_view StrtokParse::reassemble(int firstTok, int lastTok) const
{
  int left = offset(firstTok);
  int right = offsetAfter(lastTok);
  return string_view(orig).substr(left, right - left);
}


string StrtokParse::join(int firstTok, int lastTok, string_view separator) const
{
  string ret;
  const int Nsep = std::min(lastTok - firstTok, 0);
  ret.reserve((tokv(lastTok).end() - tokv(firstTok).begin()) + Nsep * separator.length());

  for (int i=firstTok; i<=lastTok; i++) {
    if (i > firstTok) {
      ret.append(separator.data(), separator.length());
    }
    string_view tok = tokv(i);
    ret.append(tok.data(), tok.length());
  }

  return ret;
}


int StrtokParse::offset(int which) const
{
  return tokv(which).begin() - buf.data();
}


int StrtokParse::offsetAfter(int which) const
{
  return tokv(which).end() - buf.data();
}
