/* This file is part of the KDE project
   Copyright (C) 2005-2006 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KO_XMLVECTOR_H
#define KO_XMLVECTOR_H

// comment it to test this class without the compression
#define KOXMLVECTOR_USE_LZF

#ifdef KOXMLVECTOR_USE_LZF
#include "KoLZF.h"
#endif

#include <QVector>
#include <QByteArray>
#include <QDataStream>
#include <QBuffer>

/**
 * KoXmlVector
 *
 * similar to QVector, but using LZF compression to save memory space
 * this class is however not reentrant
 *
 * Needs to be used like this, otherwise will crash:
 * <ul>
 * <li>add content with newItem()</li>
 * <li>finish adding content with squeeze()</li>
 * <li>just read content with operator[]</li>
 * </ul>
 *
 * @param uncompressedItemCount when number of buffered items reach this,
 *      compression will start small value will give better memory usage at the
 *      cost of speed bigger value will be better in term of speed, but use
 *      more memory
 */
template <typename T, int uncompressedItemCount = 256, int reservedBufferSize = 1024*1024>
class KoXmlVector
{
private:
    unsigned m_totalItems;
    QVector<unsigned> m_startIndex;
    QVector<QByteArray> m_blocks;

    mutable unsigned m_bufferStartIndex;
    mutable QVector<T> m_bufferItems;
    mutable QByteArray m_bufferData;

protected:
    /**
     * fetch given item index to the buffer
     * will INVALIDATE all references to the buffer
     */
    void fetchItem(unsigned index) const {
        // already in the buffer ?
        if (index >= m_bufferStartIndex)
            if (index - m_bufferStartIndex < (unsigned)m_bufferItems.count())
                return;

        // search in the stored blocks
        // TODO: binary search to speed up
        int loc = m_startIndex.count() - 1;
        for (int c = 0; c < m_startIndex.count() - 1; ++c)
            if (index >= m_startIndex[c])
                if (index < m_startIndex[c+1]) {
                    loc = c;
                    break;
                }

        m_bufferStartIndex = m_startIndex[loc];
#ifdef KOXMLVECTOR_USE_LZF
        KoLZF::decompress(m_blocks[loc], m_bufferData);
#else
        m_bufferData = m_blocks[loc];
#endif
        QBuffer buffer(&m_bufferData);
        buffer.open(QIODevice::ReadOnly);
        QDataStream in(&buffer);
        m_bufferItems.clear();
        in >> m_bufferItems;
    }

    /**
     * store data in the buffer to main m_blocks
     */
    void storeBuffer() {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QDataStream out(&buffer);
        out << m_bufferItems;

        m_startIndex.append(m_bufferStartIndex);
#ifdef KOXMLVECTOR_USE_LZF
        m_blocks.append(KoLZF::compress(buffer.data()));
#else
        m_blocks.append(buffer.data());
#endif

        m_bufferStartIndex += m_bufferItems.count();
        m_bufferItems.clear();
    }

public:
    inline KoXmlVector(): m_totalItems(0), m_bufferStartIndex(0) {};

    void clear() {
        m_totalItems = 0;
        m_startIndex.clear();
        m_blocks.clear();

        m_bufferStartIndex = 0;
        m_bufferItems.clear();
        m_bufferData.reserve(reservedBufferSize);
    }

    inline int count() const {
        return (int)m_totalItems;
    }
    inline int size() const {
        return (int)m_totalItems;
    }
    inline bool isEmpty() const {
        return m_totalItems == 0;
    }

    /**
     * append a new item
     * WARNING: use the return value as soon as possible
     * it may be invalid if another function is invoked
     */
    T& newItem() {
        // buffer full?
        if (m_bufferItems.count() >= uncompressedItemCount - 1)
            storeBuffer();

        ++m_totalItems;
        m_bufferItems.resize(m_bufferItems.count() + 1);
        return m_bufferItems[m_bufferItems.count()-1];
    }

    /**
     * WARNING: use the return value as soon as possible
     * it may be invalid if another function is invoked
     */
    const T &operator[](int i) const {
        fetchItem((unsigned)i);
        return m_bufferItems[i - m_bufferStartIndex];
    }

    /**
     * optimize memory usage
     * will INVALIDATE all references to the buffer
     */
    void squeeze() {
        storeBuffer();
    }

};

#endif
