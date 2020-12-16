/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_cached_gradient_shape_strategy.h"

#include <QRect>
#include "bsplines/kis_bspline_2d.h"

#include <cmath>

#include <functional>

#include "kis_algebra_2d.h"
#include "kis_debug.h"


using namespace KisBSplines;

struct Q_DECL_HIDDEN KisCachedGradientShapeStrategy::Private
{
    QRect rc;
    qreal xStep;
    qreal yStep;
    QScopedPointer<KisGradientShapeStrategy> baseStrategy;
    QScopedPointer<KisBSpline2D> spline;
};

KisCachedGradientShapeStrategy::KisCachedGradientShapeStrategy(const QRect &rc,
                                                               qreal xStep,
                                                               qreal yStep,
                                                               KisGradientShapeStrategy *baseStrategy)
    : KisGradientShapeStrategy(),
      m_d(new Private())
{
    using namespace std::placeholders;  // for _1, _2, _3...

    KIS_ASSERT_RECOVER_NOOP(rc.width() >= 3 && rc.height() >= 3);

    m_d->rc = rc;
    m_d->xStep = xStep;
    m_d->yStep = yStep;
    m_d->baseStrategy.reset(baseStrategy);

    qreal xStart = rc.x();
    qreal yStart = rc.y();
    qreal xEnd = rc.x() + rc.width();
    qreal yEnd = rc.y() + rc.height();

    int numSamplesX = std::ceil(qreal(rc.width()) / xStep);
    int numSamplesY = std::ceil(qreal(rc.height()) / yStep);

    if (numSamplesX < 2 || numSamplesY < 2) {
        warnKrita;
        warnKrita << "############";
        warnKrita << "WARNING: KisCachedGradientShapeStrategy numSamplesX/Y is too small!"  << ppVar(numSamplesX) << ppVar(numSamplesY);
        warnKrita << "WARNING:" << ppVar(rc) << ppVar(xStep) << ppVar(yStep);
        warnKrita << "WARNING:" << ppVar(numSamplesX) << ppVar(numSamplesY);

        numSamplesX = qMax(numSamplesX, 2);
        numSamplesY = qMax(numSamplesY, 2);

        warnKrita << "WARNING: adjusting:" << ppVar(numSamplesX) << ppVar(numSamplesY);
        warnKrita << "############";
        warnKrita;
    }

    m_d->spline.reset(new KisBSpline2D(xStart, xEnd, numSamplesX, Natural,
                                       yStart, yEnd, numSamplesY, Natural));


    std::function<qreal(qreal, qreal)> valueOp =
        std::bind(&KisGradientShapeStrategy::valueAt, m_d->baseStrategy.data(), _1, _2);

    m_d->spline->initializeSpline(valueOp);

}

KisCachedGradientShapeStrategy::~KisCachedGradientShapeStrategy()
{
}

double KisCachedGradientShapeStrategy::valueAt(double x, double y) const
{
    QPointF pt = KisAlgebra2D::ensureInRect(QPointF(x, y), m_d->rc);
    return m_d->spline->value(pt.x(), pt.y());
}
