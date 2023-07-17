// bflatten.h            see license.txt for copyright and terms of use
// binary file flatten implementation

#ifndef BFLATTEN_H
#define BFLATTEN_H

#include "flatten.h"      // Flatten

#include <unordered_map>  // std::unordered_map
#include <stdio.h>        // FILE

class BFlatten : public Flatten {
private:     // data
  FILE *fp;               // file being read/written
  bool readMode;          // true=read, false=write

  struct OwnerMapping {
    void *ownerPtr;       // a pointer
    intptr_t intName;     // a unique integer name
  };
  std::unordered_map<void const *, OwnerMapping*> ownerTable;      // owner <-> int mapping
  int nextUniqueName;     // counter for making int names

public:      // funcs
  BFlatten(char const *fname, bool reading);
  virtual ~BFlatten();

  // Flatten funcs
  virtual bool reading() const { return readMode; }
  virtual void xferSimple(void *var, unsigned len);
  virtual void noteOwner(void *ownerPtr);
  virtual void xferSerf(void *&serfPtr, bool nullable=false);
};


// for debugging, write and then read something
template <class T>
T *writeThenRead(T &obj)
{
  char const *fname = "flattest.tmp";

  // write
  {
    BFlatten out(fname, false /*reading*/);
    obj.xfer(out);
  }

  // read
  BFlatten in(fname, true /*reading*/);
  T *ret = new T(in);
  ret->xfer(in);

  remove(fname);

  return ret;
}

#endif // BFLATTEN_H
