/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCPPQUIRKS_H
#define KISCPPQUIRKS_H

#include <type_traits>
#include <optional>
#include <version>

namespace std {

// from C++14

// MSVC STL is C++14 by default

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
// https://github.com/microsoft/STL/issues/1925

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

/**
 * copy_const returns type Dst with exactly the same const-qualifier
 * as type Src. In other words, it copies "constness" property from
 * type Src to Dst.
 */
template <typename Src, typename Dst>
struct copy_const {
    using type = add_const_if_t<std::is_const_v<Src>, std::remove_const_t<Dst>>;
};

template <typename Src, typename Dst>
using copy_const_t = typename copy_const<Src, Dst>::type;

} // namespace std

#if __cplusplus >= 201603L                                                     \
    || defined(__has_cpp_attribute) && __has_cpp_attribute(maybe_unused)
#define MAYBE_UNUSED [[maybe_unused]]
#elif defined(__GNUC__) || defined(__clang__)
#define MAYBE_UNUSED __attribute__((unused))
#else
#define MAYBE_UNUSED
#endif

template <typename T>
MAYBE_UNUSED
QDebug operator<<(QDebug dbg, const std::optional<T> &t)
{

    if (t) {
        dbg.nospace() << "std::optional(" << *t << ")";
    } else {
        dbg.nospace() << "std::optional(nullopt)";
    }

    return dbg.space();
}


#endif // KISCPPQUIRKS_H
