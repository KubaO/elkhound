// hashline.h
// module for maintaining and using #line info in source files

// terminology:
//   pp source: preprocessed source, i.e. whatever had the #line
//              info sprinkled throughout it
//   orig source: original source, the files to which the #line
//                directives refer

#ifndef HASHLINE_H
#define HASHLINE_H

#include <set>              // std::set<string>
#include <vector>           // std::vector
#include "str.h"            // string, rostring

// map from lines in some given pp source file to lines in
// orig source files; there should be one HashLineMap object
// for each pp source file of interest
class HashLineMap {
private:    // types
  // records a single #line directive
  class HashLine {
  public:
    int ppLine;              // pp source line where it appears
    int origLine;            // orig line it names
    char const *origFname;   // orig fname it names

  public:
    HashLine()
      : ppLine(0), origLine(0), origFname(NULL) {}
    HashLine(int pl, int ol, char const *of)
      : ppLine(pl), origLine(ol), origFname(of) {}
  };

private:    // data
  // name of the pp file; this is needed for queries to lines
  // before any #line is encountered
  string ppFname;

  // set for canonical storage of orig filenames
  std::set<string> filenames;

  // growable array of HashLine objects
  std::vector<HashLine> directives;

  // previously-added ppLine; used to verify the entries are
  // being added in sorted order
  int prev_ppLine;

public:     // funcs
  HashLineMap(rostring ppFname);
  ~HashLineMap();

  // call this time each time a #line directive is encountered;
  // successive calls must have strictly increasing values of 'ppLine'
  void addHashLine(int ppLine, int origLine, char const *origFname);

  // call this when all the #line directives have been added; this
  // consolidates the 'directives' array to reclaim any space created
  // during the growing process but that is now not needed
  void doneAdding();

  // map from pp line to orig line/file; note that queries exactly on
  // #line lines have undefined results
  void map(int ppLine, int &origLine, char const *&origFname) const;
  int mapLine(int ppLine) const;           // returns 'origLine'
  char const *mapFile(int ppLine) const;   // returns 'origFname'

  // for curiosity, find out how many unique filenames are recorded in
  // the 'filenames' dictionary
  int numUniqueFilenames() { return filenames.size(); }

  // similarly, the number of directives
  int numDirectives() const { return directives.size(); }
};

#endif // HASHLINE_H
