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

#ifndef __KIS_WRAPPED_VLINE_ITERATOR_H
#define __KIS_WRAPPED_VLINE_ITERATOR_H

#include "kis_iterator_ng.h"
#include "kis_wrapped_rect.h"

class WrappedVLineIteratorStrategy
{
private:
    static const int TOP_OFFSET = 0;
    static const int BOTTOM_OFFSET = 2;

public:
    typedef KisVLineIteratorSP IteratorTypeSP;

    WrappedVLineIteratorStrategy()
        : m_iteratorColumnStart(KisWrappedRect::TOPLEFT),
          m_lastColumnCoord(-1)
    {
    }

    inline QSize originalRectToColumnsRows(const QRect &rect) {
        return QSize(rect.height(), rect.width());
    }

    inline QPoint columnRowToXY(const QPoint &pt) const {
        return QPoint(pt.y(), pt.x());
    }

    inline IteratorTypeSP createIterator(KisDataManager *dataManager,
                                         const QRect &rc,
                                         qint32 offsetX, qint32 offsetY,
                                         bool writable) {

        return new KisVLineIterator2(dataManager,
                                     rc.x(), rc.y(),
                                     rc.height(),
                                     offsetX, offsetY,
                                     writable);
    }

    inline void completeInitialization(QVector<IteratorTypeSP> *iterators,
                                       KisWrappedRect *splitRect) {
        m_splitRect = splitRect;
        m_iterators = iterators;

        m_lastColumnCoord = m_splitRect->topLeft().right();
    }

    inline IteratorTypeSP leftColumnIterator() const {
        return m_iterators->at(m_iteratorColumnStart + TOP_OFFSET);
    }

    inline IteratorTypeSP rightColumnIterator() const {
        return m_iterators->at(m_iteratorColumnStart + BOTTOM_OFFSET);
    }

    inline bool trySwitchIteratorStripe() {
        bool needSwitching = leftColumnIterator()->x() == m_lastColumnCoord;

            if (needSwitching) {
                if (m_iteratorColumnStart == KisWrappedRect::TOPLEFT &&
                    m_iterators->at(KisWrappedRect::TOPRIGHT)) {

                    m_iteratorColumnStart = KisWrappedRect::TOPRIGHT;
                    m_lastColumnCoord = m_splitRect->topRight().right();
                } else /* if (m_iteratorColumnStart == KisWrappedRect::TOPRIGHT) */ {
                    m_iteratorColumnStart = KisWrappedRect::TOPLEFT;
                    m_lastColumnCoord = m_splitRect->topLeft().right();

                    Q_FOREACH (IteratorTypeSP it, *m_iterators) {
                        if (it) {
                            it->resetColumnPos();
                        }
                    }
                }
            }

            return needSwitching;
    }

    inline void iteratorsToNextRow() {
        leftColumnIterator()->nextColumn();
        if (rightColumnIterator()) {
            rightColumnIterator()->nextColumn();
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
    int m_iteratorColumnStart; // may be either KisWrappedRect::TOPLEFT or KisWrappedRect::TOPRIGHT
    int m_lastColumnCoord;
};

#include "kis_wrapped_line_iterator_base.h"
typedef KisWrappedLineIteratorBase<WrappedVLineIteratorStrategy, KisVLineIteratorNG> KisWrappedVLineIterator;


#endif /* __KIS_WRAPPED_VLINE_ITERATOR_H */
