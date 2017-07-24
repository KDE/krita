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

#ifndef __KIS_IMAGE_BARRIER_LOCKER_H
#define __KIS_IMAGE_BARRIER_LOCKER_H

template <typename ImagePointer>
struct PointerPolicyAlwaysPresent
{
    typedef ImagePointer ImagePointerType;
    static inline void barrierLock(ImagePointer image) {
        image->barrierLock();
    }

    static inline void unlock(ImagePointer image) {
        image->unlock();
    }
};

template <typename ImagePointer>
struct PointerPolicyAllowNull
{
    typedef ImagePointer ImagePointerType;

    static inline void barrierLock(ImagePointer image) {
        if (image) {
            image->barrierLock();
        }
    }

    static inline void unlock(ImagePointer image) {
        if (image) {
            image->unlock();
        }
    }
};

/**
 * A simple class that implements std::lock_guard concept for locking KisImage.
 * It barrier-locks the image during construction and unlocks it on destruction.
 *
 * \p PointerPolicy defines how the image pointer is handled
 */

template <class PointerPolicy>
class KisImageBarrierLockerImpl {
public:
    typedef typename PointerPolicy::ImagePointerType ImagePointer;
public:
    inline KisImageBarrierLockerImpl(ImagePointer image)
        : m_image(image)
    {
        PointerPolicy::barrierLock(m_image);
    }

    inline ~KisImageBarrierLockerImpl() {
        PointerPolicy::unlock(m_image);
    }

private:
    KisImageBarrierLockerImpl(const KisImageBarrierLockerImpl<PointerPolicy> &rhs);
    ImagePointer m_image;
};

// Lock guard for the image passed as KisImageSP pointer. Pointer cannot be null.
typedef KisImageBarrierLockerImpl<PointerPolicyAlwaysPresent<KisImageSP>> KisImageBarrierLocker;

// Lock guard for the image passed as a raw KisImage* pointer. Pointer cannot be null.
typedef KisImageBarrierLockerImpl<PointerPolicyAlwaysPresent<KisImage*>> KisImageBarrierLockerRaw;

// Lock guard for the image passed as KisImageSP pointer that *can* be null.
typedef KisImageBarrierLockerImpl<PointerPolicyAllowNull<KisImageSP>> KisImageBarrierLockerAllowNull;

#endif /* __KIS_IMAGE_BARRIER_LOCKER_H */
