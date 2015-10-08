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
class KisImageBarrierLockerImpl {
public:
    inline KisImageBarrierLockerImpl(ImagePointer image)
        : m_image(image)
    {
        m_image->barrierLock();
    }

    inline ~KisImageBarrierLockerImpl() {
        m_image->unlock();
    }

private:
    KisImageBarrierLockerImpl(const KisImageBarrierLockerImpl<ImagePointer> &rhs);
    ImagePointer m_image;
};

typedef KisImageBarrierLockerImpl<KisImageSP> KisImageBarrierLocker;
typedef KisImageBarrierLockerImpl<KisImage*> KisImageBarrierLockerRaw;

#endif /* __KIS_IMAGE_BARRIER_LOCKER_H */
