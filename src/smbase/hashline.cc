// hashline.cc
// code for hashline.h

#include "hashline.h"      // this module
#include "xassert.h"       // xassert

#include <string.h>        // memcpy


HashLineMap::HashLineMap(rostring pf)
  : ppFname(pf),
    prev_ppLine(-1)  // user shouldn't have negative line numbers
{}


HashLineMap::~HashLineMap()
{}


void HashLineMap::addHashLine(int ppLine, int origLine, char const *origFname)
{
  // check that entries are being added in sorted order
  xassert(ppLine > prev_ppLine);
  prev_ppLine = ppLine;

  // map 'origFname' to a canonical reference
  auto canon = filenames.insert(origFname); // duplicates aren't inserted
  origFname = canon.first->c_str();

  // add the entry to the array
  directives.push_back(HashLine(ppLine, origLine, origFname));
}


void HashLineMap::doneAdding()
{
  directives.shrink_to_fit();
}


// for queries exactly on #line directives we return the specified
// origLine minus 1, but I don't specify any behavior in that case
// so it's not a problem
void HashLineMap::map(int ppLine, int &origLine, char const *&origFname) const
{
  // check for a ppLine that precedes any in the array
  if (directives.empty() ||
      ppLine < directives[0].ppLine) {
    // it simply refers to the pp file
    origLine = ppLine;
    origFname = ppFname.c_str();
    return;
  }

  // perform binary search on the 'directives' array
  int low = 0;                        // index of lowest candidate
  int high = directives.size()-1;   // index of highest candidate

  while (low < high) {
    // check the midpoint (round up to ensure progress when low+1 == high)
    int mid = (low+high+1)/2;
    if (directives[mid].ppLine > ppLine) {
      // too high
      high = mid-1;
    }
    else {
      // too low or just right
      low = mid;
    }
  }
  xassert(low == high);
  HashLine const &hl = directives[low];

  // the original line is the origLine in the array entry, plus the
  // offset between the ppLine passed in and the ppLine in the array,
  // minus 1 because the #line directive itself occupies one pp line
  origLine = hl.origLine + (ppLine - hl.ppLine - 1);

  origFname = hl.origFname;
}


int HashLineMap::mapLine(int ppLine) const
{
  int origLine;
  char const *origFname;
  map(ppLine, origLine, origFname);
  return origLine;
}

char const *HashLineMap::mapFile(int ppLine) const
{
  int origLine;
  char const *origFname;
  map(ppLine, origLine, origFname);
  return origFname;
}


// --------------------- test code ---------------------
#ifdef TEST_HASHLINE

#include <stdio.h>     // printf
#include <stdlib.h>    // exit

void query(HashLineMap &hl, int ppLine,
           int expectOrigLine, char const *expectOrigFname)
{
  int origLine;
  char const *origFname;
  hl.map(ppLine, origLine, origFname);

  if (origLine != expectOrigLine ||
      0!=strcmp(origFname, expectOrigFname)) {
    printf("map(%d) yielded %s:%d, but I expected %s:%d\n",
           ppLine, origFname, origLine,
           expectOrigFname, expectOrigLine);
    exit(2);
  }
}


int main()
{
  // insert #line directives:
  //    foo.i
  //    +----------
  //   1|// nothing; it's in the pp file
  //   2|#line 1 foo.cc
  //   3|
  //   4|
  //   5|#line 1 foo.h
  //   ..
  //  76|#line 5 foo.cc
  //   ..
  // 100|#line 101 foo.i

  HashLineMap hl("foo.i");
  hl.addHashLine(2, 1, "foo.cc");
  hl.addHashLine(5, 1, "foo.h");
  hl.addHashLine(76, 5, "foo.cc");
  hl.addHashLine(100, 101, "foo.i");
  hl.doneAdding();
  int expectedFiles = 3;
  int expectedDirectives = 4;

  // make queries, and check for expected results
  query(hl, 1, 1, "foo.i");

  query(hl, 3, 1, "foo.cc");
  query(hl, 4, 2, "foo.cc");

  query(hl, 6, 1, "foo.h");
  query(hl, 7, 2, "foo.h");
  // ...
  query(hl, 75, 70, "foo.h");

  query(hl, 77, 5, "foo.cc");
  query(hl, 78, 6, "foo.cc");
  // ...
  query(hl, 99, 27, "foo.cc");

  query(hl, 101, 101, "foo.i");
  query(hl, 102, 102, "foo.i");
  // ...

  int numFiles = hl.numUniqueFilenames();
  int numDirectives = hl.numDirectives();
  if (numFiles != expectedFiles) {
    printf("the number of unique files was %d, but I expected %d\n",
           numFiles, expectedFiles);
    return 2;
  }
  if (numDirectives != expectedDirectives) {
    printf("the number of directives was %d, but I expected %d\n",
           numDirectives, expectedDirectives);
    return 2;
  }

  printf("hashline seems to work\n");

  return 0;
}

#endif // TEST_HASHLINE
