// stack.h            see license.txt for copyright and terms of use
//
// A stack adapter for a container.
// It provides all the features of std::stack, and a few more:
// - iterator for iteration from stack top to bottom
// - a pusher iterator
// - defaults to std::vector container for scalar types, std::deque otherwise
// - less redundant template arguments, viz:
//   std::stack<DataType, ContainerType<DataType>>
//   sm::stack<DataType, ContainerType>

#ifndef ELK_STACK_H
#define ELK_STACK_H

#include <deque>
#include <vector>
#include <type_traits>

namespace sm {

  template <class T, class... U>
  using stack_default_container = std::conditional_t<std::is_scalar_v<T>, std::vector<T, U...>, std::deque<T, U...>>;

  template <class T, template <class...> class TContainer = stack_default_container >
  class stack {
  protected:
    using Container = typename TContainer<T>;
    Container c{};

  public:
    using value_type = typename Container::value_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using size_type = typename Container::size_type;
    using iterator = typename Container::reverse_iterator;
    using const_iterator = typename Container::const_reverse_iterator;

    inline Container& _c() { return c; }

    stack() = default;
    stack(const stack &other) = default;
    stack(stack &&other) = default;

    explicit stack(const Container &c) : c(c) {}
    explicit stack(Container&& c) : c(std::move(c)) {}

    bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }

    reference top() { return c.back(); }
    const_reference top() const { return c.back(); }

    void push(const value_type &val) { c.push_back(val); }
    void push(value_type&& val) { c.push_back(std::move(val)); }

    template <class...Args>
    void emplace(Args&&...args) { c.emplace_back(std::forward<Args>(args)...); }

    void pop() { c.pop_back(); }
    void pop_n(size_t count) { c.resize(c.size() - count); }

    void clear() { c.clear(); }

    /* Forth-like operations on the stack */

    // B A -- A B
    void Swap()
    {
      using std::swap;
      auto level1 = begin();
      auto level2 = std::next(level1);  // [2] [1]
      swap(*level1, *level2);           // -- [1] [2]
    }

    // [B] [A] -- [BA]
    void Concat()
    {
      auto level1 = begin();
      auto level2 = std::next(level1);                                               // [2] [1]
      std::copy(level1->rbegin(), level1->rend(), std::back_inserter(level2->_c())); // -- [21] [1]
      pop();                                                                         // -- [21]
    }

    void swap(stack &other) {
      using std::swap;
      swap(c, other.c);
    }

    friend bool operator==(const stack &a, const stack &b)
      { return a.c == b.c; }

    friend bool operator!=(const stack &a, const stack &b)
      { return a.c != b.c; }

    friend bool operator<(const stack &a, const stack &b)
      { return a.c < b.c; }

    friend bool operator>(const stack &a, const stack &b)
      { return a.c > b.c; }

    friend bool operator<=(const stack &a, const stack &b)
      { return a.c <= b.c; }

    friend bool operator>=(const stack &a, const stack &b)
      { return a.c >= b.c; }

    auto cbegin() const { return c.rbegin(); }
    auto cend() const { return c.rend(); }
    auto begin() const { return c.rbegin(); }
    auto end() const { return c.rend(); }
    auto begin() { return c.rbegin(); }
    auto end() { return c.rend(); }

    auto crbegin() const { return c.begin(); }
    auto crend() const { return c.end(); }
    auto rbegin() const { return c.begin(); }
    auto rend() const { return c.end(); }
    auto rbegin() { return c.begin(); }
    auto rend() { return c.end(); }

    class push_iterator {
      stack *s;
    public:
      using iterator_category = std::output_iterator_tag;
      using value_type = void;
      using difference_type = std::ptrdiff_t;
      using pointer = void;
      using reference = void;
      using container_type = stack;

      explicit push_iterator(stack& s) : s(&s) {}

      push_iterator& operator=(typename stack::const_reference v) { s->push(v); return *this; }
      push_iterator& operator=(typename stack::value_type&& v) { s->push(std::move(v)); return *this; }
      push_iterator& operator*() { return *this; }
      push_iterator& operator++() { return *this; }
      push_iterator operator++(int) { return *this; }
    };
  };

  template <typename Container>
  void swap(stack<Container> &a, stack<Container> &b)
    { a.swap(b); }

  template <typename Container>
  auto pusher(Container& c) { return typename Container::push_iterator(c); }

  namespace detail {
    struct _compound {};
    static_assert(std::is_same_v < stack_default_container<void*>, std::vector<void*> >, "");
    static_assert(std::is_same_v < stack_default_container<int>, std::vector<int> >, "");
    static_assert(std::is_same_v < stack_default_container<_compound*>, std::vector<_compound*> >, "");
    static_assert(std::is_same_v < stack_default_container<_compound>, std::deque<_compound> >, "");
  } // namespace sm::detail

}; // namespace sm

#endif // ELK_STACK_H
