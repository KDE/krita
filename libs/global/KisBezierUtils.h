/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISBEZIERUTILS_H
#define KISBEZIERUTILS_H

#include "kritaglobal_export.h"

#include <kis_algebra_2d.h>


namespace KisBezierUtils
{
using KisAlgebra2D::lerp;

inline QPointF bezierCurve(const QPointF p0,
                           const QPointF p1,
                           const QPointF p2,
                           const QPointF p3,
                           qreal t)
{
    const qreal t_2 = pow2(t);
    const qreal t_3 = t_2 * t;
    const qreal t_inv = 1.0 - t;
    const qreal t_inv_2 = pow2(t_inv);
    const qreal t_inv_3 = t_inv_2 * t_inv;

    return
        t_inv_3 * p0 +
        3 * t_inv_2 * t * p1 +
        3 * t_inv * t_2 * p2 +
        t_3 * p3;

}

inline QPointF bezierCurve(const QList<QPointF> &points,
                           qreal t)
{
    QPointF result;

    if (points.size() == 2) {
        result = lerp(points.first(), points.last(), t);
    } else if (points.size() == 3) {
        result = bezierCurve(points[0], points[1], points[1], points[2], t);
    } else if (points.size() == 4) {
        result = bezierCurve(points[0], points[1], points[2], points[3], t);
    } else {
        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "Unsupported number of bezier control points");
    }

    return result;
}


inline QPointF bezierCurveDeriv(const QPointF p0,
                                const QPointF p1,
                                const QPointF p2,
                                const QPointF p3,
                                qreal t)
{
    const qreal t_2 = pow2(t);
    const qreal t_inv = 1.0 - t;
    const qreal t_inv_2 = pow2(t_inv);

    return
        3 * t_inv_2 * (p1 - p0) +
        6 * t_inv * t * (p2 - p1) +
        3 * t_2 * (p3 - p2);
}

inline QPointF bezierCurveDeriv2(const QPointF p0,
                                 const QPointF p1,
                                 const QPointF p2,
                                 const QPointF p3,
                                 qreal t)
{
    const qreal t_inv = 1.0 - t;

    return
        6 * t_inv * (p2 - 2 * p1 + p0) +
        6 * t * (p3 - 2 * p2 + p1);
}


inline void deCasteljau(const QPointF &q0,
                        const QPointF &q1,
                        const QPointF &q2,
                        const QPointF &q3,
                        qreal t,
                        QPointF *p0,
                        QPointF *p1,
                        QPointF *p2,
                        QPointF *p3,
                        QPointF *p4)
{
    QPointF q[4];

    q[0] = q0;
    q[1] = q1;
    q[2] = q2;
    q[3] = q3;

    // points of the new segment after the split point
    QPointF p[3];

    // the De Casteljau algorithm
    for (unsigned short j = 1; j <= 3; ++j) {
        for (unsigned short i = 0; i <= 3 - j; ++i) {
            q[i] = (1.0 - t) * q[i] + t * q[i + 1];
        }
        p[j - 1] = q[0];
    }

    *p0 = p[0];
    *p1 = p[1];
    *p2 = p[2];
    *p3 = q[1];
    *p4 = q[2];
}

inline bool isLinearSegmentByDerivatives(const QPointF &p0, const QPointF &d0,
                                         const QPointF &p1, const QPointF &d1,
                                         const qreal eps = 1e-4)
{
    const QPointF diff = p1 - p0;
    const qreal dist = KisAlgebra2D::norm(diff);

    const qreal normCoeff = 1.0 / 3.0 / dist;

    const qreal offset1 =
        normCoeff * qAbs(KisAlgebra2D::crossProduct(diff, d0));
    if (offset1 > eps) return false;

    const qreal offset2 =
        normCoeff * qAbs(KisAlgebra2D::crossProduct(diff, d1));
    if (offset2 > eps) return false;

    return true;
}

inline bool isLinearSegmentByControlPoints(const QPointF &p0, const QPointF &p1,
                                           const QPointF &p2, const QPointF &p3,
                                           const qreal eps = 1e-4)
{
    return isLinearSegmentByDerivatives(p0, (p1 - p0) * 3.0, p3, (p3 - p2) * 3.0, eps);
}

inline int bezierDegree(const QPointF p0,
                        const QPointF p1,
                        const QPointF p2,
                        const QPointF p3)
{
    const qreal eps = 1e-4;

    int degree = 3;

    if (isLinearSegmentByControlPoints(p0, p1, p2, p3, eps)) {
        degree = 1;
    } else if (KisAlgebra2D::fuzzyPointCompare(p1, p2, eps)) {
        degree = 2;
    }

    return degree;
}

inline void splitBezierCurve(const QPointF &q0,
                             const QPointF &q1,
                             const QPointF &q2,
                             const QPointF &q3,
                             qreal t,
                             QPointF *p0,
                             QPointF *p1,
                             QPointF *p2,
                             QPointF *p3,
                             QPointF *p4)
{
    const qreal eps = 1e-4;

    if (isLinearSegmentByControlPoints(q0, q1, q2, q3, eps)) {
        *p2 = lerp(q0, q3, t);
        *p0 = lerp(q0, *p2, 0.1);
        *p1 = lerp(q0, *p2, 0.9);
        *p3 = lerp(*p2, q3, 0.1);
        *p4 = lerp(*p2, q3, 0.9);
    } else {
        deCasteljau(q0, q1, q2, q3, t, p0, p1, p2, p3, p4);
    }
}

KRITAGLOBAL_EXPORT
QVector<qreal> linearizeCurve(const QPointF p0,
                              const QPointF p1,
                              const QPointF p2,
                              const QPointF p3,
                              const qreal eps);
KRITAGLOBAL_EXPORT
QVector<qreal> mergeLinearizationSteps(const QVector<qreal> &a, const QVector<qreal> &b);

KRITAGLOBAL_EXPORT
qreal nearestPoint(const QList<QPointF> controlPoints, const QPointF &point);

KRITAGLOBAL_EXPORT
int controlPolygonZeros(const QList<QPointF> &controlPoints);

/**
 * @brief calculates local (u,v) coordinates of the patch corrresponding to \p globalPoint
 * @param points control points as the layouted in KisBezierPatch
 * @param globalPoint point in global coordinates
 * @return
 */
KRITAGLOBAL_EXPORT
QPointF calculateLocalPos(const std::array<QPointF, 12> &points,
                          const QPointF &globalPoint);

}

#endif // KISBEZIERUTILS_H
