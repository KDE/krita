/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FOUR_POINT_INTERPOLATOR_BACKWARD_H
#define __KIS_FOUR_POINT_INTERPOLATOR_BACKWARD_H

#include <QPolygon>
#include <QPointF>

#include "kis_global.h"
#include "kis_algebra_2d.h"


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

    /**
     * Checks if linear dimensions of the destination polygon are
     * bigger than \p tolerance.
     */
    inline bool isValid(const qreal tolerance = 0.1) const {
        const qreal toleranceSq = pow2(tolerance);

        const qreal sq1 = qAbs(m_qB_const);
        const qreal sq2 = qAbs(KisAlgebra2D::crossProduct(m_b, m_c - m_b + m_a));

        return sq1 + sq2 > 2 * toleranceSq;
    }

    inline QPointF fallbackSourcePoint() const {
        return m_srcBase + QPointF(0.5 * m_xCoeff, 0.5 * m_yCoeff);
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
                        nu = qBound(qreal(0.0), nu, qreal(1.0));
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

    qreal m_qA {0.0}; // quadratic equation A coeff
    qreal m_qB_const {0.0}; // quadratic equation B coeff, const part
    qreal m_qB_varX {0.0}; // quadratic equation B coeff, X-dep part
    qreal m_qB_varY {0.0}; // quadratic equation B coeff, Y-dep part
    qreal m_qC_varX {0.0}; // quadratic equation C coeff, X-dep part
    qreal m_qC_varY {0.0}; // quadratic equation C coeff, Y-dep part
    qreal m_qD_div {0.0}; // inverted divisor of the quadratic equation solution
    qreal m_px {0.0}; // saved relative X coordinate
    qreal m_py {0.0}; // saved relative Y coordinate

    QPointF m_srcBase;
    QPointF m_dstBase;
    qreal m_xCoeff {0.0};
    qreal m_yCoeff {0.0};
};

#endif /* __KIS_FOUR_POINT_INTERPOLATOR_BACKWARD_H */
