/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISFILTEREDROLLINGMEAN_H
#define KISFILTEREDROLLINGMEAN_H

#include "kritaglobal_export.h"

#include <vector>
#include <boost/circular_buffer.hpp>
#include <QtGlobal>

/**
 * A special class that calculates a rolling mean of the
 * values stream, but filtering out random extreme deviations.
 *
 * On each step the algorithm sorts the entire sampling window
 * and drops a small portion of the lowest and the highest
 * values. The rest of the values are used to calculate the
 * mean of the window.
 *
 * Basically, it takes the median and a few surrounding values
 * to calculate the mean.
 *
 * PS:
 * The actually implementation does a few optimizations. For
 * example, it doesn't sort the entire range. But the idea is
 * the same.
 */
class KRITAGLOBAL_EXPORT KisFilteredRollingMean
{
public:
    /**
     * Creates a mean accumulator
     *
     * \p windowSize is the size of the samples window
     * \p effectivePortion the portion of the samples window
     *    that is used for actual mean calculation. On each
     *    side of the sorted range (0.5 * (1.0 - effectivePortion) *
     *    windowSize) values are dropped and are not counted for
     *    the mean calculation.
     */
    KisFilteredRollingMean(int windowSize, qreal effectivePortion);

    /**
     * Adds a value for the rolling mean calculation. The
     * complexity of the call is O(1).
     */
    void addValue(qreal value);

    /**
     * Calculates the filtered mean value of the current range.
     * The function is slow, its complexity is O(N) + O(N*log(M)),
     * where N is the size of the rolling window, M is the number
     * of elements dropped from the window
     * (1.0 - effectivePortion) * windowSize).
     */
    qreal filteredMean() const;

    /**
     * Returns true if the accumulator has at least some value
     */
    bool isEmpty() const;

private:
    boost::circular_buffer<qreal> m_values;
    qreal m_rollingSum;
    qreal m_effectivePortion;
    mutable std::vector<qreal> m_cutOffBuffer;
};

#endif // KISFILTEREDROLLINGMEAN_H
