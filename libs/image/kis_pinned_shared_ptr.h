/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PINNED_SHARED_PTR_H
#define KIS_PINNED_SHARED_PTR_H

/**
 * A special type of KisSharedPtr that doesn't support conversion
 * into raw pointer. You cannot convert it into raw pointer and cannot
 * accidentally delete it. It is done with a hiding KisSharedPtr's
 * conversion routine and substituting is with a custom one.
 */
template <typename T>
class KisPinnedSharedPtr : public KisSharedPtr<T>
{
    typedef KisSharedPtr<T> BaseClass;
    class NotConvertibleToT {~NotConvertibleToT() = delete;};
    typedef NotConvertibleToT* RestrictedBool;
public:
    KisPinnedSharedPtr()
    {
    }

    inline KisPinnedSharedPtr(T *other)
        : BaseClass(other)
    {
    }

    template <typename X>
    inline KisPinnedSharedPtr(const KisWeakSharedPtr<X>& other)
        : BaseClass(other)
    {
    }


    template <typename X>
    inline KisPinnedSharedPtr(const KisSharedPtr<X> &other)
        : BaseClass(other)
    {
    }


    inline operator RestrictedBool() const
    {
        return this->isNull() ? 0 : reinterpret_cast<NotConvertibleToT*>(1);
    }

    bool operator!() const
    {
        return this->isNull();
    }
private:
    explicit operator const T*() const;
};

#include <kis_debug.h>

template <typename T>
inline QDebug operator<<(QDebug dbg, const KisPinnedSharedPtr<T> &ptr)
{
    dbg.nospace() << ptr.data();
    return dbg;
}

#endif // KIS_PINNED_SHARED_PTR_H

