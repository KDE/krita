/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FOUR_POINT_INTERPOLATOR_BENCHMARK_H
#define __KIS_FOUR_POINT_INTERPOLATOR_BENCHMARK_H

#include <simpletest.h>

class KisFourPointInterpolatorBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testForwardInterpolator();
    void testBackwardInterpolatorXShear();
    void testBackwardInterpolatorYShear();
    void testBackwardInterpolatorXYShear();
    void testBackwardInterpolatorRoundTrip();
    void testBackwardInterpolatorUnevenlyShearedTetragon();
    void testBackwardInterpolatorFoldedTetragon();
    void testBackwardInterpolatorSpecialCase();
};

#endif /* __KIS_FOUR_POINT_INTERPOLATOR_BENCHMARK_H */
