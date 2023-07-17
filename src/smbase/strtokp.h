// strtokp.h            see license.txt for copyright and terms of use
// using strtok to parse an entire string at once
// Scott McPeak, 1997  This file is public domain.

#ifndef __STRTOKP_H
#define __STRTOKP_H

#include "str.h"       // string
#include <vector>


class StrtokParse {
  const string orig;                // original string
  string buf;                       // string with tokens NUL-terminated
  std::vector<string_view> tokens;  // the tokens found

private:
  void validate(int which) const;
    // throw an exception if which is invalid token

public:
  StrtokParse(string_view str, const char* delim);
    // parse 'str' into tokens delimited by chars from 'delim'

  ~StrtokParse();

  auto begin() const { return tokens.begin(); }
  auto end() const { return tokens.end(); }

  int size() const { return tokens.size(); }

  string_view tokv(int which) const;     // may throw xArrayBounds
  string_view operator[] (int which) const { return tokv(which); }
    // access to tokens; must make local copies to modify

  string_view reassemble(int firstTok, int lastTok) const;
    // return the substring of the original string spanned by the
    // given range of tokens; if firstTok==lastTok, only that token is
    // returned (without any separators); must be that firstTok <=
    // lastTok

  string join(int firstTok, int lastTok, string_view separator) const;
    // return a string created by concatenating the given range of tokens
    // together with 'separator' in between them

  int offset(int which) const;
    // return a value that, when added to the original 'str' parameter,
    // yields a pointer to where tokv(which) is, as a substring, in that string

  int offsetAfter(int which) const;
    // offset for character just beyond last one in tokv (should be either
    // a delimiter character, or 0)
};

#endif // __STRTOKP_H
