/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisOpenGLBufferCircularStorage.h"

#include "kis_assert.h"
#include <QVector>


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
    }
}


struct KisOpenGLBufferCircularStorage::Private
{
    int nextBuffer = 0;
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
    m_d->buffers.reserve(numBuffers);

    for (int i = 0; i < numBuffers; i++) {
        m_d->buffers.emplace_back(m_d->type);
        m_d->buffers[i].create();
        m_d->buffers[i].setUsagePattern(QOpenGLBuffer::DynamicDraw);
        m_d->buffers[i].bind();
        m_d->buffers[i].allocate(bufferSize);
        m_d->buffers[i].release();
    }
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

void KisOpenGLBufferCircularStorage::reset()
{
    m_d->buffers.clear();
}
