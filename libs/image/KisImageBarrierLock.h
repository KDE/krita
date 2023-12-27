/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_IMAGE_BARRIER_LOCK_H
#define __KIS_IMAGE_BARRIER_LOCK_H

#include <kis_types.h>
#include <KisAdaptedLock.h>

template <typename ImagePointer, bool readOnly>
class KisImageBarrierLockAdapterImpl {
public:
    inline KisImageBarrierLockAdapterImpl(ImagePointer image)
        : m_image(image)
    {
    }

    inline void lock() {
        m_image->barrierLock(readOnly);
    }

    inline bool try_lock() {
        return m_image->tryBarrierLock(readOnly);
    }

    inline void unlock() {
        m_image->unlock();
    }

private:
    ImagePointer m_image;
};


using KisImageBarrierLockAdapter = KisImageBarrierLockAdapterImpl<KisImageSP, false>;
using KisImageReadOnlyBarrierLockAdapter = KisImageBarrierLockAdapterImpl<KisImageSP, true>;

using KisImageBarrierLockAdapterRaw = KisImageBarrierLockAdapterImpl<KisImage*, false>;
using KisImageReadOnlyBarrierLockAdapterRaw = KisImageBarrierLockAdapterImpl<KisImage*, true>;

KIS_DECLARE_ADAPTED_LOCK(KisImageBarrierLock, KisImageBarrierLockAdapter)
KIS_DECLARE_ADAPTED_LOCK(KisImageReadOnlyBarrierLock, KisImageReadOnlyBarrierLockAdapter)

KIS_DECLARE_ADAPTED_LOCK(KisImageBarrierLockRaw, KisImageBarrierLockAdapterRaw)
KIS_DECLARE_ADAPTED_LOCK(KisImageReadOnlyBarrierLockRaw, KisImageReadOnlyBarrierLockAdapterRaw)

#endif /* __KIS_IMAGE_BARRIER_LOCK_H */
