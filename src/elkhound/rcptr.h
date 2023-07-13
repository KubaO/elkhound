// rcptr.h            see license.txt for copyright and terms of use
// a stab at a reference-counting pointer

// the object pointed-at must support this interface:
//   // increment reference count
//   void incRefCt() noexcept;
//
//   // decrement refcount, and if it becomes 0, delete yourself
//   void decRefCt();
//
//   // return the reference count
//   int getRefCt() const noexcept;

#ifndef __RCPTR_H
#define __RCPTR_H

#include <utility>    // std::swap
#include "xassert.h"

#if 0
#include <stdio.h>    // printf
#define RCPTR_DBG(msg)        do { if (ptr) printf("%6s (%p)\n", msg, ptr); else printf("%6s ()\n", msg); } while(false)
#define RCPTR_DBG2(msg, ptr2) printf("%6s (%p,%p)\n", msg, ptr, ptr2)
#else
#define RCPTR_DBG(msg)
#define RCPTR_DBG2(msg, ptr2)
#endif

struct RCPtrAcquire {};
static constexpr RCPtrAcquire RCPTR_ACQUIRE = {};

// The semantics are similar to those of std::shared_ptr.
template <class T>
class RCPtr {
private:
  T* ptr = nullptr;

private:
  void inc() noexcept { RCPTR_DBG("inc"); if (ptr) { ptr->incRefCt(); } }
  void dec()
  {
    RCPTR_DBG("dec");
    if (ptr) { ptr->decRefCt(); ptr = nullptr; }
    xassert(!ptr);
  }

public:
  RCPtr(std::nullptr_t p = nullptr) noexcept { RCPTR_DBG("cnull"); }

  explicit RCPtr(T* p, RCPtrAcquire) noexcept : ptr(p) { RCPTR_DBG("acq"); inc(); }

  // The only valid use is with a fresh object that must have ref counter == 1, or a null.
  explicit RCPtr(T* p) noexcept : ptr(p) { RCPTR_DBG("ctor"); xassert(!p || p->getRefCt() == 1); }

  RCPtr(RCPtr const& other) noexcept : ptr(other.ptr) { RCPTR_DBG("cctor"); inc(); }

  RCPtr(RCPtr&& other) noexcept
  {
    using std::swap;
    swap(ptr, other.ptr);
    RCPTR_DBG("&&ctor");
  }
  ~RCPtr() { RCPTR_DBG("dtor"); dec(); }

  void reset(T* other)
  {
    RCPTR_DBG2("reset", other);
    if (ptr != other) { // otherwise, we'd potentially release the object
      dec(); ptr = other; inc();
    }
  }

  void operator= (RCPtr<T> const& other)
  {
    RCPTR_DBG2("=obj", other.ptr);
    if (ptr != other.ptr) { // otherwise, we'd potentially release the object
      dec(); ptr = other.ptr; inc();
    }
  }

  void operator= (RCPtr<T>&& other)
  {
    RCPTR_DBG2("=&&obj", other.ptr);
    if (ptr != other.ptr) { // otherwise, we'd potentially release the object
      using std::swap;
      dec(); swap(ptr, other.ptr);
    }
  }

  explicit operator bool() const noexcept { return ptr != nullptr; }

  bool operator==(const T* p) const noexcept { return ptr == p; }
  bool operator!=(const T* p) const noexcept { return ptr != p; }

  bool operator==(RCPtr<T> const& obj) const noexcept { return ptr == obj.ptr; }
  bool operator!=(RCPtr<T> const& obj) const noexcept { return ptr != obj.ptr; }

  T& operator* () const noexcept { RCPTR_DBG("op*"); return *ptr; }
  T* operator-> () const noexcept { RCPTR_DBG("op->"); return ptr; }

  T* get() const noexcept { RCPTR_DBG("get"); return ptr; }
  T const* getC() const noexcept { RCPTR_DBG("getC"); return ptr; }
};

#undef RCPTR_DBG
#undef RCPTR_DBG2


#endif // __RCPTR_H
