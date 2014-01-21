/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SEQUENTIAL_ITERATOR_H
#define __KIS_SEQUENTIAL_ITERATOR_H

#include <KoAlwaysInline.h>

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_iterator_ng.h"


struct ReadOnlyIteratorPolicy {
    typedef KisHLineConstIteratorSP IteratorTypeSP;

    ReadOnlyIteratorPolicy(KisPaintDeviceSP dev, const QRect &rect) {
        m_iter = dev->createHLineConstIteratorNG(rect.x(), rect.y(), rect.width());
    }

    ALWAYS_INLINE void updatePointersCache() {
        m_rawDataConst = m_iter->rawDataConst();
        m_oldRawData = m_iter->oldRawData();
    }

    ALWAYS_INLINE const quint8* rawDataConst() const {
        return m_rawDataConst;
    }

    ALWAYS_INLINE const quint8* oldRawData() const {
        return m_oldRawData;
    }

    IteratorTypeSP m_iter;

private:
    const quint8 *m_rawDataConst;
    const quint8 *m_oldRawData;
};

struct WritableIteratorPolicy {
    typedef KisHLineIteratorSP IteratorTypeSP;

    WritableIteratorPolicy(KisPaintDeviceSP dev, const QRect &rect) {
        m_iter = dev->createHLineIteratorNG(rect.x(), rect.y(), rect.width());
    }

    ALWAYS_INLINE void updatePointersCache() {
        m_rawData = m_iter->rawData();
        m_oldRawData = m_iter->oldRawData();
    }

    ALWAYS_INLINE quint8* rawData() {
        return m_rawData;
    }

    ALWAYS_INLINE const quint8* rawDataConst() const {
        return m_rawData;
    }

    ALWAYS_INLINE const quint8* oldRawData() const {
        return m_oldRawData;
    }

    IteratorTypeSP m_iter;

private:
    quint8 *m_rawData;
    const quint8 *m_oldRawData;
};

template <class IteratorPolicy>
class KisSequentialIteratorBase
{
public:
    KisSequentialIteratorBase(KisPaintDeviceSP dev, const QRect &rect)
        : m_policy(dev, rect),
          m_pixelSize(dev->pixelSize()),
          m_rowsLeft(rect.height() - 1),
          m_columnOffset(0)
    {
        m_columnsLeft = m_numConseqPixels = m_policy.m_iter->nConseqPixels();
        m_policy.updatePointersCache();
    }

    inline bool nextPixel() {
        m_columnsLeft--;

        if (m_columnsLeft) {
            m_columnOffset += m_pixelSize;
            return true;
        } else {
            bool result = m_policy.m_iter->nextPixels(m_numConseqPixels);
            if (result) {
                m_columnOffset = 0;
                m_columnsLeft = m_numConseqPixels = m_policy.m_iter->nConseqPixels();
                m_policy.updatePointersCache();
            } if (!result && m_rowsLeft > 0) {
                m_rowsLeft--;
                m_policy.m_iter->nextRow();
                m_columnOffset = 0;
                m_columnsLeft = m_numConseqPixels = m_policy.m_iter->nConseqPixels();
                m_policy.updatePointersCache();
            }

        }
        return m_columnsLeft > 0;
    }

    // SFINAE: This method becomes undefined for const version of the
    //         iterator automatically
    ALWAYS_INLINE quint8* rawData() {
        return m_policy.rawData() + m_columnOffset;
    }

    ALWAYS_INLINE const quint8* rawDataConst() const {
        return m_policy.rawDataConst() + m_columnOffset;
    }

    ALWAYS_INLINE const quint8* oldRawData() const {
        return m_policy.oldRawData() + m_columnOffset;
    }

private:
    Q_DISABLE_COPY(KisSequentialIteratorBase);
    IteratorPolicy m_policy;
    const int m_pixelSize;
    int m_rowsLeft;

    int m_numConseqPixels;
    int m_columnsLeft;

    int m_columnOffset;
};

typedef KisSequentialIteratorBase<ReadOnlyIteratorPolicy> KisSequentialConstIterator;
typedef KisSequentialIteratorBase<WritableIteratorPolicy> KisSequentialIterator;

#endif /* __KIS_SEQUENTIAL_ITERATOR_H */
