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

#ifndef __KIS_NU_BSPLINE_2D_H
#define __KIS_NU_BSPLINE_2D_H

#include <kritaimage_export.h>

#include <QScopedPointer>
#include <QVector>
#include <QPointF>
#include <QSize>

#include "kis_bspline.h"


namespace KisBSplines {

class KRITAIMAGE_EXPORT KisNUBSpline2D
{
public:
    KisNUBSpline2D(const QVector<double> &xSamples, BorderCondition bcX,
                   const QVector<double> &ySamples, BorderCondition bcY);

    ~KisNUBSpline2D();

    template <class FunctionOp>
        inline void initializeSpline(const FunctionOp &op) {

        const int xSize = m_xSamples.size();
        const int ySize = m_ySamples.size();

        QVector<float> values(xSize * ySize);

        for (int x = 0; x < xSize; x++) {
            double fx = m_xSamples[x];

            for (int y = 0; y < ySize; y++) {
                double fy = m_ySamples[y];
                float v = op(fx, fy);
                values[x * ySize + y] = v;
            }
        }

        initializeSplineImpl(values);
    }

    float value(float x, float y) const;

    QPointF topLeft() const;
    QPointF bottomRight() const;

    BorderCondition borderConditionX() const;
    BorderCondition borderConditionY() const;

private:
    void initializeSplineImpl(const QVector<float> &values);

private:
    struct Private;
    const QScopedPointer<Private> m_d;

    /**
     * We need to store them separately, because they should
     * be accessible from the templated part
     */
    const QVector<double> m_xSamples;
    const QVector<double> m_ySamples;
};

}

#endif /* __KIS_NU_BSPLINE_2D_H */
