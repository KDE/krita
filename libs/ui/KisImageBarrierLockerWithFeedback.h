/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
