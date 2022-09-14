/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISZUG_H
#define KISZUG_H

#include "KisMpl.h"
#include <zug/transducer/map.hpp>
#include <zug/reducing/last.hpp>
#include <lager/lenses.hpp>

/**
 * kiszug is a namespace extending the functionality of
 * zug library. It contains transducers, lenses and other
 * tools that are not present in zug itself.
 *
 * Naming convention exception:
 *
 * The namespace follows naming convention of zug library,
 * that is, all entities should be named in "snake_case"
 * manner.
 */

namespace kiszug {

template <typename T>
constexpr auto map_static_cast = zug::map([](auto&& x) { return static_cast<T>(x); });

template <typename T>
constexpr auto map_mupliply = [] (T coeff) { return zug::map([coeff](auto&& x) { return x * coeff; }); };

template <typename T>
constexpr auto map_equal = [] (T value) { return zug::map([value](auto&& x) { return x == value; }); };

template <typename T>
constexpr auto map_greater = [] (T value) { return zug::map([value](auto&& x) { return x > value; }); };
template <typename T>
constexpr auto map_greater_equal = [] (T value) { return zug::map([value](auto&& x) { return x >= value; }); };

template <typename T>
constexpr auto map_less = [] (T value) { return zug::map([value](auto&& x) { return x < value; }); };

template <typename T>
constexpr auto map_less_equal = [] (T value) { return zug::map([value](auto&& x) { return x <= value; }); };

template <>
constexpr auto map_equal<qreal> =  [] (qreal value) { return zug::map([value](auto&& x) { return qFuzzyCompare(x, value); }); };

template <>
constexpr auto map_greater_equal<qreal> = [] (qreal value) { return zug::map([value](auto&& x) { return x >= value || qFuzzyCompare(x, value); }); };

template <>
constexpr auto map_less_equal<qreal> = [] (qreal value) { return zug::map([value](auto&& x) { return x <= value || qFuzzyCompare(x, value); }); };

constexpr auto map_round = zug::map([](qreal x) -> int { return qRound(x); });

struct empty_t
{
};

constexpr auto to_functor = [] (auto f) {
    return
        [=] (auto &&x) {
            return f(zug::last)(empty_t{}, ZUG_FWD(x));
        };
};

constexpr auto foreach_tuple =
    [] (auto mapping) {
        return zug::map([=] (auto &&t) {
            return kismpl::apply_to_tuple(to_functor(mapping), ZUG_FWD(t));
        });
    };

constexpr auto foreach_arg =
    [] (auto mapping) {
        return zug::map([=] (auto&&... t) {
            return std::make_tuple(zug::compat::invoke(to_functor(mapping), ZUG_FWD(t))...);
        });
    };

namespace lenses {
template <typename T>
auto scale = [] (T multiplier) {
    return lager::lenses::getset(
        [multiplier] (T value) { return value * multiplier; },
        [multiplier] (T, T value) { return value / multiplier; }
    );
};

template <typename Src, typename Dst>
auto do_static_cast = lager::lenses::getset(
        [] (Src value) { return static_cast<Dst>(value); },
        [] (Src, Dst value) { return static_cast<Src>(value); }
    );
}


}

#endif // KISZUG_H
