/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisOpenGLBufferCircularStorage.h"

#include <QtMath>

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
    reset();
    KIS_ASSERT(numBuffers > 0);
    KIS_ASSERT(bufferSize > 0);
    addBuffersImpl(static_cast<size_t>(numBuffers), bufferSize);
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
    return static_cast<int>(m_d->buffers.size());
}

void KisOpenGLBufferCircularStorage::reset()
{
    m_d->buffers.clear();
    m_d->nextBuffer = 0;
    m_d->bufferSize = 0;
}

void KisOpenGLBufferCircularStorage::allocateMoreBuffers()
{
    const size_t numBuffers = nextPowerOfTwo(m_d->buffers.size());

    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_d->buffers.empty());

    std::rotate(m_d->buffers.begin(), m_d->buffers.begin() + m_d->nextBuffer, m_d->buffers.end());

    m_d->nextBuffer = m_d->buffers.size();

    const size_t buffersToAdd = numBuffers - m_d->buffers.size();

    addBuffersImpl(buffersToAdd, m_d->bufferSize);
}

void KisOpenGLBufferCircularStorage::addBuffersImpl(size_t buffersToAdd, int bufferSize)
{
    m_d->bufferSize = bufferSize;

    const size_t newSize = qMax(m_d->buffers.size() + buffersToAdd, nextPowerOfTwo(m_d->buffers.size()));

    if (m_d->buffers.capacity() < newSize)
        m_d->buffers.reserve(newSize);

    // overflow check for size()
    KIS_ASSERT(m_d->buffers.size() <= std::numeric_limits<int>::max());

    for (size_t i = 0; i < buffersToAdd; i++) {
        m_d->buffers.emplace_back(m_d->type);

        QOpenGLBuffer &buf = m_d->buffers.back();

        buf.create();
        buf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        buf.bind();
        buf.allocate(m_d->bufferSize);
        buf.release();
    }
}
