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

} // namespace kismpl

#endif // KISMPL_H
