// stack.h            see license.txt for copyright and terms of use
//
// Specialized algorithms we use

#ifndef ELK_SM_ALGO_H
#define ELK_SM_ALGO_H

#include <algorithm>
#include <type_traits>

namespace sm {

  // a less verbose alternative to `std::find` when only a yes/no answer is needed
  template <class T, class Container>
  bool contains(const Container& range, const T& value)
  {
    return std::find(range.begin(), range.end(), value) != range.end();
  }

  // compares two sorted lists using a provided int-returning comparison function, the returned value has the same meaning as in `strcmp` and `memcmp`.
  template <
    class Container,
    class ConstPtr = std::add_pointer_t<
      std::add_const_t< std::remove_pointer_t<Container::value_type> > >
  >
  int compareSortedSLists(const Container& a, const Container& b,
    int(*diff)(ConstPtr, ConstPtr, void*), void* extra = nullptr)
  {
    auto ita = a.begin(), itb = b.begin();
    auto enda = a.end(), endb = b.end();

    while (ita != enda && itb != endb) {
      int cmp = diff(*ita, *itb, extra);

      if (cmp == 0) {
        // they are equal; keep going
      }
      else {
        // unequal, return which way comparison went
        return cmp;
      }
      ita++;
      itb++;
    }

    if (ita != enda) {
      // longer compares as more
      return +1;
    }
    else if (itb != endb) {
      return -1;
    }

    return 0;        // everything matches
  }

  // sorts the given container using a provided int-returning comparison function
  template <
    class Container,
    class ConstPtr = std::add_pointer_t<
      std::add_const_t < std::remove_pointer_t<Container::value_type> > >
  >
  std::enable_if_t<!std::is_const_v<Container>, void>
    sortSList(Container& c, int(*diff)(ConstPtr, ConstPtr, void*), void* extra = nullptr)
  {
    std::sort(c.begin(), c.end(), [=](ConstPtr a, ConstPtr b) { return diff(a, b, extra) < 0; });
  }

  // returns the pointer-typed value for a given key, or nullptr
  template <class Map>
  std::enable_if_t<std::is_pointer_v<typename Map::value_type::second_type>, typename Map::value_type::second_type>
    getPointerFromMap(Map const& map, typename Map::value_type::first_type const &key)
  {
    auto it = map.find(key);
    if (it != map.end()) {
      return it->second;
    }
    else {
      return nullptr;
    }
  }

  // returns val if the set contains it, or nullptr
  template <class Set>
  std::enable_if_t<std::is_pointer_v<typename Set::value_type>, typename Set::value_type>
    getPointerFromSet(Set const& set, typename Set::value_type val)
  {
    auto it = set.find(val);
    if (it != set.end()) {
      return *it;
    }
    else {
      return nullptr;
    }
  }

} // namespace sm

#endif // ELK_SM_ALGO_H
