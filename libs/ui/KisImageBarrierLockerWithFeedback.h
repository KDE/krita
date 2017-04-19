/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISIMAGEBARRIERLOCKERWITHFEEDBACK_H
#define KISIMAGEBARRIERLOCKERWITHFEEDBACK_H

#include "kis_types.h"
#include "kis_image_barrier_locker.h"
#include "kritaui_export.h"

namespace KisImageBarrierLockerWithFeedbackImplPrivate {
void KRITAUI_EXPORT blockWithFeedback(KisImageSP image);
}

/**
 * The wrapper around KisImageBarrierLocker or KisImageBarrierLockerAllowNull
 * that adds GUI feedback with a progress bar when the locking is going to be
 * long enough.
 */
template<class InternalLocker>
class KisImageBarrierLockerWithFeedbackImpl
{
public:
    KisImageBarrierLockerWithFeedbackImpl(KisImageSP image) {
        KisImageBarrierLockerWithFeedbackImplPrivate::blockWithFeedback(image);
        m_locker.reset(new InternalLocker(image));
    }

    ~KisImageBarrierLockerWithFeedbackImpl() {
    }

private:
    QScopedPointer<InternalLocker> m_locker;
};

/**
 * @brief KisImageBarrierLockerWithFeedback is a simple KisImageBarrierLocker with a
 * progress dialog feedback shown before locking.
 */
typedef KisImageBarrierLockerWithFeedbackImpl<KisImageBarrierLocker> KisImageBarrierLockerWithFeedback;

/**
 * @brief KisImageBarrierLockerWithFeedback is a simple KisImageBarrierLockerAllowEmpty with a
 * progress dialog feedback shown before locking.
 */
typedef KisImageBarrierLockerWithFeedbackImpl<KisImageBarrierLockerAllowNull> KisImageBarrierLockerWithFeedbackAllowNull;


#endif // KISIMAGEBARRIERLOCKERWITHFEEDBACK_H
