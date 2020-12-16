/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_WARP_TRANSFORM_WORKER_TEST_H
#define __KIS_WARP_TRANSFORM_WORKER_TEST_H

#include <QtTest>

class KisWarpTransformWorkerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testQImage();
    void testForwardInterpolator();
    void testBackwardInterpolatorXShear();
    void testBackwardInterpolatorYShear();
    void testBackwardInterpolatorXYShear();
    void testBackwardInterpolatorRoundTrip();
    void testGridSize();
    void testBackwardInterpolatorExtrapolation();

    void testNeedChangeRects();
};

#endif /* __KIS_WARP_TRANSFORM_WORKER_TEST_H */
