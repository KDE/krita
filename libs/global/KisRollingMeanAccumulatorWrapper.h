/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISROLLINGMEANACCUMULATORWRAPPER_H
#define KISROLLINGMEANACCUMULATORWRAPPER_H

#include <QtGlobal>
#include <QScopedPointer>
#include "kritaglobal_export.h"

/**
 * @brief A simple wrapper class that hides boost includes from QtCreator preventing it
 * from crashing when one adds boost's accumulator into a file
 */

class KRITAGLOBAL_EXPORT KisRollingMeanAccumulatorWrapper
{
public:
    /**
     * Create a rolling mean accumulator with window \p windowSize
     */
    KisRollingMeanAccumulatorWrapper(int windowSize);
    ~KisRollingMeanAccumulatorWrapper();

    /**
     * Add \p value to a set of numbers
     */
    void operator()(qreal value);

    /**
     * Get rolling mean of the numbers passed to the operator. If there are no elements
     * in the rolling window, returns NaN.
     */
    qreal rollingMean() const;

    /**
     * Get rolling mean of the numbers passed to the operator. If there are no elements
     * in the rolling window, returns 0.
     */
    qreal rollingMeanSafe() const;

    /**
     * Get the number of elements in the rolling window
     */
    int rollingCount() const;

    /**
     * Reset  accumulator and any stored value
     */
    void reset(int windowSize);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISROLLINGMEANACCUMULATORWRAPPER_H
