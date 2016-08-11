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

#include "kis_algebra_2d.h"

#include <QPainterPath>
#include <kis_debug.h>
#include "krita_utils.h"


#define SANITY_CHECKS

namespace KisAlgebra2D {

void adjustIfOnPolygonBoundary(const QPolygonF &poly, int polygonDirection, QPointF *pt)
{
    const int numPoints = poly.size();
    for (int i = 0; i < numPoints; i++) {
        int nextI = i + 1;
        if (nextI >= numPoints) {
            nextI = 0;
        }

        const QPointF &p0 = poly[i];
        const QPointF &p1 = poly[nextI];

        QPointF edge = p1 - p0;

        qreal cross = crossProduct(edge, *pt - p0)
            / (0.5 * edge.manhattanLength());

        if (cross < 1.0 &&
            isInRange(pt->x(), p0.x(), p1.x()) &&
            isInRange(pt->y(), p0.y(), p1.y())) {

            QPointF salt = 1.0e-3 * inwardUnitNormal(edge, polygonDirection);

            QPointF adjustedPoint = *pt + salt;

            // in case the polygon is self-intersecting, polygon direction
            // might not help
            if (kisDistanceToLine(adjustedPoint, QLineF(p0, p1)) < 1e-4) {
                adjustedPoint = *pt - salt;

#ifdef SANITY_CHECKS
                if (kisDistanceToLine(adjustedPoint, QLineF(p0, p1)) < 1e-4) {
                    dbgKrita << ppVar(*pt);
                    dbgKrita << ppVar(adjustedPoint);
                    dbgKrita << ppVar(QLineF(p0, p1));
                    dbgKrita << ppVar(salt);

                    dbgKrita << ppVar(poly.containsPoint(*pt, Qt::OddEvenFill));

                    dbgKrita << ppVar(kisDistanceToLine(*pt, QLineF(p0, p1)));
                    dbgKrita << ppVar(kisDistanceToLine(adjustedPoint, QLineF(p0, p1)));
                }

                *pt = adjustedPoint;

                KIS_ASSERT_RECOVER_NOOP(kisDistanceToLine(*pt, QLineF(p0, p1)) > 1e-4);
#endif /* SANITY_CHECKS */
            }
        }
    }
}

QPointF transformAsBase(const QPointF &pt, const QPointF &base1, const QPointF &base2) {
    qreal len1 = norm(base1);
    if (len1 < 1e-5) return pt;
    qreal sin1 = base1.y() / len1;
    qreal cos1 = base1.x() / len1;

    qreal len2 = norm(base2);
    if (len2 < 1e-5) return QPointF();
    qreal sin2 = base2.y() / len2;
    qreal cos2 = base2.x() / len2;

    qreal sinD = sin2 * cos1 - cos2 * sin1;
    qreal cosD = cos1 * cos2 + sin1 * sin2;
    qreal scaleD = len2 / len1;

    QPointF result;
    result.rx() = scaleD * (pt.x() * cosD - pt.y() * sinD);
    result.ry() = scaleD * (pt.x() * sinD + pt.y() * cosD);

    return result;
}

qreal angleBetweenVectors(const QPointF &v1, const QPointF &v2)
{
    qreal a1 = std::atan2(v1.y(), v1.x());
    qreal a2 = std::atan2(v2.y(), v2.x());

    return a2 - a1;
}

QPainterPath smallArrow()
{
    QPainterPath p;

    p.moveTo(5, 2);
    p.lineTo(-3, 8);
    p.lineTo(-5, 5);
    p.lineTo( 2, 0);
    p.lineTo(-5,-5);
    p.lineTo(-3,-8);
    p.lineTo( 5,-2);
    p.arcTo(QRectF(3, -2, 4, 4), 90, -180);

    return p;
}

template <class Point, class Rect>
inline Point ensureInRectImpl(Point pt, const Rect &bounds)
{
    if (pt.x() > bounds.right()) {
        pt.rx() = bounds.right();
    } else if (pt.x() < bounds.left()) {
        pt.rx() = bounds.left();
    }

    if (pt.y() > bounds.bottom()) {
        pt.ry() = bounds.bottom();
    } else if (pt.y() < bounds.top()) {
        pt.ry() = bounds.top();
    }

    return pt;
}

QPoint ensureInRect(QPoint pt, const QRect &bounds)
{
    return ensureInRectImpl(pt, bounds);
}

QPointF ensureInRect(QPointF pt, const QRectF &bounds)
{
    return ensureInRectImpl(pt, bounds);
}

QRect ensureRectNotSmaller(QRect rc, const QSize &size)
{
    if (rc.width() < size.width() ||
        rc.height() < size.height()) {

        int width = qMax(rc.width(), size.width());
        int height = qMax(rc.height(), size.height());

        rc = QRect(rc.topLeft(), QSize(width, height));
    }

    return rc;
}

bool intersectLineRect(QLineF &line, const QRect rect)
{
    QPointF pt1 = QPointF(), pt2 = QPointF();
    QPointF tmp;

    if (line.intersect(QLineF(rect.topLeft(), rect.topRight()), &tmp) != QLineF::NoIntersection) {
        if (tmp.x() >= rect.left() && tmp.x() <= rect.right()) {
            pt1 = tmp;
        }
    }

    if (line.intersect(QLineF(rect.topRight(), rect.bottomRight()), &tmp) != QLineF::NoIntersection) {
        if (tmp.y() >= rect.top() && tmp.y() <= rect.bottom()) {
            if (pt1.isNull()) pt1 = tmp;
            else pt2 = tmp;
        }
    }
    if (line.intersect(QLineF(rect.bottomRight(), rect.bottomLeft()), &tmp) != QLineF::NoIntersection) {
        if (tmp.x() >= rect.left() && tmp.x() <= rect.right()) {
            if (pt1.isNull()) pt1 = tmp;
            else pt2 = tmp;
        }
    }
    if (line.intersect(QLineF(rect.bottomLeft(), rect.topLeft()), &tmp) != QLineF::NoIntersection) {
        if (tmp.y() >= rect.top() && tmp.y() <= rect.bottom()) {
            if (pt1.isNull()) pt1 = tmp;
            else pt2 = tmp;
        }
    }

    if (pt1.isNull() || pt2.isNull()) return false;

    // Attempt to retain polarity of end points
    if ((line.x1() < line.x2()) != (pt1.x() > pt2.x()) || (line.y1() < line.y2()) != (pt1.y() > pt2.y())) {
        tmp = pt1;
        pt1 = pt2;
        pt2 = tmp;
    }

    line.setP1(pt1);
    line.setP2(pt2);

    return true;
}

QRectF cutOffRect(const QRectF &rc, const KisAlgebra2D::RightHalfPlane &p)
{
    QVector<QPointF> points;

    const QLineF cutLine = p.getLine();

    points << rc.topLeft();
    points << rc.topRight();
    points << rc.bottomRight();
    points << rc.bottomLeft();

    QPointF p1 = points[3];
    bool p1Valid = p.pos(p1) >= 0;

    QVector<QPointF> resultPoints;

    for (int i = 0; i < 4; i++) {
        const QPointF p2 = points[i];
        const bool p2Valid = p.pos(p2) >= 0;

        if (p1Valid != p2Valid) {
            QPointF intersection;
            cutLine.intersect(QLineF(p1, p2), &intersection);
            resultPoints << intersection;
        }

        if (p2Valid) {
            resultPoints << p2;
        }

        p1 = p2;
        p1Valid = p2Valid;
    }

    return KritaUtils::approximateRectFromPoints(resultPoints);
}

int quadraticEquation(qreal a, qreal b, qreal c, qreal *x1, qreal *x2)
{
    int numSolutions = 0;

    const qreal D = pow2(b) - 4 * a * c;

    if (D < 0) {
        return 0;
    } else if (qFuzzyCompare(D, 0)) {
        *x1 = -b / (2 * a);
        numSolutions = 1;
    } else {
        const qreal sqrt_D = std::sqrt(D);

        *x1 = (-b + sqrt_D) / (2 * a);
        *x2 = (-b - sqrt_D) / (2 * a);
        numSolutions = 2;
    }

    return numSolutions;
}

QVector<QPointF> intersectTwoCircles(const QPointF &c1, qreal r1,
                                     const QPointF &c2, qreal r2)
{
    QVector<QPointF> points;
    if (kisSquareDistance(c1, c2) > pow2(r1 + r2)) return points;

    const qreal x_k = c1.x();
    const qreal x_kp1 = c2.x();
    const qreal y_k = c1.y();
    const qreal y_kp1 = c2.y();

    const qreal F2 =
        0.5 * (pow2(x_kp1) - pow2(x_k) +
               pow2(y_kp1) - pow2(y_k) - pow2(r1) + pow2(r2));

    const QPointF diff = c2 - c1;

    if (qFuzzyCompare(diff.y(), 0)) {
        qreal x = F2 / diff.x();
        qreal y1, y2;
        int result = KisAlgebra2D::quadraticEquation(
            1, -2 * y_k,
            pow2(x) + pow2(x_k) -
            2 * x * x_k + pow2(y_k) - pow2(r2),
            &y1, &y2);

        KIS_ASSERT_RECOVER(result > 0) { return points; }
        if (result == 1) {
            points << QPointF(x, y1);
        } else if (result == 2) {
            KisAlgebra2D::RightHalfPlane p(c1, c2);

            QPointF p1(x, y1);
            QPointF p2(x, y2);

            if (p.pos(p1) >= 0) {
                points << p1;
                points << p2;
            } else {
                points << p2;
                points << p1;
            }
        }
    } else {
        const qreal A = diff.x() / diff.y();
        const qreal C = F2 / diff.y();

        qreal x1, x2;
        int result = KisAlgebra2D::quadraticEquation(
            1 + pow2(A), -2 * (x_k + A * C - y_k * A),
            pow2(x_k) + pow2(y_k) + pow2(C) - 2 * y_k * C - pow2(r1),
            &x1, &x2);

        KIS_ASSERT_RECOVER(result > 0) { return points; }

        if (result == 1) {
            points << QPointF(x1, C - x1 * A);
        } else if (result == 2) {
            KisAlgebra2D::RightHalfPlane p(c1, c2);

            QPointF p1(x1, C - x1 * A);
            QPointF p2(x2, C - x2 * A);

            if (p.pos(p1) >= 0) {
                points << p1;
                points << p2;
            } else {
                points << p2;
                points << p1;
            }
        }
    }

    return points;
}

}
