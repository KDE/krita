/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
                               bool writable,
                               KisIteratorCompleteListener *listener)
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
                                                       writable,
                                                       listener);
        }
        m_strategy.completeInitialization(&m_iterators, &m_splitRect);
        m_iterationAreaSize =
            m_strategy.originalRectToColumnsRows(m_splitRect.originalRect());

        m_currentIterator = m_strategy.leftColumnIterator();
    }

    bool nextPixel() {
        int result = m_currentIterator->nextPixel();
        if (!result) {
            result = trySwitchColumn();
        }

        m_currentPos.rx()++;
        return m_currentPos.rx() < m_iterationAreaSize.width();
    }

    bool nextPixels(qint32 n) {
        int result = m_currentIterator->nextPixels(n);
        if (!result) {
            result = trySwitchColumn();
        }

        m_currentPos.rx() += n;
        return m_currentPos.rx() < m_iterationAreaSize.width();
    }

    void nextRow() {
        if (!m_strategy.trySwitchIteratorStripe()) {
            m_strategy.iteratorsToNextRow();
        }

        m_currentIterator = m_strategy.leftColumnIterator();
        m_currentPos.rx() = 0;
        m_currentPos.ry()++;
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
        qint32 iteratorChunk =
            m_currentIterator->nConseqPixels();
        return qMin(iteratorChunk,
                    m_iterationAreaSize.width() - m_currentPos.x());
    }

    qint32 x() const {
        return (m_splitRect.originalRect().topLeft() +
                m_strategy.columnRowToXY(m_currentPos)).x();
    }

    qint32 y() const {
        return (m_splitRect.originalRect().topLeft() +
                m_strategy.columnRowToXY(m_currentPos)).y();
    }

    void resetPixelPos() {
        errKrita << "CRITICAL: resetPixelPos() is not implemented";
    }

    void resetRowPos() {
        errKrita << "CRITICAL: resetRowPos() is not implemented";
    }

    void resetColumnPos() {
        resetRowPos();
    }

private:
    bool trySwitchColumn() {
        int result = true;

        if (m_currentIterator == m_strategy.leftColumnIterator() &&
            m_strategy.rightColumnIterator()) {

            m_currentIterator = m_strategy.rightColumnIterator();

        } else if (m_strategy.trySwitchColumnForced()) {

            m_currentIterator = m_strategy.leftColumnIterator();

        } else {
            result = false;
        }

        return result;
    }

private:

    KisWrappedRect m_splitRect;
    QSize m_iterationAreaSize; // columns x rows
    QPoint m_currentPos; // column, row
    QVector<typename IteratorStrategy::IteratorTypeSP> m_iterators;
    typename IteratorStrategy::IteratorTypeSP m_currentIterator;
    IteratorStrategy m_strategy;
};

#endif /* __KIS_WRAPPED_LINE_ITERATOR_BASE_H */
