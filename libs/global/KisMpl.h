/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMPL_H
#define KISMPL_H

#include <tuple>
#include <utility>
#include <optional>
#include <functional>

/**
 * 'kismpl' stands for kis-meta-program-library
 */
namespace kismpl {

namespace detail {

template<std::size_t ...Idx>
struct make_index_sequence_from_1_impl;

template<std::size_t num, std::size_t ...Idx>
struct make_index_sequence_from_1_impl<num, Idx...> {
    using type = typename make_index_sequence_from_1_impl<num - 1, num, Idx...>::type;
};

template<std::size_t ...Idx>
struct make_index_sequence_from_1_impl<0, Idx...> {
    using type = std::index_sequence<Idx...>;
};

}  // namespace detail

/**
 * Creates an index sequence in range 1, 2, 3, ..., Num
 *
 * Same as std::make_index_sequence, but starts counting
 * from 1 instead of 0.
 */
template<std::size_t Num>
using make_index_sequence_from_1 =
    typename detail::make_index_sequence_from_1_impl<Num>::type;

namespace detail {

template <typename F, typename Tuple, std::size_t... I>
auto apply_to_tuple_impl(F f, Tuple &&t, std::index_sequence<I...>) {
    return std::make_tuple(f(std::get<I>(std::forward<Tuple>(t)))...);
}
}

/**
 * Apply a given functor F to each element of the tuple and
 * return a tuple consisting of the resulting values.
 */
template <typename F, typename Tuple>
auto apply_to_tuple(F f, Tuple &&t) {
    return detail::apply_to_tuple_impl(std::forward<F>(f), std::forward<Tuple>(t),
                                       std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
}

/**
 * Convert a given functor f accepting multiple arguments into
 * a function that accepts a tuple with the same number of
 * elements
 */
constexpr auto unzip_wrapper = [] (auto f) {
    return
        [=] (auto &&x) {
            return std::apply(f, std::forward<decltype(x)>(x));
        };
};

namespace detail {

template <typename First, typename... Rest>
struct first_type_impl
{
    using type = First;
};

}

/**
 * Return the first type of a parameter pack
 */
template <typename... T>
struct first_type
{
    using type = typename detail::first_type_impl<T...>::type;
};

/**
 * A helper function to return the first type of a parameter
 * pack
 */
template <typename... T>
using first_type_t = typename first_type<T...>::type;

namespace detail {
template <typename Fun, typename T>
struct fold_optional_impl {
    std::optional<T> fold(const std::optional<T> &first) {
        return first;
    }

    std::optional<T> fold(const std::optional<T> &first, const std::optional<T> &second) {
        if (first && second) {
            return m_fun(*first, *second);
        } else if (first) {
            return first;
        } else {
            return second;
        }
    }

    template <typename... Rest>
    std::optional<T> fold(const std::optional<T> &first, std::optional<T> const &second, const std::optional<Rest> &...rest) {
        return fold(fold(first, second), rest...);
    }

    const Fun m_fun;
};

} // namespace detail

/**
 * Folds all the valid optional values using the binary function \p fun into one
 * optional value. When none optional values are present, an empty optional of the
 * specified type is returned.
 */
template <typename Fun, typename... Args,
         typename T = typename first_type_t<std::remove_reference_t<Args>...>::value_type>
std::optional<T> fold_optional(Fun &&fun, Args &&...args) {
    return detail::fold_optional_impl<Fun, T>{std::forward<Fun>(fun)}.fold(args...);
}

/**
 * A helper class for creation of a visitor for an std::visit
 */
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace detail {

template<typename Op, typename Class, typename MemType, typename PtrType>
struct mem_checker
{
    template <typename Object>
    bool operator() (Object &&obj) const {
        Op op;
        return op(std::invoke(ptr, std::forward<Object>(obj)), value);
    }

    PtrType ptr;
    const MemType value;
};

template<typename Op, typename Class, typename MemType, typename PtrType>
struct mem_compare
{
    template <typename Object>
    bool operator() (Object &&lhs, Object &&rhs) const {
        Op op;
        return op(std::invoke(ptr, std::forward<Object>(lhs)),
                  std::invoke(ptr, std::forward<Object>(rhs)));
    }

    template <typename Object>
    bool operator() (Object &&lhs, const MemType &rhs) const {
        Op op;
        return op(std::invoke(ptr, std::forward<Object>(lhs)),
                  rhs);
    }

    template <typename Object>
    bool operator() (const MemType &lhs, Object &&rhs) const {
        Op op;
        return op(lhs,
                  std::invoke(ptr, std::forward<Object>(rhs)));
    }

    PtrType ptr;
};

} // detail

/**
 * @brief mem_equal_to is an unary functor that compares a member of the object to
 *                     a given value
 *
 * The functor is supposed to be used in `std::find_if` and other standard algorithms.
 * It can automatically dereference a pointer-to-member or a pointer-to-method.
 *
 *  * Usage:
 *
 *        \code{.cpp}
 *
 *        struct Struct {
 *            Struct (int _id) : id(_id) {}
 *
 *            int id = -1;
 *            int idConstFunc() const {
 *                return id;
 *            }
 *        };
 *
 *        std::vector<Struct> vec({{0},{1},{2},{3}});
 *
 *        // find an element, which has member 'id' set to 1
 *        auto it1 = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::id, 1));
 *
 *        // find an element, whose member function 'idConstFunc()' returns 1
 *        auto it2 = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::idConstFunc, 1));
 *
 *        // the functor can automatically dereference pointers and shared pointers
 *        std::vector<std::shared_ptr<Struct>> vec({std::make_shared<Struct>(0),
 *                                                  std::make_shared<Struct>(1),
 *                                                  std::make_shared<Struct>(2),
 *                                                  std::make_shared<Struct>(3),
 *                                                  std::make_shared<Struct>(4)});
 *
 *        // the shared pointer is automatically lifted by the functor
 *        auto it3 = std::find_if(vec.begin(), vec.end(), kismpl::mem_equal_to(&Struct::id, 1));
 *
 *        \endcode
 */

template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_equal_to(MemTypeNoRef Class::*ptr, MemType &&value) {
    return detail::mem_checker<std::equal_to<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// non-const member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_equal_to(MemTypeNoRef (Class::*ptr)(), MemType &&value) {
    return detail::mem_checker<std::equal_to<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// const member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_equal_to(MemTypeNoRef (Class::*ptr)() const, MemType &&value) {
    return detail::mem_checker<std::equal_to<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// non-const noexcept member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_equal_to(MemTypeNoRef (Class::*ptr)() noexcept, MemType &&value) {
    return detail::mem_checker<std::equal_to<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// const noexcept member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_equal_to(MemTypeNoRef (Class::*ptr)() const noexcept, MemType &&value) {
    return detail::mem_checker<std::equal_to<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

/**
 * @brief mem_less is an unary functor that compares a member of the object to
 *                 a given value
 *
 * @see mem_equal_to
 */

template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_less(MemTypeNoRef Class::*ptr, MemType &&value) {
    return detail::mem_checker<std::less<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// non-const member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_less(MemTypeNoRef (Class::*ptr)(), MemType &&value) {
    return detail::mem_checker<std::less<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// const member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_less(MemTypeNoRef (Class::*ptr)() const, MemType &&value) {
    return detail::mem_checker<std::less<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// non-const noexcept member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_less(MemTypeNoRef (Class::*ptr)() noexcept, MemType &&value) {
    return detail::mem_checker<std::less<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// const noexcept member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_less(MemTypeNoRef (Class::*ptr)() const noexcept, MemType &&value) {
    return detail::mem_checker<std::less<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

/**
 * @brief mem_less_equal is an unary functor that compares a member of the object to
 *                       a given value
 *
 * @see mem_equal_to
 */

template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_less_equal(MemTypeNoRef Class::*ptr, MemType &&value) {
    return detail::mem_checker<std::less_equal<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// non-const member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_less_equal(MemTypeNoRef (Class::*ptr)(), MemType &&value) {
    return detail::mem_checker<std::less_equal<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// const member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_less_equal(MemTypeNoRef (Class::*ptr)() const, MemType &&value) {
    return detail::mem_checker<std::less_equal<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// non-const noexcept member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_less_equal(MemTypeNoRef (Class::*ptr)() noexcept, MemType &&value) {
    return detail::mem_checker<std::less_equal<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// const noexcept member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_less_equal(MemTypeNoRef (Class::*ptr)() const noexcept, MemType &&value) {
    return detail::mem_checker<std::less_equal<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

/**
 * @brief mem_greater is an unary functor that compares a member of the object to
 *                    a given value
 *
 * @see mem_equal_to
 */

template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_greater(MemTypeNoRef Class::*ptr, MemType &&value) {
    return detail::mem_checker<std::greater<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// non-const member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_greater(MemTypeNoRef (Class::*ptr)(), MemType &&value) {
    return detail::mem_checker<std::greater<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// const member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_greater(MemTypeNoRef (Class::*ptr)() const, MemType &&value) {
    return detail::mem_checker<std::greater<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// non-const noexcept member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_greater(MemTypeNoRef (Class::*ptr)() noexcept, MemType &&value) {
    return detail::mem_checker<std::greater<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// const noexcept member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_greater(MemTypeNoRef (Class::*ptr)() const noexcept, MemType &&value) {
    return detail::mem_checker<std::greater<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

/**
 * @brief mem_greater_equal is an unary functor that compares a member of the object to
 *                          a given value
 *
 * @see mem_equal_to
 */


template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_greater_equal(MemTypeNoRef Class::*ptr, MemType &&value) {
    return detail::mem_checker<std::greater_equal<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// non-const member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_greater_equal(MemTypeNoRef (Class::*ptr)(), MemType &&value) {
    return detail::mem_checker<std::greater_equal<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// const member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_greater_equal(MemTypeNoRef (Class::*ptr)() const, MemType &&value) {
    return detail::mem_checker<std::greater_equal<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// non-const noexcept member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_greater_equal(MemTypeNoRef (Class::*ptr)() noexcept, MemType &&value) {
    return detail::mem_checker<std::greater_equal<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

// const noexcept member function
template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
inline auto mem_greater_equal(MemTypeNoRef (Class::*ptr)() const noexcept, MemType &&value) {
    return detail::mem_checker<std::greater_equal<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
}

/**
 * @brief mem_less is a binary functor that compares a member of the object to a
 *                 given value or two objects based on the value of their members
 *
 * The functor is supposed to be used in `std::lower_bound` and other standard algorithms.
 * It can automatically dereference a pointer-to-member or a pointer-to-method.
 *
 *  * Usage:
 *
 *        \code{.cpp}
 *
 *        struct Struct {
 *            Struct (int _id) : id(_id) {}
 *
 *            int id = -1;
 *            int idConstFunc() const {
 *                return id;
 *            }
 *        };
 *
 *        std::vector<Struct> vec({{0},{1},{2},{3}});
 *
 *        // find the first element, whose 'id' is not less than 1
 *        auto it1 = std::lower_bound(vec.begin(), vec.end(), 1, kismpl::mem_less(&Struct::id));
 *
 *        // find the first element, whose 'id' returned by 'idConstFunc()' is not less than 1
 *        auto it2 = std::lower_bound(vec.begin(), vec.end(), 1, kismpl::mem_less(&Struct::idConstFunc, 1));
 *
 *        // the functor can automatically dereference pointers and shared pointers
 *        std::vector<std::shared_ptr<Struct>> vec({std::make_shared<Struct>(0),
 *                                                  std::make_shared<Struct>(1),
 *                                                  std::make_shared<Struct>(2),
 *                                                  std::make_shared<Struct>(3),
 *                                                  std::make_shared<Struct>(4)});
 *
 *        // the shared pointer is automatically lifted by the functor
 *        auto it3 = std::lower_bound(vec.begin(), vec.end(), 1, kismpl::mem_less(&Struct::id));
 *
 *        \endcode
 */

template<typename Class, typename MemType>
inline auto mem_less(MemType Class::*ptr) {
    return detail::mem_compare<std::less<>, Class, MemType, decltype(ptr)>{ptr};
}

// non-const member function
template<typename Class, typename MemType>
inline auto mem_less(MemType (Class::*ptr)()) {
    return detail::mem_compare<std::less<>, Class, MemType, decltype(ptr)>{ptr};
}

// const member function
template<typename Class, typename MemType>
inline auto mem_less(MemType (Class::*ptr)() const) {
    return detail::mem_compare<std::less<>, Class, MemType, decltype(ptr)>{ptr};
}

// non-const noexcept member function
template<typename Class, typename MemType>
inline auto mem_less(MemType (Class::*ptr)() noexcept) {
    return detail::mem_compare<std::less<>, Class, MemType, decltype(ptr)>{ptr};
}

// const noexcept member function
template<typename Class, typename MemType>
inline auto mem_less(MemType (Class::*ptr)() const noexcept) {
    return detail::mem_compare<std::less<>, Class, MemType, decltype(ptr)>{ptr};
}

/**
 * @brief mem_less_equal is a binary functor that compares a member of the object to a
 *                       given value or two objects based on the value of their members
 *
 * @see mem_less
 */
template<typename Class, typename MemType>
inline auto mem_less_equal(MemType Class::*ptr) {
    return detail::mem_compare<std::less_equal<>, Class, MemType, decltype(ptr)>{ptr};
}

// non-const member function
template<typename Class, typename MemType>
inline auto mem_less_equal(MemType (Class::*ptr)()) {
    return detail::mem_compare<std::less_equal<>, Class, MemType, decltype(ptr)>{ptr};
}

// const member function
template<typename Class, typename MemType>
inline auto mem_less_equal(MemType (Class::*ptr)() const) {
    return detail::mem_compare<std::less_equal<>, Class, MemType, decltype(ptr)>{ptr};
}

// non-const noexcept member function
template<typename Class, typename MemType>
inline auto mem_less_equal(MemType (Class::*ptr)() noexcept) {
    return detail::mem_compare<std::less_equal<>, Class, MemType, decltype(ptr)>{ptr};
}

// const noexcept member function
template<typename Class, typename MemType>
inline auto mem_less_equal(MemType (Class::*ptr)() const noexcept) {
    return detail::mem_compare<std::less_equal<>, Class, MemType, decltype(ptr)>{ptr};
}

/**
 * @brief mem_greater is a binary functor that compares a member of the object to a
 *                    given value or two objects based on the value of their members
 *
 * @see mem_less
 */

template<typename Class, typename MemType>
inline auto mem_greater(MemType Class::*ptr) {
    return detail::mem_compare<std::greater<>, Class, MemType, decltype(ptr)>{ptr};
}

// non-const member function
template<typename Class, typename MemType>
inline auto mem_greater(MemType (Class::*ptr)()) {
    return detail::mem_compare<std::greater<>, Class, MemType, decltype(ptr)>{ptr};
}

// const member function
template<typename Class, typename MemType>
inline auto mem_greater(MemType (Class::*ptr)() const) {
    return detail::mem_compare<std::greater<>, Class, MemType, decltype(ptr)>{ptr};
}

// non-const noexcept member function
template<typename Class, typename MemType>
inline auto mem_greater(MemType (Class::*ptr)() noexcept) {
    return detail::mem_compare<std::greater<>, Class, MemType, decltype(ptr)>{ptr};
}

// const noexcept member function
template<typename Class, typename MemType>
inline auto mem_greater(MemType (Class::*ptr)() const noexcept) {
    return detail::mem_compare<std::greater<>, Class, MemType, decltype(ptr)>{ptr};
}

/**
 * @brief mem_greater_equal is a binary functor that compares a member of the object to a
 *                          given value or two objects based on the value of their members
 *
 * @see mem_less
 */

template<typename Class, typename MemType>
inline auto mem_greater_equal(MemType Class::*ptr) {
    return detail::mem_compare<std::greater_equal<>, Class, MemType, decltype(ptr)>{ptr};
}

// non-const member function
template<typename Class, typename MemType>
inline auto mem_greater_equal(MemType (Class::*ptr)()) {
    return detail::mem_compare<std::greater_equal<>, Class, MemType, decltype(ptr)>{ptr};
}

// const member function
template<typename Class, typename MemType>
inline auto mem_greater_equal(MemType (Class::*ptr)() const) {
    return detail::mem_compare<std::greater_equal<>, Class, MemType, decltype(ptr)>{ptr};
}

// non-const noexcept member function
template<typename Class, typename MemType>
inline auto mem_greater_equal(MemType (Class::*ptr)() noexcept) {
    return detail::mem_compare<std::greater_equal<>, Class, MemType, decltype(ptr)>{ptr};
}

// const noexcept member function
template<typename Class, typename MemType>
inline auto mem_greater_equal(MemType (Class::*ptr)() const noexcept) {
    return detail::mem_compare<std::greater_equal<>, Class, MemType, decltype(ptr)>{ptr};
}

/**
 * A simple wrapper class that executes a passed lambda
 * on destruction. It might be used for cleaning-up resources,
 * which are not a part of normal RAII relationships.
 */
template <typename F>
struct finally {
    finally(F &&f)
        : m_f(std::forward<F>(f))
    {
    }

    finally(const finally &) = delete;
    finally(finally &&) = delete;

    ~finally() {
        m_f();
    }
private:
    F m_f;
};


} // namespace kismpl

#endif // KISMPL_H
