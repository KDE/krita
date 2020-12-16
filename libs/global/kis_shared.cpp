/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_shared.h"
#include "kis_debug.h"


/**
 * NOTE: The description of how Weak shared pointers system works:
 *
 * Every KisShared object has his own _sharedWeakReference pointer.
 * This pointer holds QAtomicInt counter. When KisShared constructs
 * itself, it increments _sharedWeakReference by one. When it dies -
 * it decrements it. This is the only way how the number in the
 * counter can become odd. Obviously, when the number falls to zero,
 * the counter object is deleted.
 *
 * When a weak shared pointer is created, it gets the pointer and
 * increments the counter by 2, so the parity of the number is kept
 * unchanged. Now the counter is shared between the KisShared and
 * KisWeakSharedPtr and the latter one can check the correctness
 * of the pointer by checking parity!
 */

KisShared::KisShared()
{
    /**
     * We can defer creation of a weak reference until
     * better days... It gives us 41% better performance. ;)
     */
    _sharedWeakReference = 0;
}

KisShared::~KisShared()
{
    /**
     * Check no-one references us
     */
    Q_ASSERT(_ref == 0);

    if(_sharedWeakReference && !_sharedWeakReference->deref())
        delete _sharedWeakReference;
}
