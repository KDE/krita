/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_WRAPPED_RECT_ITERATOR_H
#define __KIS_WRAPPED_RECT_ITERATOR_H

#include "kis_iterator_ng.h"
#include "kis_wrapped_rect.h"


class WrappedRectIteratorStrategy
{
public:
    typedef KisRectIteratorSP IteratorTypeSP;

    WrappedRectIteratorStrategy()
        : m_iteratorRowStart(KisWrappedRect::TOPLEFT)
    {
    }

    inline IteratorTypeSP createIterator(KisDataManager *dataManager,
                                         const QRect &rc,
                                         qint32 offsetX, qint32 offsetY,
                                         bool writable) {

        return new KisRectIterator2(dataManager,
                                    rc.x(), rc.y(),
                                    rc.width(),
                                    rc.height(),
                                    offsetX, offsetY,
                                    writable);
    }

    inline void completeInitialization(QVector<IteratorTypeSP> *iterators,
                                       KisWrappedRect *splitRect) {
        m_splitRect = splitRect;
        m_iterators = iterators;
    }

    inline IteratorTypeSP leftColumnIterator() const {
        return m_iterators->at(m_iteratorRowStart + KisWrappedRect::TOPLEFT);
    }

    inline IteratorTypeSP rightColumnIterator() const {
        return m_iterators->at(m_iteratorRowStart + KisWrappedRect::TOPRIGHT);
    }

    inline bool trySwitchColumnForced() {
        bool result =
            m_iteratorRowStart != KisWrappedRect::BOTTOMLEFT &&
            m_iterators->at(KisWrappedRect::BOTTOMLEFT);

        if (result) {
            m_iteratorRowStart = KisWrappedRect::BOTTOMLEFT;
        }

        return result;
    }

    inline bool trySwitchIteratorStripe() {
        qFatal("Not accessible for rect iterator!");
        return false;
    }

    inline void iteratorsToNextRow() {
        qFatal("Not accessible for rect iterator!");
    }

private:
    KisWrappedRect *m_splitRect;
    QVector<IteratorTypeSP> *m_iterators;
    int m_iteratorRowStart; // may be either KisWrappedRect::TOPLEFT or KisWrappedRect::BOTTOMLEFT
};

#include "kis_wrapped_line_iterator_base.h"
typedef KisWrappedLineIteratorBase<WrappedRectIteratorStrategy, KisRectIteratorNG> KisWrappedRectIterator;


#endif /* __KIS_WRAPPED_RECT_ITERATOR_H */
