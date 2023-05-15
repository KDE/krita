/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISLAZYVALUEWRAPPER_H
#define KISLAZYVALUEWRAPPER_H

#include <functional>

template <typename T>
struct KisLazyValueWrapper
{
    using value_type = T;

    KisLazyValueWrapper() = default;

    explicit KisLazyValueWrapper(std::function<value_type()> func) {
        value = func();
    }

    ~KisLazyValueWrapper() = default;

    operator const value_type&() const {
        return value;
    }

    KisLazyValueWrapper(const KisLazyValueWrapper&rhs) = delete;
    KisLazyValueWrapper& operator=(const KisLazyValueWrapper&rhs) = delete;

    KisLazyValueWrapper(KisLazyValueWrapper&&rhs) = default;
    KisLazyValueWrapper& operator=(KisLazyValueWrapper&&rhs) = default;

    value_type value {};
};

#endif // KISLAZYVALUEWRAPPER_H
