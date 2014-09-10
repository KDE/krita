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

#ifndef __KIS_BSPLINE_1D_H
#define __KIS_BSPLINE_1D_H

#include <krita_export.h>

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
