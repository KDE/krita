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

template <typename T>
[[maybe_unused]]
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
