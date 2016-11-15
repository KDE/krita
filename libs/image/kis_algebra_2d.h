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

#ifndef __KIS_ALGEBRA_2D_H
#define __KIS_ALGEBRA_2D_H

#include <QPoint>
#include <QPointF>
#include <QVector>
#include <QPolygonF>
#include <cmath>
#include <kis_global.h>
#include <kritaimage_export.h>

class QPainterPath;

namespace KisAlgebra2D {

template <class T>
struct PointTypeTraits
{
};

template <>
struct PointTypeTraits<QPoint>
{
    typedef int value_type;
    typedef qreal calculation_type;
};

template <>
struct PointTypeTraits<QPointF>
{
    typedef qreal value_type;
    typedef qreal calculation_type;
};


template <class T>
typename PointTypeTraits<T>::value_type dotProduct(const T &a, const T &b)
{
    return a.x() * b.x() + a.y() * b.y();
}

template <class T>
typename PointTypeTraits<T>::value_type crossProduct(const T &a, const T &b)
{
    return a.x() * b.y() - a.y() * b.x();
}

template <class T>
qreal norm(const T &a)
{
    return std::sqrt(pow2(a.x()) + pow2(a.y()));
}

template <class Point>
Point normalize(const Point &a)
{
    const qreal length = norm(a);
    return (1.0 / length) * a;
}

/**
 * Usual sign() function with positive zero
 */
template <typename T>
T signPZ(T x) {
    return x >= T(0) ? T(1) : T(-1);
}

/**
 * Usual sign() function with zero returning zero
 */
template <typename T>
T signZZ(T x) {
    return x == T(0) ? T(0) : x > T(0) ? T(1) : T(-1);
}

/**
 * Copies the sign of \p y into \p x and returns the result
 */
template <typename T>
    inline T copysign(T x, T y) {
    T strippedX = qAbs(x);
    return y >= T(0) ? strippedX : -strippedX;
}

template <class T>
T leftUnitNormal(const T &a)
{
    T result = a.x() != 0 ? T(-a.y() / a.x(), 1) : T(-1, 0);
    qreal length = norm(result);
    result *= (crossProduct(a, result) >= 0 ? 1 : -1) / length;

    return -result;
}

template <class T>
T rightUnitNormal(const T &a)
{
    return -leftUnitNormal(a);
}

template <class T>
T inwardUnitNormal(const T &a, int polygonDirection)
{
    return polygonDirection * leftUnitNormal(a);
}

/**
 * \return 1 if the polygon is counterclockwise
 *        -1 if the polygon is clockwise
 *
 * Note: the sign is flipped because our 0y axis
 *       is reversed
 */
template <class T>
int polygonDirection(const QVector<T> &polygon) {

    typename PointTypeTraits<T>::value_type doubleSum = 0;

    const int numPoints = polygon.size();
    for (int i = 1; i <= numPoints; i++) {
        int prev = i - 1;
        int next = i == numPoints ? 0 : i;

        doubleSum +=
            (polygon[next].x() - polygon[prev].x()) *
            (polygon[next].y() + polygon[prev].y());
    }

    return doubleSum >= 0 ? 1 : -1;
}

template <typename T>
bool isInRange(T x, T a, T b) {
    T length = qAbs(a - b);
    return qAbs(x - a) <= length && qAbs(x - b) <= length;
}

void KRITAIMAGE_EXPORT adjustIfOnPolygonBoundary(const QPolygonF &poly, int polygonDirection, QPointF *pt);

/**
 * Let \p pt, \p base1 are two vectors. \p base1 is uniformly scaled
 * and then rotated into \p base2 using transformation matrix S *
 * R. The function applies the same transformation to \pt and returns
 * the result.
 **/
QPointF KRITAIMAGE_EXPORT transformAsBase(const QPointF &pt, const QPointF &base1, const QPointF &base2);

qreal KRITAIMAGE_EXPORT angleBetweenVectors(const QPointF &v1, const QPointF &v2);

namespace Private {
    inline void resetEmptyRectangle(const QPoint &pt, QRect *rc) {
        *rc = QRect(pt, QSize(1, 1));
    }

    inline void resetEmptyRectangle(const QPointF &pt, QRectF *rc) {
        static const qreal eps = 1e-10;
        *rc = QRectF(pt, QSizeF(eps, eps));
    }
}

template <class Point, class Rect>
inline void accumulateBounds(const Point &pt, Rect *bounds)
{
    if (bounds->isEmpty()) {
        Private::resetEmptyRectangle(pt, bounds);
    }

    if (pt.x() > bounds->right()) {
        bounds->setRight(pt.x());
    }

    if (pt.x() < bounds->left()) {
        bounds->setLeft(pt.x());
    }

    if (pt.y() > bounds->bottom()) {
        bounds->setBottom(pt.y());
    }

    if (pt.y() < bounds->top()) {
        bounds->setTop(pt.y());
    }
}

template <class Point, class Rect>
inline Point clampPoint(Point pt, const Rect &bounds)
{
    if (pt.x() > bounds.right()) {
        pt.rx() = bounds.right();
    }

    if (pt.x() < bounds.left()) {
        pt.rx() = bounds.left();
    }

    if (pt.y() > bounds.bottom()) {
        pt.ry() = bounds.bottom();
    }

    if (pt.y() < bounds.top()) {
        pt.ry() = bounds.top();
    }

    return pt;
}

QPainterPath KRITAIMAGE_EXPORT smallArrow();

/**
 * Multiply width and height of \p rect by \p coeff keeping the
 * center of the rectangle pinned
 */
template <class Rect>
Rect blowRect(const Rect &rect, qreal coeff)
{
    typedef decltype(rect.x()) CoordType;

    CoordType w = rect.width() * coeff;
    CoordType h = rect.height() * coeff;

    return rect.adjusted(-w, -h, w, h);
}

QPoint KRITAIMAGE_EXPORT ensureInRect(QPoint pt, const QRect &bounds);
QPointF KRITAIMAGE_EXPORT ensureInRect(QPointF pt, const QRectF &bounds);

QRect KRITAIMAGE_EXPORT ensureRectNotSmaller(QRect rc, const QSize &size);

/**
 * Attempt to intersect a line to the area of the a rectangle.
 *
 * If the line intersects the rectange, it will be modified to represent the intersecting line segment and true is returned.
 * If the line does not intersect the area, it remains unmodified and false will be returned.
 *
 * @param segment
 * @param area
 * @return true if successful
 */
bool KRITAIMAGE_EXPORT intersectLineRect(QLineF &line, const QRect rect);


template <class Point>
inline Point abs(const Point &pt) {
    return Point(qAbs(pt.x()), qAbs(pt.y()));
}


class RightHalfPlane {
public:

    RightHalfPlane(const QPointF &a, const QPointF &b)
        : m_a(a), m_p(b - a), m_norm_p_inv(1.0 / norm(m_p))
    {
    }

    RightHalfPlane(const QLineF &line)
        : RightHalfPlane(line.p1(), line.p2())
    {
    }

    qreal valueSq(const QPointF &pt) const {
        const qreal val = value(pt);
        return signZZ(val) * pow2(val);
    }

    qreal value(const QPointF &pt) const {
        return crossProduct(m_p, pt - m_a) * m_norm_p_inv;
    }

    int pos(const QPointF &pt) const {
        return signZZ(value(pt));
    }

    QLineF getLine() const {
        return QLineF(m_a, m_a + m_p);
    }

private:
    const QPointF m_a;
    const QPointF m_p;
    const qreal m_norm_p_inv;
};

class OuterCircle {
public:

    OuterCircle(const QPointF &c, qreal radius)
        : m_c(c),
          m_radius(radius),
          m_radius_sq(pow2(radius)),
          m_fadeCoeff(1.0 / (pow2(radius + 1.0) - m_radius_sq))
    {
    }

    qreal valueSq(const QPointF &pt) const {
        const qreal val = value(pt);

        return signZZ(val) * pow2(val);
    }

    qreal value(const QPointF &pt) const {
        return kisDistance(pt, m_c) - m_radius;
    }

    int pos(const QPointF &pt) const {
        return signZZ(valueSq(pt));
    }

    qreal fadeSq(const QPointF &pt) const {
        const qreal valSq = kisSquareDistance(pt, m_c);
        return (valSq - m_radius_sq) * m_fadeCoeff;
    }

private:
    const QPointF m_c;
    const qreal m_radius;
    const qreal m_radius_sq;
    const qreal m_fadeCoeff;
};


/**
 * Cuts off a portion of a rect \p rc defined by a half-plane \p p
 * \return the bounding rect of the resulting polygon
 */
KRITAIMAGE_EXPORT
QRectF cutOffRect(const QRectF &rc, const KisAlgebra2D::RightHalfPlane &p);


/**
 * Solves a quadratic equation in a form:
 *
 * a * x^2 + b * x + c = 0
 *
 * WARNING: Please note that \p a *must* be nonzero!  Otherwise the
 * equation is not quadratic! And this function doesn't check that!
 *
 * \return the number of solutions. It can be 0, 1 or 2.
 *
 * \p x1, \p x2 --- the found solution. The variables are filled with
 *                  data iff the corresponding solution is found. That
 *                  is: 0 solutions --- variabled are not touched, 1
 *                  solution --- x1 is filled with the result, 2
 *                  solutions --- x1 and x2 are filled.
 */
KRITAIMAGE_EXPORT
int quadraticEquation(qreal a, qreal b, qreal c, qreal *x1, qreal *x2);

/**
 * Finds the points of intersections between two circles
 * \return the found circles, the result can have 0, 1 or 2 points
 */
KRITAIMAGE_EXPORT
QVector<QPointF> intersectTwoCircles(const QPointF &c1, qreal r1,
                                     const QPointF &c2, qreal r2);


/**
 * Scale the relative point \pt into the bounds of \p rc. The point might be
 * outside the rectangle.
 */
inline QPointF relativeToAbsolute(const QPointF &pt, const QRectF &rc) {
    return rc.topLeft() + QPointF(pt.x() * rc.width(), pt.y() * rc.height());
}

/**
 * Get the relative position of \p pt inside rectangle \p rc. The point can be
 * outside the rectangle.
 */
inline QPointF absoluteToRelative(const QPointF &pt, const QRectF &rc) {
    if (!rc.isValid()) return QPointF();

    const QPointF rel = pt - rc.topLeft();
    return QPointF(rel.x() / rc.width(), rel.y() / rc.height());

}


}


#endif /* __KIS_ALGEBRA_2D_H */
