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

#include "kis_bspline_2d.h"

#include <kis_debug.h>
#include <limits>

#include "einspline/bspline_create.h"
#include "einspline/bspline_eval_std_s.h"

#include "kis_bspline_p.h"

namespace KisBSplines {

struct Q_DECL_HIDDEN KisBSpline2D::Private
{
    BorderCondition bcX;
    BorderCondition bcY;

    UBspline_2d_s* spline;
};

KisBSpline2D::KisBSpline2D(float xStart, float xEnd, int numSamplesX, BorderCondition bcX,
                           float yStart, float yEnd, int numSamplesY, BorderCondition bcY)
  : m_d(new Private)
{
    m_xStart = xStart;
    m_xEnd = xEnd;
    m_numSamplesX = numSamplesX;

    m_yStart = yStart;
    m_yEnd = yEnd;
    m_numSamplesY = numSamplesY;

    m_d->bcX = bcX;
    m_d->bcY = bcY;

    m_d->spline = 0;
}

KisBSpline2D::~KisBSpline2D()
{
    if (m_d->spline) {
        destroy_Bspline(m_d->spline);
    }
}

void KisBSpline2D::initializeSplineImpl(const QVector<float> &values)
{
    Ugrid xGrid;
    xGrid.start = m_xStart;
    xGrid.end = m_xEnd;
    xGrid.num = m_numSamplesX;
    xGrid.delta = 0.0;
    xGrid.delta_inv = 0.0;

    Ugrid yGrid;
    yGrid.start = m_yStart;
    yGrid.end = m_yEnd;
    yGrid.num = m_numSamplesY;
    yGrid.delta = 0.0;
    yGrid.delta_inv = 0.0;

    BCtype_s bctypeX;
    bctypeX.lCode = bctypeX.rCode = convertBorderType(m_d->bcX);

    BCtype_s bctypeY;
    bctypeY.lCode = bctypeY.rCode = convertBorderType(m_d->bcY);

    m_d->spline =
        create_UBspline_2d_s(xGrid, yGrid,
                             bctypeX, bctypeY,
                             const_cast<float*>(values.constData()));
}

float KisBSpline2D::value(float x, float y) const
{
    /**
     * The spline works for an open interval only, so include the last point
     * explicitly
     */

    if (x == m_xEnd) {
        x -= x * std::numeric_limits<float>::epsilon();
    }

    if (y == m_yEnd) {
        y -= y * std::numeric_limits<float>::epsilon();
    }

    KIS_ASSERT_RECOVER_NOOP(x >= m_xStart && x < m_xEnd);
    KIS_ASSERT_RECOVER_NOOP(y >= m_yStart && y < m_yEnd);

    float value;
    eval_UBspline_2d_s (m_d->spline, x, y, &value);

    return value;
}

BorderCondition KisBSpline2D::borderConditionX() const
{
    return m_d->bcX;
}

BorderCondition KisBSpline2D::borderConditionY() const
{
    return m_d->bcY;
}

}
