/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_IMAGE_LOCK_HIJACKER_H
#define __KIS_IMAGE_LOCK_HIJACKER_H

#include <QApplication>
#include <QThread>


#include "kis_image.h"
#include "kis_debug.h"

/**
 * This class removes all the locks from the image on construction and
 * resets them back on destruction. Never use this class unless you
 * really know what you are doing!
 *
 * Never!
 */

class KisImageLockHijacker
{
public:
    KisImageLockHijacker(KisImageSP image)
        : m_image(image),
          m_count(0)
    {
        QThread *currentThread = QThread::currentThread();
        KIS_ASSERT_RECOVER_RETURN(image->thread() == currentThread);
        KIS_ASSERT_RECOVER_RETURN(qApp->thread() == currentThread);

        while(image->locked()) {
            m_count++;
            m_image->unlock();
        }
    }

    ~KisImageLockHijacker() {
        while (m_count--) {
            m_image->barrierLock();
        }
    }

private:
    KisImageSP m_image;
    int m_count;
};

#endif /* __KIS_IMAGE_LOCK_HIJACKER_H */
