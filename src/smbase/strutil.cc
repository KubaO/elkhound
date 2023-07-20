// strutil.cc            see license.txt for copyright and terms of use
// code for strutil.h

#include "strutil.h"     // this module
#include "exc.h"         // xformat
#include "autofile.h"    // AutoFILE

#include <array>         // std::array
#include <ctype.h>       // isspace
#include <stdio.h>       // sprintf, FILE
#include <stdlib.h>      // strtoul
#include <time.h>        // time, asctime, localtime
#include <fmt/core.h>    // fmt::format


// replace all instances of oldstr in src with newstr, return result
string replace(string_view origSrc, string_view oldstr, string_view newstr)
{
  string ret;
  size_t src = 0, next = 0;

  if (! oldstr.empty()) {
    for (;;) {
      next = origSrc.find(oldstr.data(), src, oldstr.length());
      if (next == string::npos)
        break;

      // add the characters between src and next
      ret.append(origSrc.data() + src, next - src);

      // add the replace-with string
      ret.append(newstr.data(), newstr.length());

      // move src to beyond replaced substring
      src = next + oldstr.length();
    }
  }

  // add remaining characters
  ret.append(origSrc.data() + src, origSrc.length() - src);
  return ret;
}


static string expandRanges(string_view ranges)
{
  string ret;
  ret.reserve(256);

  size_t left = ranges.size();
  for (const char * chars = ranges.begin(); chars != ranges.end(); ) {
    if (left >= 3 && chars[1] == '-') {
      // range specification
      if (chars[0] > chars[2]) {
        xformat("range specification with wrong collation order");
      }
      for (char c = chars[0]; c <= chars[2]; c++) {
        ret << c;
      }
      chars += 3;
      left -= 3;
    }
    else {
      // simple character specification
      ret << chars[0];
      chars++;
      left--;
    }
  }

  ret.shrink_to_fit();
  return ret;
}


// run through 'src', applying 'map'
static string translate(string_view origSrc, const std::array<char, 256> &map)
{
  string ret;
  ret.reserve(origSrc.size());
  for (char ch : origSrc) {
    char d = map[(unsigned char)ch];
    ret.push_back(d);
  }
  return ret;
}


static std::array<char, 256> buildMap(string_view srcchars, string_view destchars)
{
  std::array<char, 256> map;

  // first, expand range notation in the specification sequences
  string srcSpec = expandRanges(srcchars);
  string destSpec = expandRanges(destchars);

  // build a translation map
  for (int i = 0; i < 256; i++) {
    map[i] = i;
  }

  // excess characters from either string are ignored ("SysV" behavior)
  for (int i = 0; i < srcSpec.length() && i < destSpec.length(); i++) {
    map[(unsigned char)(srcSpec[i])] = destSpec[i];
  }

  return map;
}


string translate(string_view origSrc, string_view srcchars, string_view destchars)
{
  std::array<char, 256> map = buildMap(srcchars, destchars);
  return translate(origSrc, map);
}

string stringToupper(string_view src)
{
  static std::array<char, 256> map = buildMap(string_view("a-z"), string_view("A-Z"));
  return translate(src, map);
}


string trimWhitespace(string_view origStr)
{
  char const* str = origStr.begin();
  char const* end = origStr.end();

  // trim leading whitespace
  while (str != end && isspace(*str)) {
    str++;
  }

  // trim trailing whitespace
  while (end != str && isspace(end[-1])) {
    end--;
  }

  // return it
  return string(str, end - str);
}


string firstAlphanumToken(string_view origStr)
{
  char const* str = origStr.begin();
  char const* strEnd = origStr.end();

  // find the first alpha-numeric; NOTE: if we hit the NUL at the end,
  // that should not be alpha-numeric and we should stop
  while(str != strEnd && !isalnum(*str)) {
    str++;
  }

  // keep going until we are not at an alpha-numeric
  char const* end = str;
  while(end != strEnd && isalnum(*end)) {
    end++;
  }

  // return it
  return string(str, end - str);
}


// table of escape codes
static struct Escape {
  char actual;      // actual character in string
  char escape;      // char that follows backslash to produce 'actual'
} const escapes[] = {
  { '\0', '0' },  // nul
  { '\a', 'a' },  // bell
  { '\b', 'b' },  // backspace
  { '\f', 'f' },  // form feed
  { '\n', 'n' },  // newline
  { '\r', 'r' },  // carriage return
  { '\t', 't' },  // tab
  { '\v', 'v' },  // vertical tab
  { '\\', '\\'},  // backslash
  { '"',  '"' },  // double-quote
  { '\'', '\''},  // single-quote
};


string encodeWithEscapes(string_view src)
{
  string sb;

  for (char ch : src) {
    // look for an escape code
    unsigned i;
    for (i=0; i<TABLESIZE(escapes); i++) {
      if (escapes[i].actual == ch) {
        sb << '\\' << escapes[i].escape;
        break;
      }
    }
    if (i<TABLESIZE(escapes)) {
      continue;   // found it and printed it
    }

    // try itself
    if (isprint(ch)) {
      sb << ch;
      continue;
    }

    // use the most general notation
    char tmp[5];
    sprintf(tmp, "\\x%02X", (unsigned char)(ch));
    sb << tmp;
  }

  return sb;
}


string quoted(string_view src)
{
  string ret = encodeWithEscapes(src);
  ret.insert(0, 1, '"');
  ret.push_back('"');
  return ret;
}


string decodeEscapes(string_view origSrc, char delim, bool allowNewlines)
{
  string dest;
  char const *src = origSrc.begin();
  char const *end = origSrc.end();

  while (src != end && *src != '\0') {
    if (*src == '\n' && !allowNewlines) {
      xformat("unescaped newline (unterminated string)");
    }
    if (*src == delim) {
      xformat("unescaped delimiter ({})", delim);
    }

    if (*src != '\\') {
      // easy case
      dest.push_back(*src);
      src++;
      continue;
    }

    // advance past backslash
    if (++src == end) {
      xformat("backslash at end of string");
    }

    // see if it's a simple one-char backslash code;
    // start at 1 so we do *not* use the '\0' code since
    // that's actually a special case of \0123', and
    // interferes with the latter
    int i;
    for (i=1; i<TABLESIZE(escapes); i++) {
      if (escapes[i].escape == *src) {
        dest.push_back(escapes[i].actual);
        src++;
        break;
      }
    }
    if (i < TABLESIZE(escapes)) {
      continue;
    }


    if (*src == '\n') {
      // escaped newline; advance to first non-whitespace
      src++;
      while (src != end && (*src==' ' || *src=='\t')) {
        src++;
      }
      continue;
    }

    if (*src == 'x' || isdigit(*src)) {
      // hexadecimal or octal char (it's unclear to me exactly how to
      // parse these since it's supposedly legal to have e.g. "\x1234"
      // mean a one-char string.. whatever)
      bool hex = (*src == 'x');
      if (hex) {
        src++;
        if (src == end) {
          xformat("end of string while following hex (\\x) escape");
        }
        // strtoul is willing to skip leading whitespace, so I need
        // to catch it myself
        if (isspace(*src)) {
          xformat("whitespace following hex (\\x) escape");
        }
      }

      char buf[16] = {}; // zero-initialized
      string_view(src, end - src).copy(buf, sizeof(buf)-1);

      char const *endptr;
      unsigned long val = strtoul(buf, (char**)&endptr, hex? 16 : 8);
      if (buf == endptr) {
        // this can't happen with the octal escapes because
        // there is always at least one valid digit
        xformat("invalid hex (\\x) escape");
      }

      dest.push_back((char)(unsigned char)val);    // possible truncation..
      src += endptr - buf;
      continue;
    }

    // everything not explicitly covered will be considered
    // an error (for now), even though the C++ spec says
    // it should be treated as if the backslash were not there
    //
    // 7/29/04: now making backslash the identity transformation in
    // this case
    //
    // copy character as if it had not been backslashed
    dest.push_back(*src);
    src++;
  }
  return dest;
}


string parseQuotedString(string_view text)
{
  if (!text.starts_with('"') || !text.ends_with('"')) {
    xformat("quoted string is missing quotes: {}", text);
  }

  // strip the quotes
  text.remove_prefix(1);
  text.remove_suffix(1);

  // decode escapes
  return decodeEscapes(text, '"');
}


string localTimeString()
{
  time_t t = time(NULL);
  const tm* t2 = localtime(&t);
  if (!t2)
    return string();

  char const *p = asctime(t2);
  if (!p)
    return string();

  string_view view = p;
  view.remove_suffix(1); // strip final newline
  return view.to_string();
}


string sm_basename(string_view src)
{
  while (src.ends_with('/')) {
    // there is a slash, but it is the last character; ignore it
    // (this behavior is what /bin/basename does)
    src.remove_suffix(1);
  }

  size_t sl = src.find_last_of('/');
  if (sl != string_view::npos) {     // everything after the slash
    return src.substr(sl + 1).to_string();
  }
  else {
    return src.to_string();      // entire string if no slashes
  }
}

string dirname(string_view src)
{
  size_t sl = src.find_last_of('/');    // locate last slash
  if (sl == 0) {
    // last slash is the leading slash
    return string("/");
  }

  if (sl != string_view::npos && sl == src.size()-1) {
    // there is a slash, but it is the last character; ignore it
    // (this behavior is what /bin/dirname does)
    return dirname(src.substr(0, sl));
    // FIXME: convert recursion to iteration
  }

  if (sl != string_view::npos) {
    return src.substr(0, sl).to_string(); // everything before slash
  }
  else {
    return string(".");
  }
}


// I will expand this definition to use more knowledge about English
// irregularities as I need it
string plural(int n, string_view prefix)
{
  if (n==1) {
    return prefix.to_string();
  }
  if (prefix == "was") {
    return string("were");
  }
  if (prefix.ends_with('y')) {
    prefix.remove_suffix(1);
    return fmt::format("{}ies", prefix);
  }
  else {
    return fmt::format("{}s", prefix);
  }
}

string pluraln(int n, string_view prefix)
{
  return fmt::format("{} {}", n, plural(n, prefix));
}


string a_or_an(string_view noun)
{
  bool use_an = !noun.empty() && strchr("aeiouAEIOU", noun[0]);

  // special case: I pronounce "mvisitor" like "em-visitor"
  if (noun.starts_with("mv")) {
    use_an = true;
  }

  if (use_an) {
    return fmt::format("an {}", noun);
  }
  else {
    return fmt::format("a {}", noun);
  }
}


char *copyToStaticBuffer(string_view s)
{
  enum { SZ=200 };
  static char buf[SZ+1];

  memset(buf, 0xFF, SZ);
  buf[SZ] = 0;

  size_t len = s.copy(buf, SZ);
  buf[len] = 0;

  return buf;
}


bool prefixEquals(string_view str, string_view prefix)
{
  return str.starts_with(prefix);
}


void writeStringToFile(string_view str, rostring fname)
{
  AutoFILE fp(fname, "w");

  if (fwrite(str.data(), 1, str.size(), fp) != str.size()) {
    xbase("fwrite: short write");
  }
}


string readStringFromFile(rostring fname)
{
  constexpr size_t CHUNK = 16384;
  AutoFILE fp(fname, "r");
  string ret;

  for (;;) {
    const size_t prevLen = ret.length();
    ret.resize(prevLen + CHUNK);
    size_t len = fread(&ret[prevLen], 1, CHUNK, fp);
    if (len < 0) {
      xbase("fread failed");
    }
    if (len != CHUNK) {
      ret.resize(prevLen + len);
      if (len == 0) {
        break;
      }
    }
  }

  return ret;
}


// ----------------------- test code -----------------------------
#ifdef TEST_STRUTIL

#include "test.h"      // USUAL_MAIN
#include <stdio.h>     // printf

void replace(char const* in, char const* from, char const*to, char const* out)
{
  string result = replace(in, from, to);
  printf("replace('%s', '%s', '%s') = '%s'\n", in, from, to, result.c_str());
  xassert(result == out);
}

void trimWhitespace(char const* in, char const* out)
{
  printf("trimWhitespace('%s', '%s')\n", in, out);
  string result = trimWhitespace(in);
  xassert(result == out);
}

void expRangeVector(char const *in, char const *out)
{
  printf("expRangeVector(%s, %s)\n", in, out);
  string result = expandRanges(in);
  xassert(result == out);
}

void trVector(char const *in, char const *srcSpec, char const *destSpec, char const *out)
{
  printf("trVector(%s, %s, %s, %s)\n", in, srcSpec, destSpec, out);
  string result = translate(in, srcSpec, destSpec);
  xassert(result == out);
}

void decodeVector(char const *in, char const *out, int outLen)
{
  printf("decodeVector: \"%s\"\n", in);
  string dest = decodeEscapes(in, '\0' /*delim, ignored*/, false /*allowNewlines*/);
  xassert(dest == string_view(out, outLen));
}

void basenameVector(char const *in, char const *out)
{
  printf("basenameVector(%s, %s)\n", in, out);
  string result = sm_basename(in);
  xassert(result == out);
}

void dirnameVector(char const *in, char const *out)
{
  printf("dirnameVector(%s, %s)\n", in, out);
  string result = dirname(in);
  xassert(result == out);
}

void pluralVector(int n, char const *in, char const *out)
{
  printf("pluralVector(%d, %s, %s)\n", n, in, out);
  string result = plural(n, in);
  xassert(result == out);
}


void entry()
{
  replace("", "", "", "");
  replace("abc", "", "", "abc");
  replace("abc", "a", "", "bc");
  replace("abc", "b", "", "ac");
  replace("abc", "c", "", "ab");
  replace("abc", "ab", "", "c");
  replace("abc", "bc", "", "a");
  replace("abc", "abc", "", "");
  replace("abc", "abc", "def", "def");
  replace("abc", "a", "def", "defbc");
  replace("abc", "b", "def", "adefc");
  replace("abc", "c", "def", "abdef");
  replace("abc", "ab", "def", "defc");
  replace("abc", "bc", "def", "adef");
  replace("foofoo", "foo", "bar", "barbar");
  replace("afoobfooc", "foo", "bar", "abarbbarc");

  trimWhitespace("", "");
  trimWhitespace("a", "a");
  trimWhitespace("abcd", "abcd");
  trimWhitespace(" ", "");
  trimWhitespace("  ", "");
  trimWhitespace(" a", "a");
  trimWhitespace(" abcd", "abcd");
  trimWhitespace("  a", "a");
  trimWhitespace("  abcd", "abcd");
  trimWhitespace("a ", "a");
  trimWhitespace("abcd ", "abcd");
  trimWhitespace("a  ", "a");
  trimWhitespace("abcd  ", "abcd");
  trimWhitespace(" a ", "a");
  trimWhitespace(" abcd ", "abcd");
  trimWhitespace("  a ", "a");
  trimWhitespace("  abcd ", "abcd");
  trimWhitespace(" a  ", "a");
  trimWhitespace(" abcd  ", "abcd");

  expRangeVector("abcd", "abcd");
  expRangeVector("a", "a");
  expRangeVector("a-k", "abcdefghijk");
  expRangeVector("0-9E-Qz", "0123456789EFGHIJKLMNOPQz");

  trVector("foo", "a-z", "A-Z", "FOO");
  trVector("foo BaR", "a-z", "A-Z", "FOO BAR");
  trVector("foo BaR", "m-z", "M-Z", "fOO BaR");

  decodeVector("\\r\\n", "\r\n", 2);
  decodeVector("abc\\0def", "abc\0def", 7);
  decodeVector("\\033", "\033", 1);
  decodeVector("\\x33", "\x33", 1);
  decodeVector("\\?", "?", 1);

  basenameVector("a/b/c", "c");
  basenameVector("abc", "abc");
  basenameVector("/", "");
  basenameVector("a/b/c/", "c");

  dirnameVector("a/b/c", "a/b");
  dirnameVector("a/b/c/", "a/b");
  dirnameVector("/a", "/");
  dirnameVector("abc", ".");
  dirnameVector("/", "/");

  pluralVector(1, "path", "path");
  pluralVector(2, "path", "paths");
  pluralVector(1, "fly", "fly");
  pluralVector(2, "fly", "flies");
  pluralVector(2, "was", "were");
}


USUAL_MAIN

#endif // TEST_STRUTIL
