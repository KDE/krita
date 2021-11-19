/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMPL_H
#define KISMPL_H

#include <tuple>
#include <utility>

/**
 * 'kismpl' stands for kis-meta-program-library
 */
namespace kismpl {
namespace detail {

template <class F, class Tuple, std::size_t... I>
void apply_impl(F f, Tuple&& t, std::index_sequence<I...>)
{
    f(std::get<I>(std::forward<Tuple>(t))...);
}
}  // namespace detail

/**
 * Calls function \p f by passing the content of tuple \t as
 * distinct arguments.
 *
 * This is a simplified version of C++17's std::apply() routine.
 * It supports only standard function (not pointer-to-members)
 * without any return value.
 */
template <class F, class Tuple>
void apply(F&& f, Tuple&& t)
{
    detail::apply_impl(
        std::forward<F>(f), std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
}

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

}

#endif // KISMPL_H
