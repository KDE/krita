/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
