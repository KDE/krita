/*

Copyright (c) 2014-2018 Jonathan B. Coe

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef JBCOE_PROPAGATE_CONST_INCLUDED
#define JBCOE_PROPAGATE_CONST_INCLUDED

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#ifndef _MSC_VER
#define PROPAGATE_CONST_CONSTEXPR constexpr
#else
#if _MSC_VER <= 1900 // MSVS 2015 and earlier
#define PROPAGATE_CONST_CONSTEXPR
#define PROPAGATE_CONST_HAS_NO_EXPRESSION_SFINAE
#else
#define PROPAGATE_CONST_CONSTEXPR constexpr
#endif
#endif

namespace std {
namespace experimental {
inline namespace fundamentals_v2 {
template <class T>
class propagate_const {
 public:
 using element_type = typename std::pointer_traits<T>::element_type;

 private:
  template <class U>
  static element_type* get_pointer(U* u) {
    return u;
  }

  template <class U>
  static element_type* get_pointer(U& u) {
    return get_pointer(u.get());
  }

  template <class U>
  static const element_type* get_pointer(const U* u) {
    return u;
  }

  template <class U>
  static const element_type* get_pointer(const U& u) {
    return get_pointer(u.get());
  }

  template <class U>
  struct is_propagate_const : false_type {};

  template <class U>
  struct is_propagate_const<propagate_const<U>> : true_type {};

 public:
  // [propagate_const.ctor], constructors
  PROPAGATE_CONST_CONSTEXPR propagate_const() = default;

  propagate_const(const propagate_const& p) = delete;

  PROPAGATE_CONST_CONSTEXPR propagate_const(propagate_const&& p) = default;

#ifdef PROPAGATE_CONST_HAS_NO_EXPRESSION_SFINAE
  //
  // Make converting constructors explicit as we cannot use SFINAE to check.
  //
  template <class U, class = enable_if_t<is_constructible<T, U&&>::value>>
  explicit PROPAGATE_CONST_CONSTEXPR propagate_const(propagate_const<U>&& pu)
      : t_(std::move(pu.t_))
  {
  }

  template <class U,
            class = enable_if_t<is_constructible<T, U&&>::value &&
                                !is_propagate_const<decay_t<U>>::value>>
  explicit PROPAGATE_CONST_CONSTEXPR propagate_const(U&& u)
      : t_(std::forward<U>(u))
  {
  }
#else
  //
  // Use SFINAE to check if converting constructor should be explicit.
  //
  template <class U, enable_if_t<!is_convertible<U&&, T>::value &&
                                     is_constructible<T, U&&>::value,
                                 bool> = true>
  explicit PROPAGATE_CONST_CONSTEXPR propagate_const(propagate_const<U>&& pu)
      : t_(std::move(pu.t_)) {}

  template <class U, enable_if_t<is_convertible<U&&, T>::value &&
                                     is_constructible<T, U&&>::value,
                                 bool> = false>
  PROPAGATE_CONST_CONSTEXPR propagate_const(propagate_const<U>&& pu) : t_(std::move(pu.t_)) {}

  template <class U, enable_if_t<!is_convertible<U&&, T>::value &&
                                     is_constructible<T, U&&>::value &&
                                     !is_propagate_const<decay_t<U>>::value,
                                 bool> = true>
  explicit PROPAGATE_CONST_CONSTEXPR propagate_const(U&& u) : t_(std::forward<U>(u)) {}

  template <class U, enable_if_t<is_convertible<U&&, T>::value &&
                                     is_constructible<T, U&&>::value &&
                                     !is_propagate_const<decay_t<U>>::value,
                                 bool> = false>
  PROPAGATE_CONST_CONSTEXPR propagate_const(U&& u) : t_(std::forward<U>(u)) {}
#endif

  // [propagate_const.assignment], assignment
  propagate_const& operator=(const propagate_const& p) = delete;

  PROPAGATE_CONST_CONSTEXPR propagate_const& operator=(propagate_const&& p) = default;

  template <class U>
  PROPAGATE_CONST_CONSTEXPR propagate_const& operator=(propagate_const<U>&& pu) {
    t_ = std::move(pu.t_);
    return *this;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  PROPAGATE_CONST_CONSTEXPR propagate_const& operator=(U&& u) {
    t_ = std::move(u);
    return *this;
  }

  // [propagate_const.const_observers], const observers
  explicit PROPAGATE_CONST_CONSTEXPR operator bool() const { return get() != nullptr; }
  PROPAGATE_CONST_CONSTEXPR const element_type* operator->() const { return get(); }

  template <class T_ = T, class U = enable_if_t<is_convertible<
                              const T_, const element_type*>::value>>
  PROPAGATE_CONST_CONSTEXPR operator const element_type*() const  // Not always defined
  {
    return get();
  }

  PROPAGATE_CONST_CONSTEXPR const element_type& operator*() const { return *get(); }

  PROPAGATE_CONST_CONSTEXPR const element_type* get() const { return get_pointer(t_); }

  // [propagate_const.non_const_observers], non-const observers
  PROPAGATE_CONST_CONSTEXPR element_type* operator->() { return get(); }

  template <class T_ = T,
            class U = enable_if_t<is_convertible<T_, element_type*>::value>>
  PROPAGATE_CONST_CONSTEXPR operator element_type*()  // Not always defined
  {
    return get();
  }

  PROPAGATE_CONST_CONSTEXPR element_type& operator*() { return *get(); }

  PROPAGATE_CONST_CONSTEXPR element_type* get() { return get_pointer(t_); }
  
  // [propagate_const.modifiers], modifiers
  PROPAGATE_CONST_CONSTEXPR void swap(propagate_const& pt) noexcept(
      noexcept(swap(declval<T&>(), declval<T&>()))) {
    swap(t_, pt.t_);
  }

 private:
  T t_;

  friend struct std::hash<propagate_const<T>>;
  friend struct std::equal_to<propagate_const<T>>;
  friend struct std::not_equal_to<propagate_const<T>>;
  friend struct std::greater<propagate_const<T>>;
  friend struct std::less<propagate_const<T>>;
  friend struct std::greater_equal<propagate_const<T>>;
  friend struct std::less_equal<propagate_const<T>>;

  // [propagate_const.relational], relational operators
  friend PROPAGATE_CONST_CONSTEXPR bool operator==(const propagate_const& pt, nullptr_t) {
    return pt.t_ == nullptr;
  }

  friend PROPAGATE_CONST_CONSTEXPR bool operator==(nullptr_t, const propagate_const& pu) {
    return nullptr == pu.t_;
  }

  friend PROPAGATE_CONST_CONSTEXPR bool operator!=(const propagate_const& pt, nullptr_t) {
    return pt.t_ != nullptr;
  }

  friend PROPAGATE_CONST_CONSTEXPR bool operator!=(nullptr_t, const propagate_const& pu) {
    return nullptr != pu.t_;
  }

  template <class U>
  friend PROPAGATE_CONST_CONSTEXPR bool operator==(const propagate_const& pt,
                                   const propagate_const<U>& pu) {
    return pt.t_ == pu.t_;
  }

  template <class U>
  friend PROPAGATE_CONST_CONSTEXPR bool operator!=(const propagate_const& pt,
                                   const propagate_const<U>& pu) {
    return pt.t_ != pu.t_;
  }

  template <class U>
  friend PROPAGATE_CONST_CONSTEXPR bool operator<(const propagate_const& pt,
                                  const propagate_const<U>& pu) {
    return pt.t_ < pu.t_;
  }

  template <class U>
  friend PROPAGATE_CONST_CONSTEXPR bool operator>(const propagate_const& pt,
                                  const propagate_const<U>& pu) {
    return pt.t_ > pu.t_;
  }

  template <class U>
  friend PROPAGATE_CONST_CONSTEXPR bool operator<=(const propagate_const& pt,
                                   const propagate_const<U>& pu) {
    return pt.t_ <= pu.t_;
  }

  template <class U>
  friend PROPAGATE_CONST_CONSTEXPR bool operator>=(const propagate_const& pt,
                                   const propagate_const<U>& pu) {
    return pt.t_ >= pu.t_;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator==(const propagate_const& pt, const U& u) {
    return pt.t_ == u;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator!=(const propagate_const& pt, const U& u) {
    return pt.t_ != u;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator<(const propagate_const& pt, const U& u) {
    return pt.t_ < u;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator>(const propagate_const& pt, const U& u) {
    return pt.t_ > u;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator<=(const propagate_const& pt, const U& u) {
    return pt.t_ <= u;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator>=(const propagate_const& pt, const U& u) {
    return pt.t_ >= u;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator==(const U& u, const propagate_const& pu) {
    return u == pu.t_;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator!=(const U& u, const propagate_const& pu) {
    return u != pu.t_;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator<(const U& u, const propagate_const& pu) {
    return u < pu.t_;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator>(const U& u, const propagate_const& pu) {
    return u > pu.t_;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator<=(const U& u, const propagate_const& pu) {
    return u <= pu.t_;
  }

  template <class U,
            class = enable_if_t<!is_propagate_const<decay_t<U>>::value>>
  friend PROPAGATE_CONST_CONSTEXPR bool operator>=(const U& u, const propagate_const& pu) {
    return u >= pu.t_;
  }
};


// [propagate_const.algorithms], specialized algorithms
template <class T>
PROPAGATE_CONST_CONSTEXPR void swap(propagate_const<T>& pt, propagate_const<T>& pu) noexcept(
    noexcept(swap(declval<T&>(), declval<T&>())))
{
  swap(pt.underlying_ptr(), pu.underlying_ptr());
}

}  //  end namespace fundamentals_v2
}  //  end namespace experimental

// [propagate_const.hash], hash support
template <class T>
struct hash<experimental::fundamentals_v2::propagate_const<T>> {
  typedef size_t result_type;
  typedef experimental::fundamentals_v2::propagate_const<T> argument_type;

  bool operator()(
      const experimental::fundamentals_v2::propagate_const<T>& pc) const {
    return std::hash<T>()(pc.t_);
  }
};

// [propagate_const.comparison_function_objects], comparison function objects
template <class T>
struct equal_to<experimental::fundamentals_v2::propagate_const<T>> {
  typedef experimental::fundamentals_v2::propagate_const<T> first_argument_type;
  typedef experimental::fundamentals_v2::propagate_const<T>
      second_argument_type;

  bool operator()(
      const experimental::fundamentals_v2::propagate_const<T>& pc1,
      const experimental::fundamentals_v2::propagate_const<T>& pc2) const {
    return std::equal_to<T>()(pc1.t_, pc2.t_);
  }
};

template <class T>
struct not_equal_to<experimental::fundamentals_v2::propagate_const<T>> {
  typedef experimental::fundamentals_v2::propagate_const<T> first_argument_type;
  typedef experimental::fundamentals_v2::propagate_const<T>
      second_argument_type;

  bool operator()(
      const experimental::fundamentals_v2::propagate_const<T>& pc1,
      const experimental::fundamentals_v2::propagate_const<T>& pc2) const {
    return std::not_equal_to<T>()(pc1.t_, pc2.t_);
  }
};

template <class T>
struct less<experimental::fundamentals_v2::propagate_const<T>> {
  typedef experimental::fundamentals_v2::propagate_const<T> first_argument_type;
  typedef experimental::fundamentals_v2::propagate_const<T>
      second_argument_type;

  bool operator()(
      const experimental::fundamentals_v2::propagate_const<T>& pc1,
      const experimental::fundamentals_v2::propagate_const<T>& pc2) const {
    return std::less<T>()(pc1.t_, pc2.t_);
  }
};

template <class T>
struct greater<experimental::fundamentals_v2::propagate_const<T>> {
  typedef experimental::fundamentals_v2::propagate_const<T> first_argument_type;
  typedef experimental::fundamentals_v2::propagate_const<T>
      second_argument_type;

  bool operator()(
      const experimental::fundamentals_v2::propagate_const<T>& pc1,
      const experimental::fundamentals_v2::propagate_const<T>& pc2) const {
    return std::greater<T>()(pc1.t_, pc2.t_);
  }
};

template <class T>
struct less_equal<experimental::fundamentals_v2::propagate_const<T>> {
  typedef experimental::fundamentals_v2::propagate_const<T> first_argument_type;
  typedef experimental::fundamentals_v2::propagate_const<T>
      second_argument_type;

  bool operator()(
      const experimental::fundamentals_v2::propagate_const<T>& pc1,
      const experimental::fundamentals_v2::propagate_const<T>& pc2) const {
    return std::less_equal<T>()(pc1.t_, pc2.t_);
  }
};

template <class T>
struct greater_equal<experimental::fundamentals_v2::propagate_const<T>> {
  typedef experimental::fundamentals_v2::propagate_const<T> first_argument_type;
  typedef experimental::fundamentals_v2::propagate_const<T>
      second_argument_type;

  bool operator()(
      const experimental::fundamentals_v2::propagate_const<T>& pc1,
      const experimental::fundamentals_v2::propagate_const<T>& pc2) const {
    return std::greater_equal<T>()(pc1.t_, pc2.t_);
  }
};

}  // end namespace std

#undef PROPAGATE_CONST_CONSTEXPR
#endif // JBCOE_PROPAGATE_CONST_INCLUDED
