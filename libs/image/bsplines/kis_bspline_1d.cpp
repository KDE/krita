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

#include "kis_bspline_1d.h"

#include <kis_debug.h>
#include <limits>

#include "einspline/bspline_create.h"
#include "einspline/bspline_eval_std_s.h"

#include "kis_bspline_p.h"

namespace KisBSplines {

struct Q_DECL_HIDDEN KisBSpline1D::Private
{
    BorderCondition bc;
    UBspline_1d_s* spline;
};

KisBSpline1D::KisBSpline1D(float gridStart,
                           float gridEnd,
                           int numSamples,
                           BorderCondition bc)
  : m_d(new Private)
{
    m_gridStart = gridStart;
    m_gridEnd = gridEnd/* + step*/;
    m_numSamples = numSamples/* + 1*/;

    m_d->bc = bc;
    m_d->spline = 0;
}

KisBSpline1D::~KisBSpline1D()
{
    if (m_d->spline) {
        destroy_Bspline(m_d->spline);
    }
}

void KisBSpline1D::initializeSplineImpl(const QVector<float> &values)
{
    Ugrid grid;
    grid.start = m_gridStart;
    grid.end = m_gridEnd;
    grid.num = m_numSamples;
    grid.delta = 0.0;
    grid.delta_inv = 0.0;

    BCtype_s bctype;
    bctype.lCode = bctype.rCode = convertBorderType(m_d->bc);

    m_d->spline =
        create_UBspline_1d_s(grid,
                             bctype,
                             const_cast<float*>(values.constData()));
}

float KisBSpline1D::value(float x) const
{
    /**
     * The spline works for an open interval only, so include the last point
     * explicitly
     */

    if (x == m_gridEnd) {
        x -= x * std::numeric_limits<float>::epsilon();
    }

    KIS_ASSERT_RECOVER_NOOP(x >= m_gridStart && x < m_gridEnd);

    float value;
    eval_UBspline_1d_s (m_d->spline, x, &value);

    return value;
}

}
