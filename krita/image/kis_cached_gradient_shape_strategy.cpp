/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_cached_gradient_shape_strategy.h"

#include <QRect>
#include "bsplines/kis_bspline_2d.h"

#include <cmath>

#include <boost/function.hpp>
#include <boost/bind.hpp>

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
        qWarning();
        qWarning() << "############";
        qWarning() << "WARNING: KisCachedGradientShapeStrategy numSamplesX/Y is too small!"  << ppVar(numSamplesX) << ppVar(numSamplesY);
        qWarning() << "WARNING:" << ppVar(rc) << ppVar(xStep) << ppVar(yStep);
        qWarning() << "WARNING:" << ppVar(numSamplesX) << ppVar(numSamplesY);

        numSamplesX = qMax(numSamplesX, 2);
        numSamplesY = qMax(numSamplesY, 2);

        qWarning() << "WARNING: adjusting:" << ppVar(numSamplesX) << ppVar(numSamplesY);
        qWarning() << "############";
        qWarning();
    }

    m_d->spline.reset(new KisBSpline2D(xStart, xEnd, numSamplesX, Natural,
                                       yStart, yEnd, numSamplesY, Natural));


    boost::function<qreal(qreal, qreal)> valueOp =
        boost::bind(&KisGradientShapeStrategy::valueAt, m_d->baseStrategy.data(), _1, _2);

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
