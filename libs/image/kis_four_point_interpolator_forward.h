/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FOUR_POINT_INTERPOLATOR_FORWARD_H
#define __KIS_FOUR_POINT_INTERPOLATOR_FORWARD_H

#include <QPolygon>
#include <QPointF>



/**
 *    A-----B         The polygons must be initialized in this order:
 *    |     |
 *    |     |         polygon << A << B << D << C;
 *    C-----D
 */

class KisFourPointInterpolatorForward
{
public:
    KisFourPointInterpolatorForward(const QPolygonF &srcPolygon, const QPolygonF &dstPolygon) {
        m_srcBase = srcPolygon[0];
        m_dstBase = dstPolygon[0];

        m_h0 = dstPolygon[1] - dstPolygon[0]; // BA
        m_h1 = dstPolygon[2] - dstPolygon[3]; // DC

        m_v0 = dstPolygon[3] - dstPolygon[0]; // CA

        m_forwardCoeffX = 1.0 / (srcPolygon[1].x() - srcPolygon[0].x());
        m_forwardCoeffY = 1.0 / (srcPolygon[3].y() - srcPolygon[0].y());

        m_xProp = 0;
        m_yProp = 0;
    }

    inline QPointF map(const QPointF &pt) {
        setX(pt.x());
        setY(pt.y());
        return getValue();
    }

    inline void setX(qreal x) {
        qreal diff = x - m_srcBase.x();
        m_xProp = diff * m_forwardCoeffX;
    }

    inline void setY(qreal y) {
        qreal diff = y - m_srcBase.y();
        m_yProp = diff * m_forwardCoeffY;
    }

    inline QPointF getValue() const {
        QPointF dstPoint = m_dstBase +
            m_yProp * m_v0 +
            m_xProp * (m_yProp * m_h1 + (1.0 - m_yProp) * m_h0);

        return dstPoint;
    }

private:
    QPointF m_srcBase;
    QPointF m_dstBase;

    QPointF m_h0;
    QPointF m_h1;
    QPointF m_v0;

    qreal m_forwardCoeffX;
    qreal m_forwardCoeffY;

    qreal m_xProp;
    qreal m_yProp;
};

#endif /* __KIS_FOUR_POINT_INTERPOLATOR_FORWARD_H */
