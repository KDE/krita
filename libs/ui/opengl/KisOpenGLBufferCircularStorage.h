/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISOPENGLBUFFERCIRCULARSTORAGE_H
#define KISOPENGLBUFFERCIRCULARSTORAGE_H

#include <QScopedPointer>
#include <QOpenGLBuffer>

/**
 * A simple storage class that owns a fixed amount of
 * QOpenGLBuffer objects and returns them sequentially.
 * Using multiple distinct buffers lets us avoid blocks
 */
class KisOpenGLBufferCircularStorage
{
public:
    struct BufferBinder
    {
        /**
         * When \p bufferStorage is non-null, fetches the
         * next buffer from the circular storage,  binds it
         * and loads the provided data into the buffer. After
         * loading the data pointer `*dataPtr` is reset to null
         * to signal that glTexImage2D should not be provided
         * with any pointer.
         *
         * When \p bufferStorage is null, the binder does nothing,
         * the data pointer `*dataPtr`is kept unchanged.
         */
        BufferBinder(KisOpenGLBufferCircularStorage *bufferStorage, const void **dataPtr, int dataSize);
        ~BufferBinder();

        BufferBinder(const BufferBinder &) = delete;
        BufferBinder &operator=(const BufferBinder &) = delete;
        BufferBinder(BufferBinder &&) = delete;
        BufferBinder &operator=(BufferBinder &&) = delete;

    private:
        QOpenGLBuffer *m_buffer = nullptr;
    };

    KisOpenGLBufferCircularStorage();
    KisOpenGLBufferCircularStorage(QOpenGLBuffer::Type type);
    ~KisOpenGLBufferCircularStorage();

    KisOpenGLBufferCircularStorage(const KisOpenGLBufferCircularStorage &) = delete;
    KisOpenGLBufferCircularStorage &operator=(const KisOpenGLBufferCircularStorage &) = delete;
    KisOpenGLBufferCircularStorage(KisOpenGLBufferCircularStorage &&) = delete;
    KisOpenGLBufferCircularStorage &operator=(KisOpenGLBufferCircularStorage &&) = delete;

    void allocate(int numBuffers, int bufferSize);
    QOpenGLBuffer* getNextBuffer();
    bool isValid() const;
    int size() const;

    void reset();

    void allocateMoreBuffers();

private:
    void addBuffersImpl(size_t buffersToAdd, int bufferSize);

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISOPENGLBUFFERCIRCULARSTORAGE_H
