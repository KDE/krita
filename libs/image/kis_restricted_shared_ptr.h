/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

