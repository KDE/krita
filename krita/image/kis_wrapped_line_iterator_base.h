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

#ifndef __KIS_WRAPPED_LINE_ITERATOR_BASE_H
#define __KIS_WRAPPED_LINE_ITERATOR_BASE_H


template <class IteratorStrategy, class BaseClass>
    class KisWrappedLineIteratorBase : public BaseClass
{
public:
    KisWrappedLineIteratorBase(KisDataManager *dataManager,
                               const KisWrappedRect &splitRect,
                               qint32 offsetX, qint32 offsetY,
                               bool writable)
        : m_splitRect(splitRect)
    {
        Q_ASSERT(m_splitRect.isSplit());

        m_iterators.resize(4);
        for (int i = 0; i < 4; i++) {
            QRect rc = m_splitRect[i];
            if (rc.isEmpty()) continue;

            m_iterators[i] = m_strategy.createIterator(dataManager,
                                                       rc,
                                                       offsetX, offsetY,
                                                       writable);
        }
        m_strategy.completeInitialization(&m_iterators, &m_splitRect);

        m_currentIterator = m_strategy.leftColumnIterator();
    }

    bool nextPixel() {
        int result = m_currentIterator->nextPixel();
        if (!result) {
            result = trySwitchColumn();
        }

        return result;
    }

    bool nextPixels(qint32 n) {
        int result = m_currentIterator->nextPixels(n);
        if (!result) {
            result = trySwitchColumn();
        }

        return result;
    }

    void nextRow() {
        if (!m_strategy.trySwitchIteratorStripe()) {
            m_strategy.iteratorsToNextRow();
        }

        m_currentIterator = m_strategy.leftColumnIterator();
    }

    void nextColumn() {
        nextRow();
    }

    const quint8* oldRawData() const {
        return m_currentIterator->oldRawData();
    }

    const quint8* rawDataConst() const {
        return m_currentIterator->rawDataConst();
    }

    quint8* rawData() {
        return m_currentIterator->rawData();
    }

    qint32 nConseqPixels() const {
        return m_currentIterator->nConseqPixels();
    }

    qint32 x() const {
        return m_splitRect.wrappedXToX(m_currentIterator->x());
    }

    qint32 y() const {
        return m_splitRect.wrappedYToY(m_currentIterator->y());
    }

private:
    bool trySwitchColumn() {
        int result =
            m_currentIterator == m_strategy.leftColumnIterator() &&
            m_strategy.rightColumnIterator();

        if (result) {
            m_currentIterator = m_strategy.rightColumnIterator();
        } else {
            result = m_strategy.trySwitchColumnForced();
            if (result) {
                m_currentIterator = m_strategy.leftColumnIterator();
            }
        }

        return result;
    }

private:

    KisWrappedRect m_splitRect;
    QVector<typename IteratorStrategy::IteratorTypeSP> m_iterators;
    typename IteratorStrategy::IteratorTypeSP m_currentIterator;
    IteratorStrategy m_strategy;
};

#endif /* __KIS_WRAPPED_LINE_ITERATOR_BASE_H */
