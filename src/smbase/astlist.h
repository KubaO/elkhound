// astlist.h            see license.txt for copyright and terms of use
// thin owner list invariant checker around std::vector,
// with a stealing constructor used by astgen-generated code
// name 'AST' is because the first application is in ASTs

#ifndef ASTLIST_H
#define ASTLIST_H

#include <vector>         // std::vector
#include "xassert.h"      // xassert

// a list which is supposed to own the items in it -
// it must never be destroyed while not-empty;
// user code must ensure that owned items are deleted
// or moved elsewhere before destruction;
// this is so that potentially we can use a straight
// std::vector later
template <class T>
class ASTList : public std::vector<T*>
{
public:
  using std::vector<T*>::vector;

  ASTList(ASTList &&) = default;
  ASTList &operator=(ASTList &&) = default;

  ASTList(ASTList const&) = delete;
  ASTList& operator=(ASTList const&) = delete;

  ASTList() = default;
  ~ASTList()                            { xassert(empty()); } // invariant

  // stealing ctor
  explicit ASTList(ASTList<T> *src);
};


// this stealing constructor is used by code generated
// by astgen; it is the consequence of storing these
// list by pointer, thus "construction from a pointer"
// means "stealing";
// TODO the lists should be held by value and be
// move-constructed, avoiding the indirection and
// the need for this constructor
template <class T>
ASTList<T>::ASTList(ASTList<T>* src)
{
  if (src) {
    *this = std::move(*src);
    delete src;
  }
}


// disposes the owned contents of the 'to' list,
// then moves the other list's contents in, and
// deletes that list
template <class T>
void astSteal(ASTList<T>& to, ASTList<T>* from)
{
  deleteAll(to);
  if (from) {
    to = std::move(*from);
    delete from;
  }
}


// transfers ownership of items from tail to head,
// by appending them; tail is emptied
template <class T>
void astConcat(ASTList<T>& head, ASTList<T>& tail)
{
  head.reserve(head.size() + tail.size());
  std::copy(tail.begin(), tail.end(), std::back_inserter(head));
  tail.clear();
}


// disposes of all owned elements; ends up with
// empty list
template <class T>
void deleteAll(ASTList<T>& list)
{
  for (T* item : list) {
    delete item;
  }
  list.clear();
}

// these macros are used all over the place and can
// be left in place for now to avoid too diff noise;
// these will have to be removed eventually

#define FOREACH_ASTLIST(T, list, iter) \
  for(T const *iter : list)

#define FOREACH_ASTLIST_NC(T, list, iter) \
  for(T *iter : list)


#endif // ASTLIST_H
