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

#ifndef __KIS_FOUR_POINT_INTERPOLATOR_BACKWARD_H
#define __KIS_FOUR_POINT_INTERPOLATOR_BACKWARD_H

#include <QPolygon>
#include <QPointF>

#include "kis_global.h"


/**
 *    A-----B         The polygons must be initialized in this order:
 *    |     |
 *    |     |         polygon << A << B << D << C;
 *    C-----D
 */

class KisFourPointInterpolatorBackward
{
public:
    KisFourPointInterpolatorBackward(const QPolygonF &srcPolygon, const QPolygonF &dstPolygon) {
        m_a = dstPolygon[1] - dstPolygon[0]; // AB
        m_b = dstPolygon[2] - dstPolygon[1]; // BD
        m_c = dstPolygon[3] - dstPolygon[0]; // AC
        m_d = m_b - m_c; // BD - AC

        m_qA = m_c.x() * m_d.y() - m_c.y() * m_d.x();

        m_srcBase = srcPolygon[0];
        m_dstBase = dstPolygon[0];
        m_xCoeff = srcPolygon[1].x() - srcPolygon[0].x(); // AB_src
        m_yCoeff = srcPolygon[3].y() - srcPolygon[0].y(); // AC_src

        m_qB_const = m_c.x() * m_a.y() - m_c.y() * m_a.x();

        m_qD_div = 1.0 / (2 * m_qA);

        //m_qB_varX = 0.0;
        //m_qB_varY = 0.0;
    }

    inline QPointF map(const QPointF &pt) {
        setX(pt.x());
        setY(pt.y());
        return getValue();
    }

    inline void setX(qreal x) {
        x -= m_dstBase.x();

        m_qB_varX = - x * m_d.y();
        m_qC_varX = - x * m_a.y();
        m_px = x;
    }

    inline void setY(qreal y) {
        y -= m_dstBase.y();

        m_qB_varY = y * m_d.x();
        m_qC_varY = y * m_a.x();
        m_py = y;
    }

    inline QPointF getValue() const {
        static const qreal eps = 1e-10;

        qreal qB = m_qB_const + m_qB_varX + m_qB_varY;
        qreal qC = m_qC_varX + m_qC_varY;

        qreal nu = 0.0;

        if (qAbs(m_qA) < eps) {
            nu = -qC / qB;
        } else {
            qreal D = pow2(qB) - 4 * m_qA * qC;
            if (D > 0.0) {
                qreal sqrtD = std::sqrt(D);
                nu = (-qB - sqrtD) * m_qD_div;
                if (nu < 0.0 || nu > 1.0) {
                    qreal nu2 = (-qB + sqrtD) * m_qD_div;

                    if (nu2 < 0.0 || nu2 > 1.0) {
                        nu = qBound(0.0, nu, 1.0);
                    } else {
                        nu = nu2;
                    }
                }
            } else {
                nu = 0.0;
            }
        }

        qreal xBasedDenominator = m_a.x() + nu * m_d.x();

        qreal mu;

        if (qAbs(xBasedDenominator) > eps) {
            mu = (m_px - nu * m_c.x()) / xBasedDenominator;
        } else {
            mu = (m_py - nu * m_c.y()) / (m_a.y() + nu * m_d.y());
        }

        return m_srcBase + QPointF(mu * m_xCoeff, nu * m_yCoeff);
    }

private:
    QPointF m_a; // AB
    QPointF m_b; // BD
    QPointF m_c; // AC
    QPointF m_d; // m_b - m_c

    qreal m_qA; // quadratic equation A coeff
    qreal m_qB_const; // quadratic equation B coeff, const part
    qreal m_qB_varX; // quadratic equation B coeff, X-dep part
    qreal m_qB_varY; // quadratic equation B coeff, Y-dep part
    qreal m_qC_varX; // quadratic equation C coeff, X-dep part
    qreal m_qC_varY; // quadratic equation C coeff, Y-dep part
    qreal m_qD_div; // inverted divisor of the quadratic equation solution
    qreal m_px; // saved relative X coordinate
    qreal m_py; // saved relative Y coordinate

    QPointF m_srcBase;
    QPointF m_dstBase;
    qreal m_xCoeff;
    qreal m_yCoeff;
};

#endif /* __KIS_FOUR_POINT_INTERPOLATOR_BACKWARD_H */
