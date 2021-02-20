/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
