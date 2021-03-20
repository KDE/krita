/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_bsplines_test.h"

#include <simpletest.h>

#include <cmath>

#include <bsplines/kis_bspline_1d.h>
#include <bsplines/kis_bspline_2d.h>
#include <bsplines/kis_nu_bspline_2d.h>
#include <kis_debug.h>
#include <kis_global.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>

using namespace boost::accumulators;
using namespace KisBSplines;

struct FunctionOp {
    float operator() (qreal x) {
        return std::sqrt(x);
    }

    float operator() (qreal x, qreal y) const {
        return std::sqrt(x) + y - pow2(x + y);
    }
};

void KisBSplinesTest::test1D()
{
    const qreal start = 1.0;
    const qreal end = 11.0;

    KisBSpline1D spline(start, end, 10, Natural);
    spline.initializeSpline<FunctionOp>();

    accumulator_set<qreal, stats<tag::variance, tag::max, tag::min> > accum;

    FunctionOp op;
    for (qreal x = start; x < end; x += 0.01) {
        qreal value = op(x);
        qreal relError = (spline.value(x) - value) / value;

        accum(relError);

        if (relError > 0.10) {
            dbgKrita << ppVar(x) << ppVar(op(x)) << ppVar(spline.value(x)) << ppVar(relError);
        }
    }

    dbgKrita << ppVar(count(accum));
    dbgKrita << ppVar(mean(accum));
    dbgKrita << ppVar(variance(accum));
    dbgKrita << ppVar((min)(accum));
    dbgKrita << ppVar((max)(accum));

    qreal maxError = qMax(qAbs((min)(accum)), qAbs((max)(accum)));
    QVERIFY(maxError < 0.10); // Error is less than 10%

}

void KisBSplinesTest::testEmpty1D()
{
    const qreal start = 1.0;
    const qreal end = 11.0;
    KisBSpline1D spline(start, end, 10, Natural);
    // just let it be destructed uninitialized
}

template <class Spline, class Op>
bool test2DSpline(const Spline &spline, const Op &op, qreal start, qreal end)
{
    accumulator_set<qreal, stats<tag::variance, tag::max, tag::min> > accum;

    for (qreal y = start; y < end; y += 0.01) {
        for (qreal x = start; x < end; x += 0.01) {
            qreal value = op(x, y);
            qreal relError = (spline.value(x, y) - value) / value;

            accum(relError);

            if (relError > 0.10) {
                dbgKrita << ppVar(x) << ppVar(y) << ppVar(op(x, y)) << ppVar(spline.value(x, y)) << ppVar(relError);
            }
        }
    }

    dbgKrita << ppVar(count(accum));
    dbgKrita << ppVar(mean(accum));
    dbgKrita << ppVar(variance(accum));
    dbgKrita << ppVar((min)(accum));
    dbgKrita << ppVar((max)(accum));

    qreal maxError = qMax(qAbs((min)(accum)), qAbs((max)(accum)));
    return maxError < 0.10; // Error is less than 10%
}

void KisBSplinesTest::test2D()
{
    const qreal start = 1.0;
    const qreal end = 11.0;

    KisBSpline2D spline(start, end, 10, Natural,
                        start, end, 10, Natural);

    FunctionOp op;
    spline.initializeSpline(op);

    QVERIFY(test2DSpline(spline, op, start, end));

    dbgKrita << "Resampled";

    QScopedPointer<KisBSpline2D> resampledSpline(
        KisBSpline2D::createResampledSpline(spline, 7, 7));

    QVERIFY(test2DSpline(*resampledSpline.data(), op, start, end));
}

void KisBSplinesTest::testEmpty2D()
{
    const qreal start = 1.0;
    const qreal end = 11.0;
    KisBSpline2D spline(start, end, 10, Natural,
                        start, end, 10, Natural);
    // just let it be destructed uninitialized
}

void KisBSplinesTest::testNU2D()
{
    const qreal start = 1.0;
    const qreal end = 11.0;

    QVector<double> samples;

    double v = start;
    int i = 1;
    do {
        samples << v;
        v += (0.1 * i++);
    } while (v < end);

    samples << end;

    KisNUBSpline2D spline(samples, Natural,
                         samples, Natural);
    FunctionOp op;
    spline.initializeSpline(op);

    QVERIFY(test2DSpline(spline, op, start, end));

    dbgKrita << "Resampled";

    QScopedPointer<KisBSpline2D> resampledSpline(
        KisBSpline2D::createResampledSpline(spline, 7, 7));

    QVERIFY(test2DSpline(*resampledSpline.data(), op, start, end));

}

SIMPLE_TEST_MAIN(KisBSplinesTest)
