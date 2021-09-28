/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisOpenGLBufferCreationGuard.h"

#include "kis_opengl.h"


KisOpenGLBufferCreationGuard::KisOpenGLBufferCreationGuard(QOpenGLBuffer *buffer, int size, QOpenGLBuffer::UsagePattern usagePattern)
{
    m_buffer = buffer;

    m_buffer->create();
    m_buffer->setUsagePattern(usagePattern);
    m_buffer->bind();
    m_buffer->allocate(size);

    if (KisOpenGL::supportsBufferMapping()) {
        m_bufferPtr = reinterpret_cast<quint8*>(m_buffer->map(QOpenGLBuffer::WriteOnly));
        m_bufferIsMapped = true;
    }

    if (!m_bufferPtr) {
        m_bufferPtr = new quint8[size];
    }
}

KisOpenGLBufferCreationGuard::~KisOpenGLBufferCreationGuard()
{
    if (m_bufferIsMapped) {
        m_buffer->unmap();
    } else {
        m_buffer->write(0, m_bufferPtr, m_buffer->size());
        delete[] m_bufferPtr;
    }
    m_buffer->release();
}
