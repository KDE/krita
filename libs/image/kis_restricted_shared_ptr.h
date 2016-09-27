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

#ifndef KIS_RESTRICTED_SHARED_PTR
#define KIS_RESTRICTED_SHARED_PTR

#include "kis_shared_ptr.h"
#include "kis_pinned_shared_ptr.h"

/**
 * A special type of KisSharedPtr that forbids creation of a shared
 * pointer from raw pointer. This is needed to avoid cases when we
 * pass 'this' into a shared pointer and end up in potentially
 * dangerous state. See KisUniformPaintOpProperty for an example.
 */
template <typename T>
class KisRestrictedSharedPtr : public KisSharedPtr<T>
{
    typedef KisSharedPtr<T> BaseClass;
public:
    KisRestrictedSharedPtr()
    {
    }

    template <typename X>
    inline KisRestrictedSharedPtr(const KisWeakSharedPtr<X>& other)
        : BaseClass(other)
    {
    }


    template <typename X>
    inline KisRestrictedSharedPtr(const KisSharedPtr<X> &other)
        : BaseClass(other)
    {
    }

    template <typename X>
    inline KisRestrictedSharedPtr(const KisRestrictedSharedPtr<X> &other)
        : BaseClass(other)
    {
    }

    template <typename X>
    inline KisRestrictedSharedPtr(const KisPinnedSharedPtr<X> &other)
        : BaseClass(other)
    {
    }


private:
    template <typename X>
    KisRestrictedSharedPtr(X other);
};

#endif // KIS_RESTRICTED_SHARED_PTR

