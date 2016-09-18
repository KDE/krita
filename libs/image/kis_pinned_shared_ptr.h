/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

