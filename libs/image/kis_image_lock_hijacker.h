/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
