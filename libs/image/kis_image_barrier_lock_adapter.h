/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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
        m_image->unlock();
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
