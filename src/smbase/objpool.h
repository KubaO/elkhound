// objpool.h            see license.txt for copyright and terms of use
// custom allocator: array of objects meant to be
// re-used frequently, with high locality

#ifndef OBJPOOL_H
#define OBJPOOL_H

#include "xassert.h"  // xassert

#include <vector>     // std::vector

// the class T should have:
//
//   // object is done being used for now - a cheaper destructor
//   void deinit();
//
//   // needed so we can make arrays
//   T::T();

template <class T>
class ObjectPool {
private:     // types
  struct Block {
    T value;
    Block* nextInFreeList = nullptr;
  };

private:     // data
  // when the pool needs to expand, it expands by allocating an
  // additional 'rackSize' objects; I use a linear (instead of
  // exponential) expansion strategy because these are supposed
  // to be used for small sets of rapidly-reused objects, not
  // things allocated for long-term storage
  int const rackSize;

  // growable array of pointers to arrays of 'rackSize' T objects
  std::vector<Block *> racks;

  // head of the free list; NULL when empty
  Block *head = nullptr;

  // whether we're destroying the pool
  bool inDestructor = false;

private:     // funcs
  void expandPool();

public:      // funcs
  explicit ObjectPool(int rackSize);
  ~ObjectPool();

  // yields a pointer to an object ready to be used; typically,
  // T should have some kind of init method to play the role a
  // constructor ordinarily does; this might cause the pool to
  // expand (but previously allocated objects do *not* move)
  inline T *alloc();

  // return an object to the pool of objects; dealloc internally
  // calls obj->deinit()
  inline void dealloc(T *obj);

  // available for diagnostic purposes
  int freeObjectsInPool() const;
};


template <class T>
ObjectPool<T>::ObjectPool(int rs)
  : rackSize(rs)
{
  racks.reserve(5);
}

template <class T>
ObjectPool<T>::~ObjectPool()
{
  // if the objects held links to other objects in the pool,
  // we don't want the deallocations to have any effect anymore
  inDestructor = true;

  // deallocate all the racks
  for (Block *rack : racks) {
    delete[] rack;
  }
}


template <class T>
inline T *ObjectPool<T>::alloc()
{
  xassert(!inDestructor);

  if (!head) {
    // need to expand the pool
    expandPool();
  }

  Block *ret = head;               // prepare to return this one
  head = head->nextInFreeList;     // move to next free node

#ifndef NDEBUG
  ret->nextInFreeList = NULL;      // helps with debugging
#endif

  return &ret->value;
}


// this is pulled out of 'alloc' so alloc can be inlined
// without causing excessive object code bloat
template <class T>
void ObjectPool<T>::expandPool()
{
  xassert(!inDestructor);

  Block *rack = new Block[rackSize];
  racks.push_back(rack);

  // thread new nodes into a free list
  for (int i=rackSize-1; i>=0; i--) {
    rack[i].nextInFreeList = head;
    head = &(rack[i]);
  }
}


template <class T>
inline void ObjectPool<T>::dealloc(T *obj)
{
  if (!inDestructor) {
    // call obj's pseudo-dtor (the decision to have dealloc do this is
    // motivated by not wanting to have to remember to call deinit
    // before dealloc)
    obj->deinit();

    // I don't check that nextInFreeList == NULL, despite having set it
    // that way in alloc(), because I want to allow for users to make
    // nextInFreeList share storage (e.g. with a union) with some other
    // field that gets used while the node is allocated

    // prepend the object to the free list; will be next yielded
    Block *blk = reinterpret_cast<Block *>(obj);
    blk->nextInFreeList = head;
    head = blk;
  }
}


template <class T>
int ObjectPool<T>::freeObjectsInPool() const
{
  Block *p = head;
  int ct = 0;

  while (p) {
    ct++;
    p = p->nextInFreeList;
  }

  return ct;
}


#endif // OBJPOOL_H
