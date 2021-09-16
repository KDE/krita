/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISOPENGLBUFFERCREATIONGUARD_H
#define KISOPENGLBUFFERCREATIONGUARD_H

#include <QOpenGLBuffer>

struct KisOpenGLBufferCreationGuard {
    KisOpenGLBufferCreationGuard(QOpenGLBuffer *buffer, int size, QOpenGLBuffer::UsagePattern usagePattern);
    ~KisOpenGLBufferCreationGuard();

    inline quint8* data() {
        return m_bufferPtr;
    }

private:
    QOpenGLBuffer *m_buffer;
    quint8 *m_bufferPtr = 0;
    bool m_bufferIsMapped = false;

};

#endif // KISOPENGLBUFFERCREATIONGUARD_H
