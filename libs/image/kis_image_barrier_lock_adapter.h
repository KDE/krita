/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_IMAGE_BARRIER_LOCK_ADAPTER_H
#define __KIS_IMAGE_BARRIER_LOCK_ADAPTER_H


template <typename ImagePointer>
class KisImageBarrierLockAdapterImpl {
public:
    inline KisImageBarrierLockAdapterImpl(ImagePointer image, bool readOnly = false)
        : m_image(image),
          m_readOnly(readOnly)
    {
    }

    inline ~KisImageBarrierLockAdapterImpl() {
    }

    inline void lock() {
        m_image->barrierLock(m_readOnly);
    }

    // Qt syntax
    inline bool tryLock() {
        return m_image->tryBarrierLock(m_readOnly);
    }

    // Boost.Thread syntax
    inline bool try_lock() {
        return tryLock();
    }

    inline void unlock() {
        m_image->unlock();
    }

private:
    KisImageBarrierLockAdapterImpl(const KisImageBarrierLockAdapterImpl<ImagePointer> &rhs);
    ImagePointer m_image;
    bool m_readOnly;
};

typedef KisImageBarrierLockAdapterImpl<KisImageSP> KisImageBarrierLockAdapter;
typedef KisImageBarrierLockAdapterImpl<KisImage*> KisImageBarrierLockAdapterRaw;

#endif /* __KIS_IMAGE_BARRIER_LOCK_ADAPTER_H */
