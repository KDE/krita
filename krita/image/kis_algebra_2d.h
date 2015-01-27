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
#include <krita_export.h>

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
QRect KRITAIMAGE_EXPORT blowRect(const QRect &rect, qreal coeff);

QPoint ensureInRect(QPoint pt, const QRect &bounds);
QPointF ensureInRect(QPointF pt, const QRectF &bounds);



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

}


#endif /* __KIS_ALGEBRA_2D_H */
