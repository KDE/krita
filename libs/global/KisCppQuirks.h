/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCPPQUIRKS_H
#define KISCPPQUIRKS_H

#include <type_traits>

namespace std {

// from C++14

#if __cplusplus < 201402L && (!defined(_MSC_VER))

template <typename Cont>
inline auto rbegin(Cont &cont) -> decltype(declval<Cont>().rbegin()) {
    return cont.rbegin();
}

template <typename Cont>
inline auto rend(Cont &cont) -> decltype(declval<Cont>().rend()) {
    return cont.rend();
}

template <class BidirectionalIterator>
inline reverse_iterator<BidirectionalIterator> make_reverse_iterator(BidirectionalIterator x)
{
    return reverse_iterator<BidirectionalIterator>(x);
}

template< bool B, class T = void >
using enable_if_t = typename enable_if<B,T>::type;

template< bool B, class T, class F >
using conditional_t = typename conditional<B,T,F>::type;

template< class T >
using add_const_t    = typename add_const<T>::type;


#endif

// from C++17

// NOTE: MSVC breaks the standard and defines these functions
//       even when C++14 is explicitly selected

#if (__cplusplus < 201703L) && (!defined(_MSC_VER))

template<typename...>
using void_t = void;

template <class T>
constexpr std::add_const_t<T>& as_const(T& t) noexcept
{
    return t;
}

template <class T>
void as_const(const T&&) = delete;

#endif

template <bool is_const, class T>
struct add_const_if
{
    using type = std::conditional_t<is_const, std::add_const_t<T>, T>;
};

template <bool is_const, class T>
using add_const_if_t = typename add_const_if<is_const, T>::type;

}

#endif // KISCPPQUIRKS_H
