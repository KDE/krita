/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_nu_bspline_2d.h"

#include <kis_debug.h>
#include <limits>

#include "einspline/nubspline_create.h"
#include "einspline/nubspline_eval_std_s.h"

#include "kis_bspline_p.h"

namespace KisBSplines {

struct Q_DECL_HIDDEN KisNUBSpline2D::Private
{
    BorderCondition bcX;
    BorderCondition bcY;

    NUBspline_2d_s* spline;

    NUgrid *xGrid;
    NUgrid *yGrid;

    float minX;
    float maxX;
    float minY;
    float maxY;
};

KisNUBSpline2D::KisNUBSpline2D(const QVector<double> &xSamples, BorderCondition bcX,
                               const QVector<double> &ySamples, BorderCondition bcY)
    : m_d(new Private),
      m_xSamples(xSamples),
      m_ySamples(ySamples)
{
    m_d->xGrid = create_general_grid(const_cast<double*>(m_xSamples.constData()), m_xSamples.size());
    m_d->yGrid = create_general_grid(const_cast<double*>(m_ySamples.constData()), m_ySamples.size());

    m_d->bcX = bcX;
    m_d->bcY = bcY;

    m_d->minX = xSamples.first();
    m_d->maxX = xSamples.last();

    m_d->minY = ySamples.first();
    m_d->maxY = ySamples.last();

    m_d->spline = 0;
}

KisNUBSpline2D::~KisNUBSpline2D()
{
    if (m_d->spline) {
        destroy_Bspline(m_d->spline);
    }

    destroy_grid(m_d->xGrid);
    destroy_grid(m_d->yGrid);
}

void KisNUBSpline2D::initializeSplineImpl(const QVector<float> &values)
{
    BCtype_s bctypeX;
    bctypeX.lCode = bctypeX.rCode = convertBorderType(m_d->bcX);
    bctypeX.lVal = bctypeX.rVal = 0.0;

    BCtype_s bctypeY;
    bctypeY.lCode = bctypeY.rCode = convertBorderType(m_d->bcY);
    bctypeY.lVal = bctypeY.rVal = 0.0;

    m_d->spline =
        create_NUBspline_2d_s(m_d->xGrid, m_d->yGrid,
                              bctypeX, bctypeY,
                              const_cast<float*>(values.constData()));
}

float KisNUBSpline2D::value(float x, float y) const
{
    /**
     * The spline works for an open interval only, so include the last point
     * explicitly
     */

    if (x == m_d->maxX) {
        x -= x * std::numeric_limits<float>::epsilon();
    }

    if (y == m_d->maxY) {
        y -= y * std::numeric_limits<float>::epsilon();
    }

    KIS_ASSERT_RECOVER_NOOP(x >= m_d->minX && x < m_d->maxX);
    KIS_ASSERT_RECOVER_NOOP(y >= m_d->minY && y < m_d->maxY);

    float value;
    eval_NUBspline_2d_s(m_d->spline, x, y, &value);

    return value;
}

QPointF KisNUBSpline2D::topLeft() const
{
    return QPointF(m_d->minX, m_d->minY);
}

QPointF KisNUBSpline2D::bottomRight() const
{
    return QPointF(m_d->maxX, m_d->maxY);
}

BorderCondition KisNUBSpline2D::borderConditionX() const
{
    return m_d->bcX;
}

BorderCondition KisNUBSpline2D::borderConditionY() const
{
    return m_d->bcY;
}

}
