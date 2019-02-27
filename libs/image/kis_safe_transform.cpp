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

#include "kis_safe_transform.h"

#include <QTransform>
#include <QLineF>
#include <QPolygonF>


#include "kis_debug.h"
#include "kis_algebra_2d.h"



struct Q_DECL_HIDDEN KisSafeTransform::Private
{
    bool needsClipping = true;

    QRect bounds;
    QTransform forwardTransform;
    QTransform backwardTransform;

    QPolygonF srcClipPolygon;
    QPolygonF dstClipPolygon;

    bool getHorizon(const QTransform &t, QLineF *horizon) {
        static const qreal eps = 1e-10;

        QPointF vanishingX(t.m11() / t.m13(), t.m12() / t.m13());
        QPointF vanishingY(t.m21() / t.m23(), t.m22() / t.m23());

        if (qAbs(t.m13()) < eps && qAbs(t.m23()) < eps) {
            *horizon = QLineF();
            return false;
        } else if (qAbs(t.m23()) < eps) {
            QPointF diff = t.map(QPointF(0.0, 10.0)) - t.map(QPointF());
            vanishingY = vanishingX + diff;
        } else if (qAbs(t.m13()) < eps) {
            QPointF diff = t.map(QPointF(10.0, 0.0)) - t.map(QPointF());
            vanishingX = vanishingY + diff;
        }

        *horizon = QLineF(vanishingX, vanishingY);
        return true;
    }

    qreal getCrossSign(const QLineF &horizon, const QRectF &rc) {
        if (rc.isEmpty()) return 1.0;

        QPointF diff = horizon.p2() - horizon.p1();
        return KisAlgebra2D::signPZ(KisAlgebra2D::crossProduct(diff, rc.center() - horizon.p1()));
    }

    QPolygonF getCroppedPolygon(const QLineF &baseHorizon, const QRect &rc, const qreal crossCoeff) {
        if (rc.isEmpty()) return QPolygonF();

        QRectF boundsRect(rc);
        QPolygonF polygon(boundsRect);
        QPolygonF result;

        // calculate new (offset) horizon to avoid infinity
        const qreal offsetLength = 10.0;
        const QPointF horizonOffset = offsetLength * crossCoeff *
            KisAlgebra2D::rightUnitNormal(baseHorizon.p2() - baseHorizon.p1());

        const QLineF horizon = baseHorizon.translated(horizonOffset);

        // base vectors to calculate the side of the horizon
        const QPointF &basePoint = horizon.p1();
        const QPointF horizonVec = horizon.p2() - basePoint;


        // iteration
        QPointF prevPoint = polygon[polygon.size() - 1];
        qreal prevCross = crossCoeff * KisAlgebra2D::crossProduct(horizonVec, prevPoint - basePoint);

        for (int i = 0; i < polygon.size(); i++) {
            const QPointF &pt = polygon[i];

            qreal cross = crossCoeff * KisAlgebra2D::crossProduct(horizonVec, pt - basePoint);

            if ((cross >= 0 && prevCross >= 0) || (cross == 0 && prevCross < 0)) {
                result << pt;
            } else if (cross * prevCross < 0) {
                QPointF intersection;
                QLineF edge(prevPoint, pt);
                QLineF::IntersectType intersectionType =
                    horizon.intersect(edge, &intersection);

                KIS_ASSERT_RECOVER_NOOP(intersectionType != QLineF::NoIntersection);

                result << intersection;

                if (cross > 0) {
                    result << pt;
                }
            }

            prevPoint = pt;
            prevCross = cross;
        }

        if (result.size() > 0 && !result.isClosed()) {
            result << result.first();
        }

        return result;
    }

};

KisSafeTransform::KisSafeTransform(const QTransform &transform,
                                   const QRect &bounds,
                                   const QRect &srcInterestRect)
    : m_d(new Private)
{
    m_d->bounds = bounds;

    m_d->forwardTransform = transform;
    m_d->backwardTransform = transform.inverted();

    m_d->needsClipping = transform.type() > QTransform::TxShear;

    if (m_d->needsClipping) {
        m_d->srcClipPolygon = QPolygonF(QRectF(m_d->bounds));
        m_d->dstClipPolygon = QPolygonF(QRectF(m_d->bounds));

        qreal crossCoeff = 1.0;

        QLineF srcHorizon;
        if (m_d->getHorizon(m_d->backwardTransform, &srcHorizon)) {
            crossCoeff = m_d->getCrossSign(srcHorizon, srcInterestRect);
            m_d->srcClipPolygon = m_d->getCroppedPolygon(srcHorizon, m_d->bounds, crossCoeff);
        }

        QLineF dstHorizon;
        if (m_d->getHorizon(m_d->forwardTransform, &dstHorizon)) {
            crossCoeff = m_d->getCrossSign(dstHorizon, mapRectForward(srcInterestRect));
            m_d->dstClipPolygon = m_d->getCroppedPolygon(dstHorizon, m_d->bounds, crossCoeff);
        }
    }
}

KisSafeTransform::~KisSafeTransform()
{
}

QPolygonF KisSafeTransform::srcClipPolygon() const
{
    return m_d->srcClipPolygon;
}

QPolygonF KisSafeTransform::dstClipPolygon() const
{
    return m_d->dstClipPolygon;
}

QPolygonF KisSafeTransform::mapForward(const QPolygonF &p)
{
    QPolygonF poly;

    if (!m_d->needsClipping) {
        poly = m_d->forwardTransform.map(p);
    } else {
        poly = m_d->srcClipPolygon.intersected(p);
        poly = m_d->forwardTransform.map(poly).intersected(QRectF(m_d->bounds));
    }

    return poly;
}

QPolygonF KisSafeTransform::mapBackward(const QPolygonF &p)
{
    QPolygonF poly;

    if (!m_d->needsClipping) {
        poly = m_d->backwardTransform.map(p);
    } else {
        poly = m_d->dstClipPolygon.intersected(p);
        poly = m_d->backwardTransform.map(poly).intersected(QRectF(m_d->bounds));
    }

    return poly;
}

QRectF KisSafeTransform::mapRectForward(const QRectF &rc)
{
    return mapForward(rc).boundingRect();
}

QRectF KisSafeTransform::mapRectBackward(const QRectF &rc)
{
    return mapBackward(rc).boundingRect();
}

QRect KisSafeTransform::mapRectForward(const QRect &rc)
{
    return mapRectForward(QRectF(rc)).toAlignedRect();
}

QRect KisSafeTransform::mapRectBackward(const QRect &rc)
{
    return mapRectBackward(QRectF(rc)).toAlignedRect();
}

