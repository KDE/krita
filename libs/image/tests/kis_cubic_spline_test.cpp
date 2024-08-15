/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_cubic_spline_test.h"

#include <kis_cubic_curve.h>
#include <kis_cubic_curve_spline.h>

static void compareSplines(const QList<KisCubicCurvePoint> points)
{
    KisLegacyCubicSpline<KisCubicCurvePoint, qreal> legacySpline(points);
    KisCubicSpline<KisCubicCurvePoint, qreal> newSpline(points);

    constexpr qreal epsilon { 0.0000001 };
    constexpr qreal nSteps { 256.0 };
    const qreal stepSize = (points.last().x() - points.first().x()) / nSteps;

    for (qreal x = points.first().x(); x <= points.last().x(); x += stepSize) {
        const qreal legacyY = legacySpline.getValue(x);
        const qreal newY = newSpline.getValue(x);
        const qreal difference = qAbs(newY - legacyY);
        QVERIFY(difference < epsilon);
    }
}

void KisCubicSplineTest::testComparisonLegacyNew1()
{
    QList<KisCubicCurvePoint> points;

    points.append({ 0.0, 0.0, false });
    points.append({ 1.0, 1.0, false });

    compareSplines(points);
}

void KisCubicSplineTest::testComparisonLegacyNew2()
{
    QList<KisCubicCurvePoint> points;

    points.append({ 0.0, 1.0, false });
    points.append({ 0.5, 0.0, false });
    points.append({ 1.0, 1.0, false });

    compareSplines(points);
}

void KisCubicSplineTest::testComparisonLegacyNew3()
{
    QList<KisCubicCurvePoint> points;

    points.append({ 0.0, 0.0, false });
    points.append({ 0.5, 0.25, false });
    points.append({ 1.0, 1.0, false });

    compareSplines(points);
}

void KisCubicSplineTest::testComparisonLegacyNew4()
{
    QList<KisCubicCurvePoint> points;

    points.append({ 0.0, 1.0, false });
    points.append({ 0.25, 0.875, false });
    points.append({ 0.75, 0.125, false });
    points.append({ 1.0, 0.0, false });

    compareSplines(points);
}

SIMPLE_TEST_MAIN(KisCubicSplineTest)
