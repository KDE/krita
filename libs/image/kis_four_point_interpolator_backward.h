/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FOUR_POINT_INTERPOLATOR_BACKWARD_H
#define __KIS_FOUR_POINT_INTERPOLATOR_BACKWARD_H

#include <QPolygon>
#include <QPointF>

#include "kis_global.h"
#include "kis_algebra_2d.h"

//#define FPIB_DEBUG


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
#ifdef FPIB_DEBUG
        m_dbgSrcPolygon = srcPolygon;
        m_dbgDstPolygon = dstPolygon;
#endif

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
#ifdef FPIB_DEBUG
        m_dbgOrigX = x;
#endif
        x -= m_dstBase.x();

        m_qB_varX = - x * m_d.y();
        m_qC_varX = - x * m_a.y();
        m_px = x;
    }

    inline void setY(qreal y) {
#ifdef FPIB_DEBUG
        m_dbgOrigY = y;
#endif
        y -= m_dstBase.y();

        m_qB_varY = y * m_d.x();
        m_qC_varY = y * m_a.x();
        m_py = y;
    }


    inline QPointF getValue() const {
        static const qreal eps = 1e-6; // pixels in Krita only get to 32-33k in every direction


        qreal qB = m_qB_const + m_qB_varX + m_qB_varY;
        qreal qC = m_qC_varX + m_qC_varY;

        qreal nu = 0.0;
        qreal D = 0.0;
        qreal sqrtD = 0.0;

        bool dontCheckOtherNu = false;

        if (qAbs(m_qA) < eps) {
            nu = -qC / qB;
            dontCheckOtherNu = true;
        } else {
            D = pow2(qB) - 4 * m_qA * qC;
            if (D > 0.0) {
                sqrtD = std::sqrt(D);
                nu = (-qB - sqrtD) * m_qD_div;
            } else {
                nu = 0.0;
                dontCheckOtherNu = true;
            }
        }

        qreal nu1 = nu;

        qreal xDenomNu1 = xBasedDenom(nu1);
        qreal xMu1 = xBasedMu(nu1, xDenomNu1);

        bool goodNu1 = inGoodRange(nu1);

        if (goodNu1 && inGoodRange(xMu1)) {
            return getResult(nu1, xMu1);
        }

        qreal yDenomNu1 = yBasedDenom(nu1);
        qreal yMu1 = yBasedMu(nu1, yDenomNu1);


        if (goodNu1 && inGoodRange(yMu1)) {
            return getResult(nu1, yMu1);
        }

        qreal nu2 = nu1, xDenomNu2 = xDenomNu1, xMu2 = xMu1, yDenomNu2 = yDenomNu1, yMu2 = yMu1;

        if (!dontCheckOtherNu) {
            nu2 = (-qB + sqrtD) * m_qD_div;

            xDenomNu2 = xBasedDenom(nu2);
            xMu2 = xBasedMu(nu2, xDenomNu2);

            bool goodNu2 = inGoodRange(nu2);

            if (goodNu2 && inGoodRange(xMu2)) {
                return getResult(nu2, xMu2);
            }

            yDenomNu2 = yBasedDenom(nu2);
            yMu2 = xBasedMu(nu2, yDenomNu2);

            if (goodNu2 && inGoodRange(yMu2)) {
                return getResult(nu2, yMu2);
            }
        }


        const int count = 4;
        qreal denoms[count] = {xDenomNu1, yDenomNu1, xDenomNu2, yDenomNu2};
        qreal mus[count] = {xMu1, yMu1, xMu2, yMu2};
        qreal nus[count] = {nu1, nu1, nu2, nu2};
        QPointF results[count];
#ifdef FPIB_DEBUG
        qCritical() << "For point: x = " << m_dbgOrigX << " y = " << m_dbgOrigY << " | src polygon = " << m_dbgSrcPolygon << " | dst polygon = " << m_dbgDstPolygon;
        for (int i = 0; i < count; i++) {
            qCritical() << "for i = " << i << ": denoms[i] = " << denoms[i] << "mus[i] = " << mus[i] << "nus[i] = " << nus[i] << "result: " << getResult(nus[i], mus[i]);
        }
#endif

        int bestI = -1;
        qreal distanceFromCenter = 0.0;
        QPointF center = fallbackSourcePoint();

        int meaningfulCount = dontCheckOtherNu ? 2 : 4;
        for (int i = 0; i < meaningfulCount; i++) {

            if (qAbs(denoms[i]) < eps) {
                continue;
            }

            results[i] = getResult(nus[i], mus[i]);
            qreal dist = KisAlgebra2D::normSquared(center - results[i]);
            if (bestI < 0 || dist < distanceFromCenter) {
                distanceFromCenter = dist;
                bestI = i;
            }
        }
#ifdef FPIB_DEBUG
        if (bestI < 0) {
            for (int i = 0; i < count; i++) {
                qCritical() << "for i = " << i << "denom=" << denoms[i] << "mu=" << mus[i] << "nu=" << nus[i] << "result: " << getResult(nus[i], mus[i]);
            }
            qCritical() << "Center point: " << center;
            qCritical() << "dont check other nu: " << dontCheckOtherNu;

        }
#endif

        // there won't be any sane result here in case we didn't find any bestI
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(bestI >= 0, center);

        return results[bestI];
    }

private:


    inline qreal xBasedDenom(qreal nu) const {
        return m_a.x() + nu * m_d.x();
    }

    inline qreal yBasedDenom(qreal nu) const {
        return m_a.y() + nu * m_d.y();
    }

    inline qreal xBasedMu(qreal nu, qreal xBasedDenominator) const {
        return (m_px - nu * m_c.x()) / xBasedDenominator;
    }

    inline qreal yBasedMu(qreal nu, qreal yBasedDenominator) const {
        return (m_py - nu * m_c.y()) / yBasedDenominator;
    }

    inline QPointF getResult(qreal nu, qreal mu) const {
        return m_srcBase + QPointF(mu * m_xCoeff, nu * m_yCoeff);
    }

    inline bool inGoodRange(qreal value) const {
        return value >= 0.0 && value <= 1.0;
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

#ifdef FPIB_DEBUG
    QPolygonF m_dbgDstPolygon;
    QPolygonF m_dbgSrcPolygon;

    qreal m_dbgOrigX;
    qreal m_dbgOrigY;
#endif


};

#endif /* __KIS_FOUR_POINT_INTERPOLATOR_BACKWARD_H */
