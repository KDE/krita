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

#ifndef __KIS_WRAPPED_HLINE_ITERATOR_H
#define __KIS_WRAPPED_HLINE_ITERATOR_H

#include "kis_iterator_ng.h"
#include "kis_wrapped_rect.h"


class WrappedHLineIteratorStrategy
{
public:
    typedef KisHLineIteratorSP IteratorTypeSP;

    WrappedHLineIteratorStrategy()
        : m_iteratorRowStart(KisWrappedRect::TOPLEFT),
          m_lastRowCoord(-1)
    {
    }

    inline QSize originalRectToColumnsRows(const QRect &rect) {
        return rect.size();
    }

    inline QPoint columnRowToXY(const QPoint &pt) const {
        return pt;
    }

    inline IteratorTypeSP createIterator(KisDataManager *dataManager,
                                         const QRect &rc,
                                         qint32 offsetX, qint32 offsetY,
                                         bool writable) {

        return new KisHLineIterator2(dataManager,
                                     rc.x(), rc.y(),
                                     rc.width(),
                                     offsetX, offsetY,
                                     writable);
    }

    inline void completeInitialization(QVector<IteratorTypeSP> *iterators,
                                       KisWrappedRect *splitRect) {
        m_splitRect = splitRect;
        m_iterators = iterators;

        m_lastRowCoord = m_splitRect->topLeft().bottom();
    }

    inline IteratorTypeSP leftColumnIterator() const {
        return m_iterators->at(m_iteratorRowStart + KisWrappedRect::TOPLEFT);
    }

    inline IteratorTypeSP rightColumnIterator() const {
        return m_iterators->at(m_iteratorRowStart + KisWrappedRect::TOPRIGHT);
    }

    inline bool trySwitchIteratorStripe() {
        bool needSwitching = leftColumnIterator()->y() == m_lastRowCoord;

        if (needSwitching) {
            if (m_iteratorRowStart == KisWrappedRect::TOPLEFT &&
                m_iterators->at(KisWrappedRect::BOTTOMLEFT)) {

                m_iteratorRowStart = KisWrappedRect::BOTTOMLEFT;
                m_lastRowCoord = m_splitRect->bottomLeft().bottom();
            } else /* if (m_iteratorRowStart == KisWrappedRect::BOTTOMLEFT) */ {
                m_iteratorRowStart = KisWrappedRect::TOPLEFT;
                m_lastRowCoord = m_splitRect->topLeft().bottom();

                Q_FOREACH (IteratorTypeSP it, *m_iterators) {
                    if (it) {
                        it->resetRowPos();
                    }
                }
            }
        }

        return needSwitching;
    }

    inline void iteratorsToNextRow() {
        leftColumnIterator()->nextRow();
        if (rightColumnIterator()) {
            rightColumnIterator()->nextRow();
        }
    }

    inline bool trySwitchColumnForced() {
        leftColumnIterator()->resetPixelPos();
        if (rightColumnIterator()) {
            rightColumnIterator()->resetPixelPos();
        }
        return true;
    }

private:
    KisWrappedRect *m_splitRect;
    QVector<IteratorTypeSP> *m_iterators;
    int m_iteratorRowStart; // may be either KisWrappedRect::TOPLEFT or KisWrappedRect::BOTTOMLEFT
    int m_lastRowCoord;
};

#include "kis_wrapped_line_iterator_base.h"
typedef KisWrappedLineIteratorBase<WrappedHLineIteratorStrategy, KisHLineIteratorNG> KisWrappedHLineIterator;


#endif /* __KIS_WRAPPED_HLINE_ITERATOR_H */
