/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ALGEBRA_2D_H
#define __KIS_ALGEBRA_2D_H

#include <QPoint>
#include <QPointF>
#include <QVector>
#include <QPolygonF>
#include <QTransform>
#include <cmath>
#include <kis_global.h>
#include <kritaglobal_export.h>
#include <functional>
#include <boost/optional.hpp>

class QPainterPath;
class QTransform;

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
    typedef QRect rect_type;
};

template <>
struct PointTypeTraits<QPointF>
{
    typedef qreal value_type;
    typedef qreal calculation_type;
    typedef QRectF rect_type;
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

template <typename Point>
Point lerp(const Point &pt1, const Point &pt2, qreal t)
{
    return pt1 + (pt2 - pt1) * t;
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

template<class T>
typename std::enable_if<std::is_integral<T>::value, T>::type
divideFloor(T a, T b)
{
    const bool a_neg = a < T(0);
    const bool b_neg = b < T(0);

    if (a == T(0)) {
        return 0;
    } else if (a_neg == b_neg) {
        return a / b;
    } else {
        const T a_abs = qAbs(a);
        const T b_abs = qAbs(b);

        return - 1 - (a_abs - T(1)) / b_abs;
    }
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

void KRITAGLOBAL_EXPORT adjustIfOnPolygonBoundary(const QPolygonF &poly, int polygonDirection, QPointF *pt);

/**
 * Let \p pt, \p base1 are two vectors. \p base1 is uniformly scaled
 * and then rotated into \p base2 using transformation matrix S *
 * R. The function applies the same transformation to \pt and returns
 * the result.
 **/
QPointF KRITAGLOBAL_EXPORT transformAsBase(const QPointF &pt, const QPointF &base1, const QPointF &base2);

qreal KRITAGLOBAL_EXPORT angleBetweenVectors(const QPointF &v1, const QPointF &v2);

/**
 * Computes an angle indicating the direction from p1 to p2. If p1 and p2 are too close together to
 * compute an angle, defaultAngle is returned.
 */
qreal KRITAGLOBAL_EXPORT directionBetweenPoints(const QPointF &p1, const QPointF &p2,
                                                qreal defaultAngle);

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

template <template <class T> class Container, class Point, class Rect>
inline void accumulateBounds(const Container<Point> &points, Rect *bounds)
{
    Q_FOREACH (const Point &pt, points) {
        accumulateBounds(pt, bounds);
    }
}

template <template <class T> class Container, class Point>
inline typename PointTypeTraits<Point>::rect_type
accumulateBounds(const Container<Point> &points)
{
    typename PointTypeTraits<Point>::rect_type result;

    Q_FOREACH (const Point &pt, points) {
        accumulateBounds(pt, &result);
    }

    return result;
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

template <class Size>
auto maxDimension(Size size) -> decltype(size.width()) {
    return qMax(size.width(), size.height());
}

template <class Size>
auto minDimension(Size size) -> decltype(size.width()) {
    return qMin(size.width(), size.height());
}

QPainterPath KRITAGLOBAL_EXPORT smallArrow();

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

QPoint KRITAGLOBAL_EXPORT ensureInRect(QPoint pt, const QRect &bounds);
QPointF KRITAGLOBAL_EXPORT ensureInRect(QPointF pt, const QRectF &bounds);

template <class Rect>
Rect ensureRectNotSmaller(Rect rc, const decltype(Rect().size()) &size)
{
    typedef decltype(Rect().size()) Size;
    typedef decltype(Rect().top()) ValueType;

    if (rc.width() < size.width() ||
        rc.height() < size.height()) {

        ValueType width = qMax(rc.width(), size.width());
        ValueType  height = qMax(rc.height(), size.height());

        rc = Rect(rc.topLeft(), Size(width, height));
    }

    return rc;
}

template <class Size>
Size ensureSizeNotSmaller(const Size &size, const Size &bounds)
{
    Size result = size;

    const auto widthBound = qAbs(bounds.width());
    auto width = result.width();
    if (qAbs(width) < widthBound) {
        width = copysign(widthBound, width);
        result.setWidth(width);
    }

    const auto heightBound = qAbs(bounds.height());
    auto height = result.height();
    if (qAbs(height) < heightBound) {
        height = copysign(heightBound, height);
        result.setHeight(height);
    }

    return result;
}


/**
 * Attempt to intersect a line to the area of the a rectangle.
 *
 * If the line intersects the rectangle, it will be modified to represent the intersecting line segment and true is returned.
 * If the line does not intersect the area, it remains unmodified and false will be returned.
 *
 * @param segment
 * @param area
 * @return true if successful
 */
bool KRITAGLOBAL_EXPORT intersectLineRect(QLineF &line, const QRect rect);


template <class Point>
inline Point abs(const Point &pt) {
    return Point(qAbs(pt.x()), qAbs(pt.y()));
}

template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
inline T wrapValue(T value, T wrapBounds) {
    value %= wrapBounds;
    if (value < 0) {
        value += wrapBounds;
    }
    return value;
}

template<typename T, typename std::enable_if<std::is_floating_point<T>::value, T>::type* = nullptr>
inline T wrapValue(T value, T wrapBounds) {
    value = std::fmod(value, wrapBounds);
    if (value < 0) {
        value += wrapBounds;
    }
    return value;
}

template<typename T, typename std::enable_if<std::is_same<decltype(T().x()), decltype(T().y())>::value, T>::type* = nullptr>
inline T wrapValue(T value, T wrapBounds) {
    value.rx() = wrapValue(value.x(), wrapBounds.x());
    value.ry() = wrapValue(value.y(), wrapBounds.y());
    return value;
}

template<typename T>
inline T wrapValue(T value, T min, T max) {
    return wrapValue(value - min, max - min) + min;
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

QVector<QPoint> KRITAGLOBAL_EXPORT sampleRectWithPoints(const QRect &rect);
QVector<QPointF> KRITAGLOBAL_EXPORT sampleRectWithPoints(const QRectF &rect);

QRect KRITAGLOBAL_EXPORT approximateRectFromPoints(const QVector<QPoint> &points);
QRectF KRITAGLOBAL_EXPORT approximateRectFromPoints(const QVector<QPointF> &points);

QRect KRITAGLOBAL_EXPORT approximateRectWithPointTransform(const QRect &rect, std::function<QPointF(QPointF)> func);


/**
 * Cuts off a portion of a rect \p rc defined by a half-plane \p p
 * \return the bounding rect of the resulting polygon
 */
KRITAGLOBAL_EXPORT
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
KRITAGLOBAL_EXPORT
int quadraticEquation(qreal a, qreal b, qreal c, qreal *x1, qreal *x2);

/**
 * Finds the points of intersections between two circles
 * \return the found circles, the result can have 0, 1 or 2 points
 */
KRITAGLOBAL_EXPORT
QVector<QPointF> intersectTwoCircles(const QPointF &c1, qreal r1,
                                     const QPointF &c2, qreal r2);

KRITAGLOBAL_EXPORT
QTransform mapToRect(const QRectF &rect);

KRITAGLOBAL_EXPORT
QTransform mapToRectInverse(const QRectF &rect);

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
    const QPointF rel = pt - rc.topLeft();
    return QPointF(rc.width() > 0 ? rel.x() / rc.width() : 0,
                   rc.height() > 0 ? rel.y() / rc.height() : 0);

}

/**
 * Scales relative isotropic value from relative to absolute coordinate system
 * using SVG 1.1 rules (see chapter 7.10)
 */
inline qreal relativeToAbsolute(qreal value, const QRectF &rc) {
    const qreal coeff = std::sqrt(pow2(rc.width()) + pow2(rc.height())) / std::sqrt(2.0);
    return value * coeff;
}

/**
 * Scales absolute isotropic value from absolute to relative coordinate system
 * using SVG 1.1 rules (see chapter 7.10)
 */
inline qreal absoluteToRelative(const qreal value, const QRectF &rc) {
    const qreal coeff = std::sqrt(pow2(rc.width()) + pow2(rc.height())) / std::sqrt(2.0);
    return coeff != 0 ? value / coeff : 0;
}

/**
 * Scales relative isotropic value from relative to absolute coordinate system
 * using SVG 1.1 rules (see chapter 7.10)
 */
inline QRectF relativeToAbsolute(const QRectF &rel, const QRectF &rc) {
    return QRectF(relativeToAbsolute(rel.topLeft(), rc), relativeToAbsolute(rel.bottomRight(), rc));
}

/**
 * Scales absolute isotropic value from absolute to relative coordinate system
 * using SVG 1.1 rules (see chapter 7.10)
 */
inline QRectF absoluteToRelative(const QRectF &rel, const QRectF &rc) {
    return QRectF(absoluteToRelative(rel.topLeft(), rc), absoluteToRelative(rel.bottomRight(), rc));
}

/**
 * Compare the matrices with tolerance \p delta
 */
bool KRITAGLOBAL_EXPORT fuzzyMatrixCompare(const QTransform &t1, const QTransform &t2, qreal delta);

/**
 * Returns true if the two points are equal within some tolerance, where the tolerance is determined
 * by Qt's built-in fuzzy comparison functions.
 */
bool KRITAGLOBAL_EXPORT fuzzyPointCompare(const QPointF &p1, const QPointF &p2);

/**
 * Returns true if the two points are equal within the specified tolerance
 */
bool KRITAGLOBAL_EXPORT fuzzyPointCompare(const QPointF &p1, const QPointF &p2, qreal delta);

/**
 * Returns true if points in two containers are equal with specified tolerance
 */
template <template<typename> class Cont, class Point>
bool fuzzyPointCompare(const Cont<Point> &c1, const Cont<Point> &c2, qreal delta)
{
    if (c1.size() != c2.size()) return false;

    const qreal eps = delta;

    return std::mismatch(c1.cbegin(),
                         c1.cend(),
                         c2.cbegin(),
                         [eps] (const QPointF &pt1, const QPointF &pt2) {
                               return fuzzyPointCompare(pt1, pt2, eps);
                         })
            .first == c1.cend();
}

/**
 * Compare two rectangles with tolerance \p tolerance. The tolerance means that the
 * coordinates of top left and bottom right corners should not differ more than \p tolerance
 * pixels.
 */
template<class Rect, typename Difference = decltype(Rect::width())>
bool fuzzyCompareRects(const Rect &r1, const Rect &r2, Difference tolerance) {
    typedef decltype(r1.topLeft()) Point;

    const Point d1 = abs(r1.topLeft() - r2.topLeft());
    const Point d2 = abs(r1.bottomRight() - r2.bottomRight());

    const Difference maxError = std::max({d1.x(), d1.y(), d2.x(), d2.y()});
    return maxError < tolerance;
}

struct KRITAGLOBAL_EXPORT DecomposedMatix {
    DecomposedMatix();

    DecomposedMatix(const QTransform &t0);

    inline QTransform scaleTransform() const
    {
        return QTransform::fromScale(scaleX, scaleY);
    }

    inline QTransform shearTransform() const
    {
        QTransform t;
        t.shear(shearXY, 0);
        return t;
    }

    inline QTransform rotateTransform() const
    {
        QTransform t;
        t.rotate(angle);
        return t;
    }

    inline QTransform translateTransform() const
    {
        return QTransform::fromTranslate(dx, dy);
    }

    inline QTransform projectTransform() const
    {
        return
            QTransform(
                1,0,proj[0],
                0,1,proj[1],
                0,0,proj[2]);
    }

    inline QTransform transform() const {
        return
            scaleTransform() *
            shearTransform() *
            rotateTransform() *
            translateTransform() *
            projectTransform();
    }

    inline bool isValid() const {
        return valid;
    }

    qreal scaleX = 1.0;
    qreal scaleY = 1.0;
    qreal shearXY = 0.0;
    qreal angle = 0.0;
    qreal dx = 0.0;
    qreal dy = 0.0;
    qreal proj[3] = {0.0, 0.0, 1.0};

private:
    bool valid = true;
};

std::pair<QPointF, QTransform> KRITAGLOBAL_EXPORT transformEllipse(const QPointF &axes, const QTransform &fullLocalToGlobal);

QPointF KRITAGLOBAL_EXPORT alignForZoom(const QPointF &pt, qreal zoom);


/**
 * Linearly reshape function \p x so that in range [x0, x1]
 * it would cross points (x0, y0) and (x1, y1).
 */
template <typename T>
inline T linearReshapeFunc(T x, T x0, T x1, T y0, T y1)
{
    return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}


/**
 * Find intersection of a bounded line \p boundedLine with unbounded
 * line \p unboundedLine (if an intersection exists)
 */
KRITAGLOBAL_EXPORT
boost::optional<QPointF> intersectLines(const QLineF &boundedLine, const QLineF &unboundedLine);


/**
 * Find intersection of a bounded line \p p1, \p p2 with unbounded
 * line \p q1, \p q2 (if an intersection exists)
 */
KRITAGLOBAL_EXPORT
boost::optional<QPointF> intersectLines(const QPointF &p1, const QPointF &p2,
                                        const QPointF &q1, const QPointF &q2);

/**
 * Find possible positions for point p3, such that points \p1, \p2 and p3 form
 * a triangle, such that the distance between p1 ad p3 is \p a and the distance
 * between p2 and p3 is b. There might be 0, 1 or 2 such positions.
 */
QVector<QPointF> KRITAGLOBAL_EXPORT findTrianglePoint(const QPointF &p1, const QPointF &p2, qreal a, qreal b);

/**
 * Find a point p3 that forms a triangle with \p1 and \p2 and is the nearest
 * possible point to \p nearest
 *
 * \see findTrianglePoint
 */
boost::optional<QPointF> KRITAGLOBAL_EXPORT findTrianglePointNearest(const QPointF &p1, const QPointF &p2, qreal a, qreal b, const QPointF &nearest);

/**
 * @brief moveElasticPoint moves point \p pt based on the model of elasticity
 * @param pt point in question, tied to points \p base, \p wingA and \p wingB
 *           using springs
 * @param base initial position of the dragged point
 * @param newBase final position of tht dragged point
 * @param wingA first anchor point
 * @param wingB second anchor point
 * @return the new position of \p pt
 *
 * The function requires (\p newBase - \p base) be much smaller than any
 * of the distances between other points. If larger distances are necessary,
 * then use integration.
 */
QPointF KRITAGLOBAL_EXPORT moveElasticPoint(const QPointF &pt,
                                            const QPointF &base, const QPointF &newBase,
                                            const QPointF &wingA, const QPointF &wingB);

/**
 * @brief moveElasticPoint moves point \p pt based on the model of elasticity
 * @param pt point in question, tied to points \p base, \p anchorPoints
 *           using springs
 * @param base initial position of the dragged point
 * @param newBase final position of tht dragged point
 * @param anchorPoints anchor points
 * @return the new position of \p pt
 *
 * The function expects (\p newBase - \p base) be much smaller than any
 * of the distances between other points. If larger distances are necessary,
 * then use integration.
 */
QPointF KRITAGLOBAL_EXPORT moveElasticPoint(const QPointF &pt,
                                            const QPointF &base, const QPointF &newBase,
                                            const QVector<QPointF> &anchorPoints);


/**
 * @brief a simple class to generate Halton sequence
 *
 * This sequence of numbers can be used to sample areas
 * in somewhat uniform way. See Wikipedia for more info:
 *
 * https://en.wikipedia.org/wiki/Halton_sequence
 */

class HaltonSequenceGenerator
{
public:
    HaltonSequenceGenerator(int base)
        : m_base(base)
    {
    }

    int generate(int maxRange) {
        generationStep();
        return (m_n * maxRange + m_d / 2) / m_d;
    }

    qreal generate() {
        generationStep();
        return qreal(m_n) / m_d;
    }

private:
    inline void generationStep() {
        int x = m_d - m_n;

        if (x == 1) {
            m_n = 1;
            m_d *= m_base;
        } else {
            int y = m_d / m_base;
            while (x <= y) {
                y /= m_base;
            }
            m_n = (m_base + 1) * y - x;
        }
    }

private:
    int m_n = 0;
    int m_d = 1;
    const int m_base = 0;
};


}


#endif /* __KIS_ALGEBRA_2D_H */
