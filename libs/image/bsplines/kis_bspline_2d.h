/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BSPLINE_2D_H
#define __KIS_BSPLINE_2D_H

#include <kritaimage_export.h>

#include <QScopedPointer>
#include <QVector>
#include <QPointF>
#include <QSize>

#include "kis_bspline.h"


namespace KisBSplines {

template <class Spline>
struct ConvertSplineOp {
    ConvertSplineOp(const Spline &spline) : m_spline(spline) {}

    float operator() (float x, float y) const {
        return m_spline.value(x, y);
    }

    const Spline &m_spline;
};

class KRITAIMAGE_EXPORT KisBSpline2D
{
public:
    KisBSpline2D(float xStart, float xEnd, int numSamplesX, BorderCondition bcX,
                 float yStart, float yEnd, int numSamplesY, BorderCondition bcY);

    ~KisBSpline2D();

    template <class Spline>
        static inline KisBSpline2D* createResampledSpline(const Spline &other, int xSamples, int ySamples) {
        QPointF tl = other.topLeft();
        QPointF br = other.bottomRight();

        KisBSpline2D *newSpline =
            new KisBSpline2D(tl.x(), br.x(), xSamples, other.borderConditionX(),
                             tl.y(), br.y(), ySamples, other.borderConditionY());

        ConvertSplineOp<Spline> op(other);
        newSpline->initializeSpline(op);

        return newSpline;
    }

    template <class FunctionOp>
    inline void initializeSpline(const FunctionOp &op) {

        float xStep = (m_xEnd - m_xStart) / (m_numSamplesX - 1);
        float yStep = (m_yEnd - m_yStart) / (m_numSamplesY - 1);

        QVector<float> values(m_numSamplesX * m_numSamplesY);

        for (int x = 0; x < m_numSamplesX; x++) {
            float fx = m_xStart + xStep * x;

            for (int y = 0; y < m_numSamplesY; y++) {
                float fy = m_yStart + yStep * y;
                float v = op(fx, fy);
                values[x * m_numSamplesY + y] = v;
            }
        }

        initializeSplineImpl(values);
    }

    float value(float x, float y) const;

    inline QPointF topLeft() const {
        return QPointF(m_xStart, m_yStart);
    }

    inline QPointF bottomRight() const {
        return QPointF(m_xEnd, m_yEnd);
    }

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
    float m_xStart;
    float m_xEnd;
    int m_numSamplesX;

    float m_yStart;
    float m_yEnd;
    int m_numSamplesY;
};

}

#endif /* __KIS_BSPLINE_2D_H */
