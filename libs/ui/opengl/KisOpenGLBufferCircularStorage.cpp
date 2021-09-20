/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisOpenGLBufferCircularStorage.h"

#include "kis_assert.h"
#include "kis_opengl.h"


KisOpenGLBufferCircularStorage::BufferBinder::BufferBinder(KisOpenGLBufferCircularStorage *bufferStorage, const void **dataPtr, int dataSize) {
    if (bufferStorage) {
        m_buffer = bufferStorage->getNextBuffer();
        m_buffer->bind();
        m_buffer->write(0, *dataPtr, dataSize);
        *dataPtr = 0;
    }

}

KisOpenGLBufferCircularStorage::BufferBinder::~BufferBinder() {
    if (m_buffer) {
        m_buffer->release();

        if (KisOpenGL::useTextureBufferInvalidation()) {
            KisOpenGL::glInvalidateBufferData(m_buffer->bufferId());
        }
    }
}


struct KisOpenGLBufferCircularStorage::Private
{
    int nextBuffer = 0;
    int bufferSize = 0;
    QOpenGLBuffer::Type type = QOpenGLBuffer::QOpenGLBuffer::VertexBuffer;
    std::vector<QOpenGLBuffer> buffers;
};


KisOpenGLBufferCircularStorage::KisOpenGLBufferCircularStorage()
    : KisOpenGLBufferCircularStorage(QOpenGLBuffer::VertexBuffer)
{
}

KisOpenGLBufferCircularStorage::KisOpenGLBufferCircularStorage(QOpenGLBuffer::Type type)
    : m_d(new Private)
{
    m_d->type = type;
}

KisOpenGLBufferCircularStorage::~KisOpenGLBufferCircularStorage()
{
}

void KisOpenGLBufferCircularStorage::allocate(int numBuffers, int bufferSize)
{
    m_d->buffers.clear();
    addBuffersImpl(numBuffers, bufferSize);
}

QOpenGLBuffer *KisOpenGLBufferCircularStorage::getNextBuffer()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(isValid(), 0);

    QOpenGLBuffer *buffer = &m_d->buffers[m_d->nextBuffer];
    m_d->nextBuffer = (m_d->nextBuffer + 1) % m_d->buffers.size();
    return buffer;
}

bool KisOpenGLBufferCircularStorage::isValid() const
{
    return !m_d->buffers.empty();
}

int KisOpenGLBufferCircularStorage::size() const
{
    return m_d->buffers.size();
}

void KisOpenGLBufferCircularStorage::reset()
{
    m_d->buffers.clear();
}

void KisOpenGLBufferCircularStorage::allocateMoreBuffers(int numBuffers)
{
    if (numBuffers <= m_d->buffers.size()) return;
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_d->buffers.empty());

    std::rotate(m_d->buffers.begin(), m_d->buffers.begin() + m_d->nextBuffer, m_d->buffers.end());

    m_d->nextBuffer = m_d->buffers.size();
    addBuffersImpl(numBuffers - m_d->buffers.size(), m_d->bufferSize);
}

void KisOpenGLBufferCircularStorage::addBuffersImpl(int buffersToAdd, int bufferSize)
{
    m_d->bufferSize = bufferSize;
    m_d->buffers.reserve(m_d->buffers.size() + buffersToAdd);

    for (int i = 0; i < buffersToAdd; i++) {
        m_d->buffers.emplace_back(m_d->type);

        QOpenGLBuffer &buf = m_d->buffers.back();

        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        buf.bind();
        buf.allocate(bufferSize);
        buf.release();
    }
}
