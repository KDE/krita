/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
