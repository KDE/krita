/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISLAGER_H
#define KISLAGER_H

#include <QtGlobal>
#include <QVariant>
#include <type_traits>
#include "KisMpl.h"

#include <lager/lenses.hpp>
#include <lager/reader.hpp>


namespace kislager {

/**
 * Fold all valid optional cursors from a set into one by using \p func
 *
 * Example:
 *
 * std::optional<lager::reader<int>> brushSize1 = ...;
 * std::optional<lager::reader<int>> brushSize2 = ...;
 *
 * std::optional<lager::reader<int>> maxSize =
 *     fold_optional_cursors(&std::max, brushSize1, brushSize2);
 *
 * The resulting reader 'maxSize' will hold the maximum value of the two
 * brushes, or nothing if the source readers do not exist.
 */
template <typename Func, typename... Cursors,
         typename FirstCursor = typename kismpl::first_type_t<std::remove_reference_t<Cursors>...>::value_type,
         typename T = typename FirstCursor::value_type>
std::optional<lager::reader<T>> fold_optional_cursors(const Func &func, Cursors&& ...cursors) {
    auto fold_func = [func] (const auto &lhs, const auto &rhs) {
        return lager::with(lhs, rhs).map(func);
    };

    return kismpl::fold_optional(fold_func, cursors...);
}

namespace lenses {
template <typename T>
auto scale = [] (T multiplier) {
    return lager::lenses::getset(
        [multiplier] (T value) { return value * multiplier; },
        [multiplier] (T, T value) { return value / multiplier; }
        );
};

constexpr auto scale_int_to_real = [] (qreal multiplier) {
    return lager::lenses::getset(
        [multiplier] (int value) { return value * multiplier; },
        [multiplier] (int, qreal value) { return qRound(value / multiplier); }
        );
};

constexpr auto scale_real_to_int = [] (qreal multiplier) {
    return lager::lenses::getset(
        [multiplier] (qreal value) { return qRound(value * multiplier); },
        [multiplier] (qreal, int value) { return value / multiplier; }
        );
};

template <typename Src, typename Dst, typename SrcConstRef = std::add_lvalue_reference_t<std::add_const_t<Src>>>
auto do_static_cast = lager::lenses::getset(
    [] (SrcConstRef value) { return static_cast<Dst>(value); },
    [] (Src, Dst value) { return static_cast<Src>(value); }
    );

/**
 * A lens that accesses a base class \p Base of the derived
 * type \p Derived
 *
 * to_base2 variant accepts two types, Base and Derived,
 * which might be convenient for debugging.
 */
template <typename Derived, typename Base,
         typename = std::enable_if_t<
             std::is_base_of_v<Base, Derived>>>
auto to_base2 = lager::lenses::getset(
    [] (const Derived &value) -> Base { return static_cast<const Base&>(value); },
    [] (Derived src, const Base &value) { static_cast<Base&>(src) = value; return src; }
    );

/**
 * A lens that accesses a base class \p Base of the derived
 * type \p Derived
 *
 * to_base variant accepts only one type \p Base, that is,
 * destination type into which we should convert the value
 * to
 */
template <typename Base>
auto to_base = lager::lenses::getset(
    [] (const auto &value) -> Base { return static_cast<const Base&>(value); },
    [] (auto src, const Base &value) { static_cast<Base&>(src) = value; return src; }
    );

template <typename T>
auto variant_to = lager::lenses::getset(
    [] (const QVariant &src) {
        return src.value<T>();
    },
    [] (QVariant src, const T &value) {
        src = QVariant::fromValue<T>(value);
        return src;
    }
);

constexpr auto logical_not = [] {
    return lager::lenses::getset(
        [](bool value) -> bool {
            return !value;
        },
        [](bool, bool value) -> bool {
            return !value;
        });
};

} // namespace lenses

} // namespace kislager


#endif // KISLAGER_H
