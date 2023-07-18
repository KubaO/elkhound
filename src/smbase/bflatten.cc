// bflatten.cc            see license.txt for copyright and terms of use
// code for bflatten.h

#include "bflatten.h"     // this module
#include "algo.h"         // sm::getPointerFromMap
#include "exc.h"          // throw_XOpen
#include "syserr.h"       // xsyserror


BFlatten::BFlatten(char const *fname, bool r)
  : readMode(r),
    nextUniqueName(1)
{
  fp = fopen(fname, readMode? "rb" : "wb");
  if (!fp) {
    throw_XOpen(fname);
  }
}

BFlatten::~BFlatten()
{
  fclose(fp);
}


void BFlatten::xferSimple(void *var, unsigned len)
{
  if (writing()) {
    if (fwrite(var, 1, len, fp) < len) {
      xsyserror("fwrite");
    }
  }
  else {
    if (fread(var, 1, len, fp) < len) {
      xsyserror("fread");
    }
  }
}


void BFlatten::noteOwner(void *ownerPtr)
{
  // make a new mapping
  OwnerMapping *map = new OwnerMapping;
  map->ownerPtr = ownerPtr;
  map->intName = nextUniqueName++;

  // add it to the table
  if (writing()) {
    // index by pointer
    ownerTable.insert(std::make_pair(ownerPtr, map));
  }
  else {
    // index by int name
    ownerTable.insert(std::make_pair((void const*)(map->intName), map));
  }
}


void BFlatten::xferSerf(void *&serfPtr, bool nullable)
{
  if (writing()) {
    xassert(nullable || serfPtr!=NULL);

    if (serfPtr == NULL) {
      // encode as 0; the names start with 1
      writeInt(0);
    }
    else {
      // lookup the mapping
      OwnerMapping* map = sm::getPointerFromMap(ownerTable, serfPtr);

      // we must have already written the owner pointer
      xassert(map != NULL);

      // write the int name
      writeInt(map->intName);
    }
  }
  else /*reading*/ {
    // read the int name
    intptr_t name = readInt();

    if (name == 0) {      // null
      xassert(nullable);
      serfPtr = NULL;
    }
    else {
      // lookup the mapping
      OwnerMapping* map = sm::getPointerFromMap(ownerTable, (void const*)name);
      formatAssert(map != NULL);

      // return the pointer
      serfPtr = map->ownerPtr;
    }
  }
}


// ------------------------ test code ---------------------
#ifdef TEST_BFLATTEN

#include "test.h"      // USUAL_MAIN

void entry()
{
  // make up some data
  int x = 9, y = 22;
  string s("foo bar");
  int *px = &x, *py = &y;

  // open a file for writing them
  {
    BFlatten flat("bflat.tmp", false /*reading*/);
    flat.xferInt(x);
    flat.noteOwner(&x);
    flat.xferString(s);
    flat.xferSerf((void*&)px);
    flat.xferInt(y);
    flat.noteOwner(&y);
    flat.xferSerf((void*&)py);
  }

  // place to put the data we read
  int x2, y2;
  string s2;
  int *px2, *py2;

  // read them back
  {
    BFlatten flat("bflat.tmp", true /*reading*/);
    flat.xferInt(x2);
    flat.noteOwner(&x2);
    flat.xferString(s2);
    flat.xferSerf((void*&)px2);
    flat.xferInt(y2);
    flat.noteOwner(&y2);
    flat.xferSerf((void*&)py2);
  }

  // compare
  xassert(x == x2);
  xassert(y == y2);
  xassert(s == s2);
  xassert(px2 == &x2);
  xassert(py2 == &y2);

  // delete the temp file
  remove("bflat.tmp");

  printf("bflatten works\n");
}


USUAL_MAIN


#endif // TEST_BFLATTEN
