// astlist.h            see license.txt for copyright and terms of use
// owner list wrapper around std::vector
// name 'AST' is because the first application is in ASTs

#ifndef ASTLIST_H
#define ASTLIST_H

#include <vector>         // std::vector

// a list which owns the items in it (will deallocate them), and
// has constant-time access to all elements
template <class T>
class ASTList {
protected:
  std::vector<T*> list;                 // list itself

  ASTList(ASTList const &) = delete;    // not allowed

public:
  using iterator = typename std::vector<T*>::iterator;

  ASTList() = default;
  ~ASTList()                            { deleteAll(); }

  // ctor to make singleton list; often quite useful
  explicit ASTList(T *elt)              { list.push_back(elt); }

  // stealing ctor; among other things, since &src->list is assumed to
  // point at 'src', this class can't have virtual functions;
  // these ctors delete 'src'
  explicit ASTList(ASTList<T> *src)     { if (src) list = std::move(src->list); delete src; }
  void steal(ASTList<T> *src)           { deleteAll(); if (src) list = std::move(src->list); delete src; }

  // iterators
  auto begin() const { return list.begin(); }
  auto end() const { return list.end(); }
  auto begin() { return list.begin(); }
  auto end() { return list.end(); }

  // selectors
  int size() const                      { return list.size(); }
  bool empty() const                    { return list.empty(); }
  T const *front() const                { return list.front(); }
  T const *second() const               { return *std::next(list.begin()); }

  // insertion
  void prepend(T *newitem)              { list.insert(list.begin(), newitem); }
  void append(T *newitem)               { list.push_back(newitem); }

  void insert(iterator it, T* newitem)  { list.insert(it, newitem); }
  void concat(ASTList<T>& tail);

  // removal
  void removeItem(T *item)              { std::remove(list.begin(), list.end(), item); }
  void clear()                          { list.clear(); }
                                        // remove all items, no deallocation
                                        // in the SM API, this would be called removeAll
  // deletion
  void deleteAll();
};


template <class T>
void ASTList<T>::concat(ASTList<T>& tail)
{
  list.reserve(list.size() + tail.list.size());
  std::copy(tail.list.begin(), tail.list.end(), std::back_inserter(list));
  tail.clear();
}


template <class T>
void ASTList<T>::deleteAll()
{
  for (T* item : list) {
    delete item;
  }
  list.clear();
}


#define FOREACH_ASTLIST(T, list, iter) \
  for(T const *iter : list)

#define FOREACH_ASTLIST_NC(T, list, iter) \
  for(T *iter : list)


#endif // ASTLIST_H
