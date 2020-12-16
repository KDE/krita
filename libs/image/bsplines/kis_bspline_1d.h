/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BSPLINE_1D_H
#define __KIS_BSPLINE_1D_H

#include <kritaimage_export.h>

#include <QScopedPointer>
#include <QVector>

#include "kis_bspline.h"


namespace KisBSplines {

class KRITAIMAGE_EXPORT KisBSpline1D
{
public:
    KisBSpline1D(float gridStart, float gridEnd, int numSamples, BorderCondition bc);
    ~KisBSpline1D();

    template <class FunctionOp>
        inline void initializeSpline() {

        FunctionOp op;
        float step = (m_gridEnd - m_gridStart) / (m_numSamples - 1);
        QVector<float> values(m_numSamples);

        for (int i = 0; i < m_numSamples; i++) {
            float x = m_gridStart + i * step;
            float y = op(x);

            values[i] = y;
        }

        initializeSplineImpl(values);
    }

    float value(float x) const;

    inline float gridStart() const {
        return m_gridStart;
    }

    inline float gridEnd() const {
        return m_gridEnd;
    }

private:
    void initializeSplineImpl(const QVector<float> &values);

private:
    struct Private;
    const QScopedPointer<Private> m_d;

    /**
     * We need to store them separately, because they should
     * be accessible from the templated part
     */
    float m_gridStart;
    float m_gridEnd;
    int m_numSamples;
};

}

#endif /* __KIS_BSPLINE_1D_H */
