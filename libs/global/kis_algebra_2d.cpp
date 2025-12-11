/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_algebra_2d.h"

#include <QTransform>
#include <QPainterPath>
#include <kis_debug.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>

#include <array>
#include <QVector2D>
#include <QVector3D>

#include <QtMath>

#include <config-gsl.h>

#ifdef HAVE_GSL
#include <gsl/gsl_multimin.h>
#endif /*HAVE_GSL*/

#include <Eigen/Eigenvalues>
#include <KisBezierUtils.h>

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

qreal directionBetweenPoints(const QPointF &p1, const QPointF &p2, qreal defaultAngle)
{
    if (fuzzyPointCompare(p1, p2)) {
        return defaultAngle;
    }

    const QVector2D diff(p2 - p1);
    return std::atan2(diff.y(), diff.x());
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

bool intersectLineRect(QLineF &line, const QRect rect, bool extend)
{
    return intersectLineRect(line, rect, extend, extend);
}

bool intersectLineRect(QLineF &line, const QRect rect, bool extendFirst, bool extendSecond)
{
    // using Liang-Barsky algorithm
    // using parametric equation for a line:
    // X = x1 + t(x2-x1) = x1 + t*p2
    // Y = y1 + t(y2-y1) = y1 + t*p4
    // note that we have a line *segment* here, not a line
    // t1 = 0, t2 = 1, no matter what points we got

    double p1 = -(line.x2() - line.x1());
    double p2 = -p1;
    double p3 = -(line.y2() - line.y1());
    double p4 = -p3;

    QVector<double> p = QVector<double>({p1, p2, p3, p4});
    QVector<double> q = QVector<double>({line.x1() - rect.x(), rect.x() + rect.width() - line.x1(), line.y1() - rect.y(), rect.y() + rect.height() - line.y1()});

    float tmin = extendFirst ? -std::numeric_limits<float>::infinity() : 0; // line.p1() - first point, or infinity
    float tmax = extendSecond ? std::numeric_limits<float>::infinity() : 1; // line.p2() - second point, or infinity

    for (int i = 0; i < p.length(); i++) {
        // now clipping
        if (p[i] == 0 && q[i] < 0) {
            // line is parallel to the boundary and outside of it
            return false;
        } else if (p[i] < 0) {
            // line entry point (should be tmin)
            double t = q[i]/p[i];
            if (t > tmax) {
                if (extendSecond) {
                    tmax = t;
                } else {
                    return false;
                }
            }
            if (t > tmin) {
                tmin = t;
            }


        } else if (p[i] > 0) {
            // line leaving point (should be tmax)
            double t = q[i]/p[i];
            if (t < tmin) {
                if (extendFirst) {
                    tmin = t;
                } else {
                    return false;
                }
            }
            if (t < tmax) {
                tmax = t;
            }
        }
    }

    QPointF pt1 = line.p1();
    QPointF pt2 = line.p2();

    if (extendFirst || tmin > 0) {
        pt1 = QPointF(line.x1() + tmin*(line.x2() - line.x1()), line.y1() + tmin*(line.y2() - line.y1()));
    }
    if (extendSecond || tmax < 1) {
        pt2 = QPointF(line.x1() + tmax*(line.x2() - line.x1()), line.y1() + tmax*(line.y2() - line.y1()));
    }

    line.setP1(pt1);
    line.setP2(pt2);

    return true;

}

bool intersectLineConvexPolygon(QLineF &line, const QPolygonF polygon, bool extendFirst, bool extendSecond)
{
    qreal epsilon = 1e-07;

    if (polygon.size() == 0) {
        return false;
    }

    // trivial case: no extends, all points inside the polygon
    if (!extendFirst && !extendSecond && polygon.containsPoint(line.p1(), Qt::WindingFill) && polygon.containsPoint(line.p2(), Qt::WindingFill)) {
        return true;
    }

    // Cyrus-Beck algorithm: https://en.wikipedia.org/wiki/Cyrus%E2%80%93Beck_algorithm

    // parametric equation for the line:
    // p(t) = t*P1 + (1-t)*P2

    // we can't use infinity here, because the points would all end up as (+/- inf, +/- inf)
    // which would be useless if we have a very specific direction
    // hence we use just "a big number"
    float aBigNumber = 100000; // that will keep t*Px still on the line // this is LIMITATION of the algorithm
    float tmin = extendFirst ? -aBigNumber : 0; // line.p1() - first point, or infinity
    float tmax = extendSecond ? aBigNumber : 1; // line.p2() - second point, or infinity



    QList<QLineF> clippingLines;
    QList<QPointF> normals;

    bool clockwise = false;
    {
        // only temporary values to check whether the polygon is clockwise or counterclockwise
        QPointF vec1 = polygon[1] - polygon[0];
        QPointF vec2 = polygon[2] - polygon[0];

        QLineF line1(QPointF(0, 0), vec1);
        QLineF line2(QPointF(0, 0), vec2);

        qreal angle = line1.angleTo(line2); // range: <0, 360) // <0, 180) means counter-clockwise
        if (angle >= 180) {
            clockwise = true;
        }
    }


    for (int i = 0; i < polygon.size() - 1; i++) {
        clippingLines.append(QLineF(polygon[i], polygon[i + 1]));
        float xdiff = polygon[i].x() - polygon[i + 1].x();
        float ydiff = polygon[i].y() - polygon[i + 1].y();

        if (!clockwise) {
            normals.append(QPointF(ydiff, -xdiff));
        } else {
            normals.append(QPointF(-ydiff, xdiff));
        }

    }

    if (!polygon.isClosed()) {
        int i = polygon.size() - 1;
        clippingLines.append(QLineF(polygon[i], polygon[0]));
        float xdiff = polygon[i].x() - polygon[0].x();
        float ydiff = polygon[i].y() - polygon[0].y();

        if (!clockwise) {
            normals.append(QPointF(ydiff, -xdiff));
        } else {
            normals.append(QPointF(-ydiff, xdiff));
        }

    }

    QPointF lineVec = line.p2() - line.p1();
    // ax + by + c = 0
    // c = -ax - by
//    qreal cFromStandardEquation = -lineVec.x()*line.p1().x() - lineVec.y()*line.p1().y();


    QPointF p1 = line.p1();
    QPointF p2 = line.p2();

    qreal pA, pB, pC; // standard equation for the line: ax + by + c = 0
    if (qAbs(lineVec.x()) < epsilon) {
        // x1 ~= x2, line looks like: x = -pC
        pB = 0;
        pA = 1;
        pC = -p1.x();
    } else {
        pB = 1; // let b = 1 to simplify
        qreal tmp = (p1.y() - p2.y())/(p1.x() - p2.x());
        pA = -tmp;
        pC = tmp * p1.x() - p1.y();
    }


    int pEFound = 0;


    for (int i = 0; i < clippingLines.size(); i++) {

        // is the clipping line parallel to the line?
        QPointF clipVec = clippingLines[i].p2() - clippingLines[i].p1();

        if (qFuzzyCompare(clipVec.x()*lineVec.y(), clipVec.y()*lineVec.x())) {

            // vectors are parallel
            // let's check if one of the clipping points is on the line (extended) to see if it's the same line; if not, proceed normally

            qreal distanceBetweenLines = qAbs(pA*clippingLines[i].p1().x() + pB*clippingLines[i].p1().y() + pC)
                    /(sqrt(pA*pA + pB*pB));


            if (qAbs(distanceBetweenLines) < epsilon) {
                // they are the same line

                qreal t1;
                qreal t2;

                if (qAbs(p2.x() - p1.x()) > epsilon) {
                    t1 = (clippingLines[i].p1().x() - p1.x())/(p2.x() - p1.x());
                    t2 = (clippingLines[i].p2().x() - p1.x())/(p2.x() - p1.x());
                } else {
                    t1 = (clippingLines[i].p1().y() - p1.y())/(p2.y() - p1.y());
                    t2 = (clippingLines[i].p2().y() - p1.y())/(p2.y() - p1.y());
                }

                qreal tmin1 = qMin(t1, t2);
                qreal tmax2 = qMax(t1, t2);



                if (tmin < tmin1) {
                    tmin = tmin1;
                }
                if (tmax > tmax2) {
                    tmax = tmax2;
                }
                continue;
            }
            else {
                // if clipping line points are P1 and P2, and some other point is P(t) (given by parametric equation),
                // and N is the normal vector
                // and one of the points of the line we're intersecting is K,
                // then we have a following equation:
                // K = P(t) + a * N
                // where t and a are unknown.
                // after simplifying we get:
                // t*(p2 - p1) + a*(n) = k - p1
                // and since we have two axis, we get a linear system of two equations
                // A * [t; a] = B

                Eigen::Matrix2f A;
                Eigen::Vector2f b;
                A << p2.x() - p1.x(), normals[i].x(),
                     p2.y() - p1.y(), normals[i].y();
                b << clippingLines[i].p1().x() - p1.x(),
                     clippingLines[i].p1().y() - p1.y();

                Eigen::Vector2f ta = A.colPivHouseholderQr().solve(b);

                qreal tt = ta(1);
                if (tt < 0) {
                    return false;
                }
            }

        }


        boost::optional<QPointF> pEOptional = intersectLines(clippingLines[i], line); // bounded, unbounded


        if (pEOptional) {
            pEFound++;

            QPointF pE = pEOptional.value();
            QPointF n = normals[i];

            QPointF A = line.p2() - line.p1();
            QPointF B = line.p1() - pE;

            qreal over = (n.x()*B.x() + n.y()*B.y());
            qreal under = (n.x()*A.x() + n.y()*A.y());
            qreal t = -over/under;

//            if (pE.x() != p2.x()) {
//                qreal maybet = (pE.x() - p1.x())/(p2.x() - p1.x());
//            }

            qreal tminvalue = tmin * under + over;
            qreal tmaxvalue = tmax * under + over;

            if (tminvalue > 0 && tmaxvalue > 0) {
                // both outside of the edge
                return false;
            }
            if (tminvalue <= 0 && tmaxvalue <= 0) {
                // noop, both inside
            }
            if (tminvalue*tmaxvalue < 0) {

                // on two different sides
                if (tminvalue > 0) {
                    // tmin outside, tmax inside
                    if (t > tmin) {
                        tmin = t;
                    }
                } else {
                    if (t < tmax) {
                        tmax = t;
                    }
                }
            }



            if (tmax < tmin) {
                return false;
            }
        }
        else {
        }

    }

    if (pEFound == 0) {
        return false;
    }

    QLineF response = QLineF(tmin*line.p2() + (1-tmin)*line.p1(), tmax*line.p2() + (1-tmax)*line.p1());
    line = response;
    return true;
}

    template <class Rect, class Point>
    QVector<Point> sampleRectWithPoints(const Rect &rect)
    {
        QVector<Point> points;

        Point m1 = 0.5 * (rect.topLeft() + rect.topRight());
        Point m2 = 0.5 * (rect.bottomLeft() + rect.bottomRight());

        points << rect.topLeft();
        points << m1;
        points << rect.topRight();

        points << 0.5 * (rect.topLeft() + rect.bottomLeft());
        points << 0.5 * (m1 + m2);
        points << 0.5 * (rect.topRight() + rect.bottomRight());

        points << rect.bottomLeft();
        points << m2;
        points << rect.bottomRight();

        return points;
    }

    QVector<QPoint> sampleRectWithPoints(const QRect &rect)
    {
        return sampleRectWithPoints<QRect, QPoint>(rect);
    }

    QVector<QPointF> sampleRectWithPoints(const QRectF &rect)
    {
        return sampleRectWithPoints<QRectF, QPointF>(rect);
    }


    template <class Rect, class Point, bool alignPixels>
    Rect approximateRectFromPointsImpl(const QVector<Point> &points)
    {
        using namespace boost::accumulators;
        accumulator_set<qreal, stats<tag::min, tag::max > > accX;
        accumulator_set<qreal, stats<tag::min, tag::max > > accY;

        Q_FOREACH (const Point &pt, points) {
            accX(pt.x());
            accY(pt.y());
        }

        Rect resultRect;

        if (alignPixels) {
            resultRect.setCoords(std::floor(min(accX)), std::floor(min(accY)),
                                 std::ceil(max(accX)), std::ceil(max(accY)));
        } else {
            resultRect.setCoords(min(accX), min(accY),
                                 max(accX), max(accY));
        }

        return resultRect;
    }

    QRect approximateRectFromPoints(const QVector<QPoint> &points)
    {
        return approximateRectFromPointsImpl<QRect, QPoint, true>(points);
    }

    QRectF approximateRectFromPoints(const QVector<QPointF> &points)
    {
        return approximateRectFromPointsImpl<QRectF, QPointF, false>(points);
    }

    QRect approximateRectWithPointTransform(const QRect &rect, std::function<QPointF(QPointF)> func)
    {
        QVector<QPoint> points = sampleRectWithPoints(rect);

        using namespace boost::accumulators;
        accumulator_set<qreal, stats<tag::min, tag::max > > accX;
        accumulator_set<qreal, stats<tag::min, tag::max > > accY;

        Q_FOREACH (const QPoint &pt, points) {
            QPointF dstPt = func(pt);

            accX(dstPt.x());
            accY(dstPt.y());
        }

        QRect resultRect;
        resultRect.setCoords(std::floor(min(accX)), std::floor(min(accY)),
                             std::ceil(max(accX)), std::ceil(max(accY)));

        return resultRect;
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
            cutLine.intersects(QLineF(p1, p2), &intersection);
            resultPoints << intersection;
        }

        if (p2Valid) {
            resultPoints << p2;
        }

        p1 = p2;
        p1Valid = p2Valid;
    }

    return approximateRectFromPoints(resultPoints);
}

int quadraticEquation(qreal a, qreal b, qreal c, qreal *x1, qreal *x2)
{
    int numSolutions = 0;

    const qreal D = pow2(b) - 4 * a * c;
    const qreal eps = 1e-14;

    if (qAbs(D) <= eps) {
        *x1 = -b / (2 * a);
        numSolutions = 1;
    } else if (D < 0) {
        return 0;
    } else {
        const qreal sqrt_D = std::sqrt(D);

        *x1 = (-b + sqrt_D) / (2 * a);
        *x2 = (-b - sqrt_D) / (2 * a);
        numSolutions = 2;
    }

    return numSolutions;
}

QVector<QPointF> intersectTwoCircles(const QPointF &center1, qreal r1,
                                     const QPointF &center2, qreal r2)
{
    QVector<QPointF> points;

    const QPointF diff = (center2 - center1);
    const QPointF c1;
    const QPointF c2 = diff;

    const qreal centerDistance = norm(diff);

    if (centerDistance > r1 + r2) return points;
    if (centerDistance < qAbs(r1 - r2)) return points;

    if (centerDistance < qAbs(r1 - r2) + 0.001) {
        dbgKrita << "Skipping intersection" << ppVar(center1) << ppVar(center2) << ppVar(r1) << ppVar(r2) << ppVar(centerDistance) << ppVar(qAbs(r1-r2));
        return points;
    }

    const qreal x_kp1 = diff.x();
    const qreal y_kp1 = diff.y();

    const qreal F2 =
        0.5 * (pow2(x_kp1) +
               pow2(y_kp1) + pow2(r1) - pow2(r2));

    const qreal eps = 1e-6;

    if (qAbs(diff.y()) < eps) {
        qreal x = F2 / diff.x();
        qreal y1, y2;
        int result = KisAlgebra2D::quadraticEquation(
            1, 0,
            pow2(x) - pow2(r2),
            &y1, &y2);

        KIS_SAFE_ASSERT_RECOVER(result > 0) { return points; }

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
            1 + pow2(A), -2 * A * C,
            pow2(C) - pow2(r1),
            &x1, &x2);

        KIS_SAFE_ASSERT_RECOVER(result > 0) { return points; }

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

    for (int i = 0; i < points.size(); i++) {
        points[i] = center1 + points[i];
    }

    return points;
}

QTransform mapToRect(const QRectF &rect)
{
    return
        QTransform(rect.width(), 0, 0, rect.height(),
                   rect.x(), rect.y());
}

QTransform mapToRectInverse(const QRectF &rect)
{
    return
        QTransform::fromTranslate(-rect.x(), -rect.y()) *
        QTransform::fromScale(rect.width() != 0 ? 1.0 / rect.width() : 0.0,
                              rect.height() != 0 ? 1.0 / rect.height() : 0.0);
}

bool fuzzyMatrixCompare(const QTransform &t1, const QTransform &t2, qreal delta) {
    return
            qAbs(t1.m11() - t2.m11()) < delta &&
            qAbs(t1.m12() - t2.m12()) < delta &&
            qAbs(t1.m13() - t2.m13()) < delta &&
            qAbs(t1.m21() - t2.m21()) < delta &&
            qAbs(t1.m22() - t2.m22()) < delta &&
            qAbs(t1.m23() - t2.m23()) < delta &&
            qAbs(t1.m31() - t2.m31()) < delta &&
            qAbs(t1.m32() - t2.m32()) < delta &&
            qAbs(t1.m33() - t2.m33()) < delta;
}

bool fuzzyPointCompare(const QPointF &p1, const QPointF &p2)
{
    return qFuzzyCompare(p1.x(), p2.x()) && qFuzzyCompare(p1.y(), p2.y());
}


bool fuzzyPointCompare(const QPointF &p1, const QPointF &p2, qreal delta)
{
    return qAbs(p1.x() - p2.x()) < delta && qAbs(p1.y() - p2.y()) < delta;
}


inline QTransform toQTransformStraight(const Eigen::Matrix3d &m)
{
    return QTransform(m(0,0), m(0,1), m(0,2),
                      m(1,0), m(1,1), m(1,2),
                      m(2,0), m(2,1), m(2,2));
}

inline Eigen::Matrix3d fromQTransformStraight(const QTransform &t)
{
    Eigen::Matrix3d m;

    m << t.m11() , t.m12() , t.m13()
        ,t.m21() , t.m22() , t.m23()
        ,t.m31() , t.m32() , t.m33();

    return m;
}

/********************************************************/
/*             DecomposedMatrix                         */
/********************************************************/

DecomposedMatrix::DecomposedMatrix()
{
}

DecomposedMatrix::DecomposedMatrix(const QTransform &t0)
{
    QTransform t(t0);

    QTransform projMatrix;

    if (t.m33() == 0.0 || t0.determinant() == 0.0) {
        qWarning() << "Cannot decompose matrix!" << t;
        valid = false;
        return;
    }

    if (t.type() == QTransform::TxProject) {
        QTransform affineTransform(
            t.m11(), t.m12(), 0,
            t.m21(), t.m22(), 0,
            t.m31(), t.m32(), 1
        );
        projMatrix = affineTransform.inverted() * t;

        t = affineTransform;
        proj[0] = projMatrix.m13();
        proj[1] = projMatrix.m23();
        proj[2] = projMatrix.m33();
    }

    // can't use QVector3D, because they have too little accuracy for ellipse in perspective calculations
    std::array<Eigen::Vector3d, 3> rows;


    //  << t.m11() << t.m12() << t.m13())
    rows[0] = Eigen::Vector3d(t.m11(), t.m12(), t.m13());
    rows[1] = Eigen::Vector3d(t.m21(), t.m22(), t.m23());
    rows[2] = Eigen::Vector3d(t.m31(), t.m32(), t.m33());

    if (!qFuzzyCompare(t.m33(), 1.0)) {
        const qreal invM33 = 1.0 / t.m33();

        for (auto &row : rows) {
            row *= invM33;
        }
    }

    dx = rows[2].x();
    dy = rows[2].y();

    rows[2] = Eigen::Vector3d(0,0,1);


    scaleX = rows[0].norm();
    rows[0] *= 1.0 / scaleX;

    shearXY = rows[0].dot(rows[1]);
    rows[1] = rows[1] - shearXY * rows[0];

    scaleY = rows[1].norm();
    rows[1] *= 1.0 / scaleY;
    shearXY *= 1.0 / scaleY;

    // If determinant is negative, one axis was flipped.
    qreal determinant = rows[0].x() * rows[1].y() - rows[0].y() * rows[1].x();
    if (determinant < 0) {
        // Flip axis with minimum unit vector dot product.
        if (rows[0].x() < rows[1].y()) {
            scaleX = -scaleX;
            rows[0] = -rows[0];
        } else {
            scaleY = -scaleY;
            rows[1] = -rows[1];
        }
        shearXY = - shearXY;
    }

    angle = kisRadiansToDegrees(std::atan2(rows[0].y(), rows[0].x()));

    if (angle != 0.0) {
        // Rotate(-angle) = [cos(angle), sin(angle), -sin(angle), cos(angle)]
        //                = [row0x, -row0y, row0y, row0x]
        // Thanks to the normalization above.

        qreal sn = -rows[0].y();
        qreal cs = rows[0].x();
        qreal m11 = rows[0].x();
        qreal m12 = rows[0].y();
        qreal m21 = rows[1].x();
        qreal m22 = rows[1].y();
        rows[0][0] = (cs * m11 + sn * m21);
        rows[0][1] = (cs * m12 + sn * m22);
        rows[1][0] = (-sn * m11 + cs * m21);
        rows[1][1] = (-sn * m12 + cs * m22);
    }

    QTransform leftOver(
                rows[0].x(), rows[0].y(), rows[0].z(),
            rows[1].x(), rows[1].y(), rows[1].z(),
            rows[2].x(), rows[2].y(), rows[2].z());

    if (/*true || */!fuzzyMatrixCompare(leftOver, QTransform(), 1e-4)) {
        // what's wrong?
        ENTER_FUNCTION() << "FAILING THE ASSERT BELOW!";
        ENTER_FUNCTION() << ppVar(leftOver);
        ENTER_FUNCTION() << "matrix to decompose was: " << ppVar(t0);
        ENTER_FUNCTION() << ppVar(t.m33()) << ppVar(t0.determinant());
        Eigen::Matrix3d mat1 = fromQTransformStraight(QTransform());
        Eigen::Matrix3d mat2 = fromQTransformStraight(leftOver);
        Eigen::Matrix3d mat3 = mat2 - mat1;
        ENTER_FUNCTION() << mat3(0, 0) << mat3(0, 1) << mat3(0, 2);
        ENTER_FUNCTION() << mat3(1, 0) << mat3(1, 1) << mat3(1, 2);
        ENTER_FUNCTION() << mat3(2, 0) << mat3(2, 1) << mat3(2, 2);
        //ENTER_FUNCTION() << ppVar(mat1 - mat2);
    }


    KIS_SAFE_ASSERT_RECOVER_NOOP(fuzzyMatrixCompare(leftOver, QTransform(), 1e-4));
    KIS_ASSERT(fuzzyMatrixCompare(leftOver, QTransform(), 1e-4));
}


std::pair<QPointF, QTransform> transformEllipse(const QPointF &axes, const QTransform &fullLocalToGlobal)
{
    KisAlgebra2D::DecomposedMatrix decomposed(fullLocalToGlobal);
    const QTransform localToGlobal =
            decomposed.scaleTransform() *
            decomposed.shearTransform() *

            /* decomposed.projectTransform() * */
            /*decomposed.translateTransform() * */

            decomposed.rotateTransform();

    const QTransform localEllipse = QTransform(1.0 / pow2(axes.x()), 0.0, 0.0,
                                               0.0, 1.0 / pow2(axes.y()), 0.0,
                                               0.0, 0.0, 1.0);


    const QTransform globalToLocal = localToGlobal.inverted();

    Eigen::Matrix3d eqM =
        fromQTransformStraight(globalToLocal *
                               localEllipse *
                               globalToLocal.transposed());

//    std::cout << "eqM:" << std::endl << eqM << std::endl;

    Eigen::EigenSolver<Eigen::Matrix3d> eigenSolver(eqM);

    const Eigen::Matrix3d T = eigenSolver.eigenvalues().real().asDiagonal();
    const Eigen::Matrix3d U = eigenSolver.eigenvectors().real();

    const Eigen::Matrix3d Ti = eigenSolver.eigenvalues().imag().asDiagonal();
    const Eigen::Matrix3d Ui = eigenSolver.eigenvectors().imag();

    KIS_SAFE_ASSERT_RECOVER_NOOP(Ti.isZero());
    KIS_SAFE_ASSERT_RECOVER_NOOP(Ui.isZero());
    KIS_SAFE_ASSERT_RECOVER_NOOP((U * U.transpose()).isIdentity());

//    std::cout << "T:" << std::endl << T << std::endl;
//    std::cout << "U:" << std::endl << U << std::endl;

//    std::cout << "Ti:" << std::endl << Ti << std::endl;
//    std::cout << "Ui:" << std::endl << Ui << std::endl;

//    std::cout << "UTU':" << std::endl << U * T * U.transpose() << std::endl;

    const qreal newA = 1.0 / std::sqrt(T(0,0) * T(2,2));
    const qreal newB = 1.0 / std::sqrt(T(1,1) * T(2,2));

    const QTransform newGlobalToLocal = toQTransformStraight(U);
    const QTransform newLocalToGlobal = QTransform::fromScale(-1,-1) *
            newGlobalToLocal.inverted() *
            decomposed.translateTransform();

    return std::make_pair(QPointF(newA, newB), newLocalToGlobal);
}

QPointF alignForZoom(const QPointF &pt, qreal zoom)
{
    return QPointF((pt * zoom).toPoint()) / zoom;
}

boost::optional<QPointF> intersectLines(const QLineF &boundedLine, const QLineF &unboundedLine)
{
    const QPointF B1 = unboundedLine.p1();
    const QPointF A1 = unboundedLine.p2() - unboundedLine.p1();

    const QPointF B2 = boundedLine.p1();
    const QPointF A2 = boundedLine.p2() - boundedLine.p1();

    Eigen::Matrix<float, 2, 2> A;
    A << A1.x(), -A2.x(),
         A1.y(), -A2.y();

    Eigen::Matrix<float, 2, 1> B;
    B << B2.x() - B1.x(),
         B2.y() - B1.y();

    if (qFuzzyIsNull(A.determinant())) {
        return boost::none;
    }

    Eigen::Matrix<float, 2, 1> T;

    T = A.inverse() * B;

    const qreal t2 = T(1,0);
    double epsilon = 1e-06; // to avoid precision issues

    if (t2 < 0.0 || t2 > 1.0) {
        if (qAbs(t2) > epsilon && qAbs(t2 - 1.0) > epsilon) {
            return boost::none;
        }
    }

    return t2 * A2 + B2;
}

boost::optional<QPointF> intersectLines(const QPointF &p1, const QPointF &p2,
                                        const QPointF &q1, const QPointF &q2)
{
    return intersectLines(QLineF(p1, p2), QLineF(q1, q2));
}

QVector<QPointF> findTrianglePoint(const QPointF &p1, const QPointF &p2, qreal a, qreal b)
{
    using KisAlgebra2D::norm;
    using KisAlgebra2D::dotProduct;

    QVector<QPointF> result;

    const QPointF p = p2 - p1;

    const qreal pSq = dotProduct(p, p);

    const qreal T =  0.5 * (-pow2(b) + pow2(a) + pSq);

    if (p.isNull()) return result;

    if (qAbs(p.x()) > qAbs(p.y())) {
        const qreal A = 1.0;
        const qreal B2 = -T * p.y() / pSq;
        const qreal C = pow2(T) / pSq - pow2(a * p.x()) / pSq;

        const qreal D4 = pow2(B2) - A * C;

        if (D4 > 0 || qFuzzyIsNull(D4)) {
            if (qFuzzyIsNull(D4)) {
                const qreal y = -B2 / A;
                const qreal x = (T - y * p.y()) / p.x();
                result << p1 + QPointF(x, y);
            } else {
                const qreal y1 = (-B2 + std::sqrt(D4)) / A;
                const qreal x1 = (T - y1 * p.y()) / p.x();
                result << p1 + QPointF(x1, y1);

                const qreal y2 = (-B2 - std::sqrt(D4)) / A;
                const qreal x2 = (T - y2 * p.y()) / p.x();
                result << p1 + QPointF(x2, y2);
            }
        }
    } else {
        const qreal A = 1.0;
        const qreal B2 = -T * p.x() / pSq;
        const qreal C = pow2(T) / pSq - pow2(a * p.y()) / pSq;

        const qreal D4 = pow2(B2) - A * C;

        if (D4 > 0 || qFuzzyIsNull(D4)) {
            if (qFuzzyIsNull(D4)) {
                const qreal x = -B2 / A;
                const qreal y = (T - x * p.x()) / p.y();
                result << p1 + QPointF(x, y);
            } else {
                const qreal x1 = (-B2 + std::sqrt(D4)) / A;
                const qreal y1 = (T - x1 * p.x()) / p.y();
                result << p1 + QPointF(x1, y1);

                const qreal x2 = (-B2 - std::sqrt(D4)) / A;
                const qreal y2 = (T - x2 * p.x()) / p.y();
                result << p1 + QPointF(x2, y2);
            }
        }
    }

    return result;
}

boost::optional<QPointF> findTrianglePointNearest(const QPointF &p1, const QPointF &p2, qreal a, qreal b, const QPointF &nearest)
{
    const QVector<QPointF> points = findTrianglePoint(p1, p2, a, b);

    boost::optional<QPointF> result;

    if (points.size() == 1) {
        result = points.first();
    } else if (points.size() > 1) {
        result = kisDistance(points.first(), nearest) < kisDistance(points.last(), nearest) ? points.first() : points.last();
    }

    return result;
}

QPointF moveElasticPoint(const QPointF &pt, const QPointF &base, const QPointF &newBase, const QPointF &wingA, const QPointF &wingB)
{
    using KisAlgebra2D::norm;
    using KisAlgebra2D::dotProduct;
    using KisAlgebra2D::crossProduct;

    const QPointF vecL = base - pt;
    const QPointF vecLa = wingA - pt;
    const QPointF vecLb = wingB - pt;

    const qreal L = norm(vecL);
    const qreal La = norm(vecLa);
    const qreal Lb = norm(vecLb);

    const qreal sinMuA = crossProduct(vecLa, vecL) / (La * L);
    const qreal sinMuB = crossProduct(vecL, vecLb) / (Lb * L);

    const qreal cosMuA = dotProduct(vecLa, vecL) / (La * L);
    const qreal cosMuB = dotProduct(vecLb, vecL) / (Lb * L);

    const qreal dL = dotProduct(newBase - base, -vecL) / L;


    const qreal divB = (cosMuB + La / Lb * sinMuB * cosMuA / sinMuA) / L +
            (cosMuB + sinMuB * cosMuA / sinMuA) / Lb;
    const qreal dLb = dL / ( L * divB);

    const qreal divA = (cosMuA + Lb / La * sinMuA * cosMuB / sinMuB) / L +
            (cosMuA + sinMuA * cosMuB / sinMuB) / La;
    const qreal dLa = dL / ( L * divA);

    boost::optional<QPointF> result =
        KisAlgebra2D::findTrianglePointNearest(wingA, wingB, La + dLa, Lb + dLb, pt);

    const QPointF resultPoint = result ? *result : pt;

    return resultPoint;
}

#ifdef HAVE_GSL

struct ElasticMotionData
{
    QPointF oldBasePos;
    QPointF newBasePos;
    QVector<QPointF> anchorPoints;

    QPointF oldResultPoint;
};

double elasticMotionError(const gsl_vector * x, void *paramsPtr)
{
    using KisAlgebra2D::norm;
    using KisAlgebra2D::dotProduct;
    using KisAlgebra2D::crossProduct;

    const QPointF newResultPoint(gsl_vector_get(x, 0), gsl_vector_get(x, 1));

    const ElasticMotionData *p = static_cast<const ElasticMotionData*>(paramsPtr);

    const QPointF vecL = newResultPoint - p->newBasePos;
    const qreal L = norm(vecL);

    const qreal deltaL = L - kisDistance(p->oldBasePos, p->oldResultPoint);

    QVector<qreal> deltaLi;
    QVector<qreal> Li;
    QVector<qreal> cosMuI;
    QVector<qreal> sinMuI;
    Q_FOREACH (const QPointF &anchorPoint, p->anchorPoints) {
        const QPointF vecLi = newResultPoint - anchorPoint;
        const qreal _Li = norm(vecLi);

        Li << _Li;
        deltaLi << _Li - kisDistance(p->oldResultPoint, anchorPoint);
        cosMuI << dotProduct(vecLi, vecL) / (_Li * L);
        sinMuI << crossProduct(vecL, vecLi) / (_Li * L);
    }

    qreal finalError = 0;

    qreal tangentialForceSum = 0;
    for (int i = 0; i < p->anchorPoints.size(); i++) {
        const qreal sum = deltaLi[i] * sinMuI[i] / Li[i];
        tangentialForceSum += sum;
    }

    finalError += pow2(tangentialForceSum);

    qreal normalForceSum = 0;
    {
        qreal sum = 0;
        for (int i = 0; i < p->anchorPoints.size(); i++) {
            sum += deltaLi[i] * cosMuI[i] / Li[i];
        }
        normalForceSum = (-deltaL) / L - sum;
    }

    finalError += pow2(normalForceSum);

    return finalError;
}


#endif /* HAVE_GSL */

QPointF moveElasticPoint(const QPointF &pt,
                         const QPointF &base, const QPointF &newBase,
                         const QVector<QPointF> &anchorPoints)
{
    const QPointF offset = newBase - base;

    QPointF newResultPoint = pt + offset;

#ifdef HAVE_GSL

    ElasticMotionData data;
    data.newBasePos = newBase;
    data.oldBasePos = base;
    data.anchorPoints = anchorPoints;
    data.oldResultPoint = pt;

    const gsl_multimin_fminimizer_type *T =
        gsl_multimin_fminimizer_nmsimplex2;
    gsl_multimin_fminimizer *s = 0;
    gsl_vector *ss, *x;
    gsl_multimin_function minex_func;

    size_t iter = 0;
    int status;
    double size;

    /* Starting point */
    x = gsl_vector_alloc (2);
    gsl_vector_set (x, 0, newResultPoint.x());
    gsl_vector_set (x, 1, newResultPoint.y());

    /* Set initial step sizes to 0.1 */
    ss = gsl_vector_alloc (2);
    gsl_vector_set (ss, 0, 0.1 * offset.x());
    gsl_vector_set (ss, 1, 0.1 * offset.y());

    /* Initialize method and iterate */
    minex_func.n = 2;
    minex_func.f = elasticMotionError;
    minex_func.params = (void*)&data;

    s = gsl_multimin_fminimizer_alloc (T, 2);
    gsl_multimin_fminimizer_set (s, &minex_func, x, ss);

    do
    {
        iter++;
        status = gsl_multimin_fminimizer_iterate(s);

        if (status)
            break;

        size = gsl_multimin_fminimizer_size (s);
        status = gsl_multimin_test_size (size, 1e-6);

        /**
         * Sometimes the algorithm may converge to a wrong point,
         * then just try to force it search better or return invalid
         * result.
         */
        if (status == GSL_SUCCESS && elasticMotionError(s->x, &data) > 0.5) {
            status = GSL_CONTINUE;
        }

        if (status == GSL_SUCCESS)
        {
//             qDebug() << "*******Converged to minimum";
//             qDebug() << gsl_vector_get (s->x, 0)
//                      << gsl_vector_get (s->x, 1)
//                      << "|" << s->fval << size;
//             qDebug() << ppVar(iter);

             newResultPoint.rx() = gsl_vector_get (s->x, 0);
             newResultPoint.ry() = gsl_vector_get (s->x, 1);
        }
    }
    while (status == GSL_CONTINUE && iter < 10000);

    if (status != GSL_SUCCESS) {
        ENTER_FUNCTION() << "failed to find point" << ppVar(pt) << ppVar(base) << ppVar(newBase);
    }

    gsl_vector_free(x);
    gsl_vector_free(ss);
    gsl_multimin_fminimizer_free (s);
#endif

    return newResultPoint;
}

void cropLineToRect(QLineF &line, const QRect rect, bool extendFirst, bool extendSecond)
{
    bool intersects = intersectLineRect(line, rect, extendFirst, extendSecond);
    if (!intersects) {
        line = QLineF(); // empty line to help with drawing
    }
}

void cropLineToConvexPolygon(QLineF &line, const QPolygonF polygon, bool extendFirst, bool extendSecond)
{
    bool intersects = intersectLineConvexPolygon(line, polygon, extendFirst, extendSecond);
    if (!intersects) {
        line = QLineF(); // empty line to help with drawing
    }
}

int lineSideForPoint(const QLineF &line, const QPointF &point)
{
    // TODO: do we really neede these explicit checks? Do they help with
    //       precision?

    if (fuzzyPointCompare(point, line.p1())) {
        return 0;
    }
    if (fuzzyPointCompare(point, line.p2())) {
        return 0;
    }
    if (qFuzzyCompare(line.length(), 0)) {
        return 0;
    }

    qreal whichSide = KisAlgebra2D::crossProduct(line.p2() - line.p1(), point - line.p1());
    return qFuzzyIsNull(whichSide) ? 0 : (whichSide > 0 ? 1 : -1);
}

QPolygonF combineConvexHullParts(const QPolygonF &leftPolygon, QPolygonF &rightPolygon, bool triangular) {
    // resulting polygon should start at p1, go through all the points, go to p2, then to p1 again
    // triangular: whether the last point of right is equal to the first point in left or not

    QPolygonF left = leftPolygon;
    QPolygonF right = rightPolygon;

    left.removeLast(); // remove p1 from the end (but not the beginning)
    left.removeLast(); // remove nextPoint as well, since it's present in rightPolygon (twice)

    right.removeLast(); // remove nextPoint from the end (but not the beginning)


    QPolygonF result;
    Q_FOREACH(QPointF point, left) {
        result << point;
    }

    Q_FOREACH(QPointF point, right) {
        result << point;
    }

    if (triangular) {
        result << left[0];
    }

    return result;
}

QPolygonF calculateConvexHullFromPointsOverTheLine(const QPolygonF &points, const QLineF &line)
{
    // all the points should be on the correct side already, no need to check it
    if (points.count() < 1) {
        QPolygonF result;
        result << line.p1() << line.p2() << line.p1();
        return result;
    }
    if (points.count() == 1) {
        QList<QPointF> list = points.toList();

        QPolygonF result;
        result << line.p1();
        result << list[0];
        result << line.p2();
        result << line.p1();

        return result;
    }

    double maxDistance = 0;
    QPointF nextPoint = points[0];
    Q_FOREACH(QPointF point, points) {
        double distance = kisSquareDistanceToLine(point, line);
        if (distance > maxDistance) {
            maxDistance = distance;
            nextPoint = point;
        }
    }

    QPolygonF left;
    QLineF lineForLeft = QLineF(line.p1(), nextPoint);
    QPolygonF right;
    QLineF lineForRight = QLineF(nextPoint, line.p2());
    QPolygonF triangle;
    triangle << line.p1() << line.p2() << nextPoint << line.p1();
    Q_FOREACH(QPointF point, points) {
        if (triangle.containsPoint(point, Qt::WindingFill)) {
            continue;
        }
        if (lineSideForPoint(lineForLeft, point) > 0) {
            left << point;
        } else if (lineSideForPoint(lineForRight, point) < 0) {
            right << point;
        }
    }

    QPolygonF leftPolygon = calculateConvexHullFromPointsOverTheLine(left, lineForLeft);
    QPolygonF rightPolygon = calculateConvexHullFromPointsOverTheLine(right, lineForRight);

    QPolygonF result = combineConvexHullParts(leftPolygon, rightPolygon, true);

    return result;
}


QPolygonF calculateConvexHull(const QPolygonF &polygon)
{
    if (polygon.count() < 4) {
        return polygon;
    }
    QPointF leftPoint = polygon[0];
    QPointF rightPoint = polygon[1];
    if (leftPoint.x() > rightPoint.x()) { // swap them around
        leftPoint = polygon[1];
        rightPoint = polygon[0];
    }

    Q_FOREACH(QPointF point, polygon) {
        if (point.x() < leftPoint.x()) {
            leftPoint = point;
        } else if (point.x() > rightPoint.x()) {
            rightPoint = point;
        }
    }
    QLineF line(leftPoint, rightPoint);
    QLineF lineOther(rightPoint, leftPoint);
    QPolygonF left;
    QPolygonF right;

    Q_FOREACH(QPointF point, polygon) {
        qCritical() << "Checking point " << point << "and line" << line << ": " << lineSideForPoint(line, point);
        if (lineSideForPoint(line, point) > 0) {
            left << point;
        } else if (lineSideForPoint(line, point) < 0) {
            right << point;
        }
    }

    qCritical() << "Using line: " << line << "for points: " << left;
    qCritical() << "Using line: " << lineOther << "for points: " << right;

    left = calculateConvexHullFromPointsOverTheLine(left, line);
    right = calculateConvexHullFromPointsOverTheLine(right, lineOther);

    qCritical() << "Then the result: " << left << right;

    QPolygonF result = combineConvexHullParts(left, right, false);

    return result;

}

QList<QLineF> intersectLineConcavePolygon(const QPolygonF polygon, const QLineF& line, bool extendFirst, bool extendSecond) {

    // should use kis_convex_hull instead, that one uses boost
    QPolygonF convexHull = calculateConvexHull(polygon);
    if (convexHull.count() == polygon.count()) {
        QLineF resultLine = line;
        bool result = intersectLineConvexPolygon(resultLine, polygon, extendFirst, extendSecond);
        if (result) {
            return QList<QLineF>() << resultLine;
        } else {
            return QList<QLineF>();
        }
    }

    // it was a concave polygon
    // intersectLines

    // should be as easy as cutting every line, then sorting the points and creating the lines, except gotta take care of exceptions

    KIS_ASSERT(false && "Not implemented yet");

    return QList<QLineF>();
}


qreal findMinimumGoldenSection(std::function<qreal(qreal)> f, qreal xA, qreal xB, qreal eps, int maxIter = 100)
{
    // requirements:
    // only one local minimum between xA and xB

    int i = 0;
    const double phi = 0.618033988749894; // 1/(golden ratio)
    qreal a = xA;
    qreal b = xB;
    qreal c = b - (b - a)*phi;
    qreal d = a + (b - a)*phi;

    while (qAbs(b - a) > eps) {
        if (f(c) < f(d)) {
            b = d;
        } else {
            a = c;
        }

        // Wikipedia says to recompute both c and d here to avoid loss of precision which may lead to incorrect results or infinite loop
        c = b - (b - a) * phi;
        d = a + (b - a) * phi;

        i++;
        if (i > maxIter) {
            break;
        }

    }

    return (b + a)/2;
}

qreal findMinimumTernarySection(std::function<qreal(qreal)> f, qreal xA, qreal xB, qreal eps, int maxIter = 100)
{
    // requirements:
    // only one local minimum between xA and xB

    int i = 0;
    qreal l = qMin(xA, xB);
    qreal r = qMax(xA, xB);


    qreal m1 = l + (r - l)/3;
    qreal m2 = r - (r - l)/3;

    while ((r - l) > eps) {
        qreal f1 = f(m1);
        qreal f2 = f(m2);

        if (f1 > f2) {
            l = m1;
        } else {
            r = m2;
        }

        // In Golden Section, Wikipedia says to recompute both c and d here to avoid loss of precision which may lead to incorrect results or infinite loop
        // so it's probably a good idea to do it here too
        m1 = l + (r - l)/3;
        m2 = r - (r - l)/3;

        i++;

        if (i > maxIter) {
            break;
        }
    }

    return (l + r)/2;
}

qreal pointToLineDistSquared(const QPointF &pt, const QLineF &line)
{
    // distance = |(p2 - p1) x (p1 - pt)| / |p2 - p1|

    // magnitude of (p2 - p1) x (p1 - pt)
    const qreal cross = (line.dx() * (line.y1() - pt.y()) - line.dy() * (line.x1() - pt.x()));

    return cross * cross / (line.dx() * line.dx() + line.dy() * line.dy());
}

bool tryMergePoints(QPainterPath &path,
                    const QPointF &startPoint,
                    const QPointF &endPoint,
                    qreal &distance,
                    qreal distanceThreshold,
                    bool lastSegment)
{
    qreal length = (endPoint - startPoint).manhattanLength();

    if (lastSegment || length > distanceThreshold) {
        if (lastSegment) {
            qreal wrappedLength =
                (endPoint - QPointF(path.elementAt(0))).manhattanLength();

            if (length < distanceThreshold ||
                wrappedLength < distanceThreshold) {

                return true;
            }
        }

        distance = 0;
        return false;
    }

    distance += length;

    if (distance > distanceThreshold) {
        path.lineTo(endPoint);
        distance = 0;
    }

    return true;
}

QPainterPath trySimplifyPath(const QPainterPath &path, qreal lengthThreshold)
{
    QPainterPath newPath;
    QPointF startPoint;
    qreal distance = 0;

    int count = path.elementCount();
    for (int i = 0; i < count; i++) {
        QPainterPath::Element e = path.elementAt(i);
        QPointF endPoint = QPointF(e.x, e.y);

        switch (e.type) {
        case QPainterPath::MoveToElement:
            newPath.moveTo(endPoint);
            break;
        case QPainterPath::LineToElement:
            if (!tryMergePoints(newPath, startPoint, endPoint,
                                distance, lengthThreshold, i == count - 1)) {

                newPath.lineTo(endPoint);
            }
            break;
        case QPainterPath::CurveToElement: {
            Q_ASSERT(i + 2 < count);

            if (!tryMergePoints(newPath, startPoint, endPoint,
                                distance, lengthThreshold, i == count - 1)) {

                e = path.elementAt(i + 1);
                Q_ASSERT(e.type == QPainterPath::CurveToDataElement);
                QPointF ctrl1 = QPointF(e.x, e.y);
                e = path.elementAt(i + 2);
                Q_ASSERT(e.type == QPainterPath::CurveToDataElement);
                QPointF ctrl2 = QPointF(e.x, e.y);
                newPath.cubicTo(ctrl1, ctrl2, endPoint);
            }

            i += 2;
        }
        default:
            ;
        }
        startPoint = endPoint;
    }

    return newPath;
}


QList<QLineF> getParallelLines(const QLineF& line, const qreal distance) {

    QPointF lineNormalVector = QPointF(line.dy(), -line.dx());
    QLineF line1 = QLineF(movePointInTheDirection(line.p1(), lineNormalVector, distance), movePointInTheDirection(line.p2(), lineNormalVector, distance));
    QLineF line2 = QLineF(movePointInTheDirection(line.p1(), -lineNormalVector, distance), movePointInTheDirection(line.p2(), -lineNormalVector, distance));

    if (line1.isNull() || line2.isNull()) {
        return QList<QLineF>();
    }
    return {line1, line2};
}

QPainterPath getOnePathFromRectangleCutThrough(const QList<QPointF> &points, const QLineF &line, bool left) {

    auto onTheLine = [](QPointF first, QPointF second) {
        float delta = 0.1f;
        return qAbs(first.x() - second.x()) < delta || qAbs(first.y() - second.y()) < delta;
    };

    bool started = false;
    QPainterPath path;

    path.moveTo(line.p1());
    path.lineTo(line.p2());

    QList<QPointF> availablePoints;
    int maxRectPointsNumber = 4; // in case the list has five points to count the first one twice
    for(int i = 0; i < maxRectPointsNumber; i++) {
        qreal whichSide = KisAlgebra2D::crossProduct(line.p2() - line.p1(), line.p2() - points[i]);
        if (whichSide*(left ? 1 : -1) >= 0) {
            availablePoints << points[i];
        }
    }

    int startValue, increment;
    if (!left) {
        startValue = 2*availablePoints.length();
        increment = -1;
    } else {
        startValue = 0;
        increment = 1;
    }

    auto stopFor = [](int index, int max, bool left) {
        if (!left) {
            return index <= 0;
        } else {
            return index >= max;
        }
    };

    for (int i = startValue; !stopFor(i, 2*availablePoints.length(), left); i += increment) {
        int index = KisAlgebra2D::wrapValue(i, 0, (int)availablePoints.length());

        if (onTheLine(availablePoints[index], path.currentPosition())) {
            if (!started) {
                started = true;

                // TODO: if it's not the same point?
                path.lineTo(availablePoints[index]);
                if (onTheLine(availablePoints[index], line.p1())) {
                    break;
                }

            } else {
                // usually would add, unless it's the ending
                if (onTheLine(availablePoints[index], line.p1())) {
                    path.lineTo(availablePoints[index]);
                    break;
                }
                path.lineTo(availablePoints[index]);
            }
        }
    }


    path.lineTo(line.p1());

    return path;
}

QList<QPainterPath> getPathsFromRectangleCutThrough(const QRectF &rect, const QLineF &leftLine, const QLineF &rightLine) {


    QPainterPath left;
    QPainterPath right;

    left.moveTo(leftLine.p1());
    left.lineTo(leftLine.p2());

    right.moveTo(rightLine.p1());
    right.lineTo(rightLine.p2());

    QList<QPointF> rectPoints;
    rectPoints << rect.topLeft() << rect.bottomLeft() << rect.bottomRight() << rect.topRight();

    left = getOnePathFromRectangleCutThrough(rectPoints, leftLine, true);
    right = getOnePathFromRectangleCutThrough(rectPoints, rightLine, false);

    QList<QPainterPath> paths;
    paths << left << right;


    return paths;
}

QPointF findNearestPointOnLine(const QPointF &point, const QLineF &line, bool unbounded)
{
    // TODO: can we reuse (or deduplicate with) KisBezierUtils.cpp:nearestPoint()?

    if (line.length() == 0) {
        return point;
    }

    QPointF lineVector = line.p2() - line.p1();
    QPointF lineNormalVector = QPointF(-lineVector.y(), lineVector.x());

    QLineF pointToLineNormalLine = QLineF(point, point + lineNormalVector);
    QPointF result;
    QLineF::IntersectionType intersectionType = pointToLineNormalLine.intersects(line, &result);
    if (unbounded || intersectionType == QLineF::IntersectionType::BoundedIntersection) {
        return result;
    }
    qreal distance1 = kisDistance(line.p1(), result);
    qreal distance2 = kisDistance(line.p2(), result);
    if (distance1 + distance2 <= line.length() || qFuzzyCompare(distance1 + distance2, line.length())) {
        return result; // it's still on the line
    }
    if (distance1 < distance2) {
        return line.p1();
    }
    return line.p2();
}

QPointF movePointInTheDirection(const QPointF &point, const QPointF &direction, qreal distance)
{
    QPointF response = point;
    QLineF line = QLineF(QPointF(0, 0), direction);
    return response + distance*direction/line.length();
}

QPointF movePointAlongTheLine(const QPointF &point, const QLineF &direction, qreal distance, bool ensureOnLine)
{
    QPointF response = point;
    if (ensureOnLine) {
        response = findNearestPointOnLine(point, direction);
    }
    return movePointInTheDirection(response, direction.p2() - direction.p1(), distance);
}



QLineF getLineFromElements(const QPainterPath &shape, int index)
{
    QPainterPath::Element element1 = shape.elementAt(wrapValue(index, 0, shape.elementCount() - 1));
    QPainterPath::Element element2 = shape.elementAt(wrapValue(index + 1, 0, shape.elementCount() - 1));
    // it doesn't handle isCurveTo, just assumes it's a line... sorry!

    return QLineF(element1, element2);
}

// local
QLineF reverseDirection(const QLineF& line)
{
    return QLineF(line.p2(), line.p1());
}

// local
QPainterPath removeGutterOneEndSmart(const QPainterPath &shape1, int index1, const QPainterPath &shape2, int index2, QLineF middleLine, bool reverseFirstPoly, bool reverseSecondPoly)
{
    // moving from shape1.index1.p1 to shape1.index2.p2

    auto reverseIfNeeded = [] (const QLineF &line, bool reverse) {
        if (reverse) {
            return reverseDirection(line);
        }
        return line;
    };

    QLineF line1 = reverseIfNeeded(getLineFromElements(shape1, index1), reverseFirstPoly);
    QLineF line2 = reverseIfNeeded(getLineFromElements(shape2, index2), reverseSecondPoly);

    QLineF leftLine;
    QLineF rightLine;

    int indexMultiplierFirst = reverseFirstPoly ? -1 : 1;
    int indexMultiplierSecond = reverseSecondPoly ? -1 : 1;

    leftLine = reverseIfNeeded(getLineFromElements(shape1, index1 - indexMultiplierFirst*1), reverseFirstPoly);
    rightLine = reverseIfNeeded(getLineFromElements(shape2, index2 + indexMultiplierSecond*1), reverseSecondPoly);

    qreal leftPointDistanceFromMiddle;
    qreal rightPointDistanceFromMiddle;

    QPointF leftPoint = line1.p1();
    QPointF rightPoint = line2.p2();

    leftPointDistanceFromMiddle = kisDistanceToLine(leftPoint, middleLine);
    rightPointDistanceFromMiddle = kisDistanceToLine(rightPoint, middleLine);

    QPainterPath simpleResult;
    simpleResult.moveTo(leftPoint);
    simpleResult.lineTo(rightPoint);

    auto makeTrianglePath = [leftPoint, rightPoint] (QPointF middle) {
        QPainterPath result;
        result.moveTo(leftPoint);
        result.lineTo(middle);
        result.lineTo(rightPoint);
        return result;
    };

    QPointF intersectionPoint;
    QLineF::IntersectionType intersectionType = leftLine.intersects(rightLine, &intersectionPoint);

    qreal distanceToMiddle = 0;
    if (intersectionType != QLineF::NoIntersection) {
        distanceToMiddle = kisDistanceToLine(intersectionPoint, middleLine);
    }


    if (intersectionType == QLineF::NoIntersection || distanceToMiddle > qMax(leftPointDistanceFromMiddle, rightPointDistanceFromMiddle)) {

        // first arg: bounded line, second: unbounded
        boost::optional<QPointF> leftIntersection = intersectLines(line2, leftLine);
        boost::optional<QPointF> rightIntersection = intersectLines(line1, rightLine);

        if (leftIntersection.has_value()) {
            return makeTrianglePath(leftIntersection.value());
        } else if (rightIntersection.has_value()) {
            return makeTrianglePath(rightIntersection.value());
        } else {
            return simpleResult;
        }

    }

    QPainterPath betterResult = makeTrianglePath(intersectionPoint);

    return betterResult;

}

QPainterPath simplifyShape(const QPainterPath& path) {

    VectorPath vector(path);
    return vector.trulySimplified().asPainterPath();
}

VectorPath mergeShapesWithGutter(const VectorPath& shape1, const VectorPath& shape2, const VectorPath& oneEnd, const VectorPath& otherEnd, int index1, int index2, bool reverseSecondPoly, bool isSameShape)
{
    using VPoint = VectorPath::VectorPathPoint;
    QList<VPoint> result;

    result.append(shape1.pointAt(0));

    auto appendFrom = [] (QList<VPoint>& res, int start, int end, const VectorPath& shape) {
        for (int i = start; i < end; i++) {
            QList<VPoint> segment = shape.segmentAt(i);
            KIS_SAFE_ASSERT_RECOVER_RETURN(segment.length() == 2);
            res.append(segment[1]);
        }
    };

    VectorPath reversedShape2 = reverseSecondPoly ? shape2.reversed() : VectorPath(shape2);
    int reversedIndex2 = reverseSecondPoly ? (shape2.segmentsCount() - index2 - 1) : index2;

    if (isSameShape) {
        int minIndex = qMin(index1, index2);
        int maxIndex = qMax(index1, index2);

        // the indexes define a way to cut the shape into two shapes
        // and one of them is kind of more outer than the other
        // (they could intersect but... that's the user problem at that point, they can fix it point by point)
        // in normal situation, it's like an outer ring of edges and inner ring of edges (which might be just one edge, which is the most common case)
        // and we can find that out by checking whether the ends of the selected edges are inside or outside of the shape
        // and if they're inside a shape, we're assuming that's the resulting shape (and the other is discarded)
        // no islands in comic panels on my watch!

        QList<VPoint> result1;
        // min index to max index
        result1 << VPoint::moveTo(shape1.segmentAt(minIndex)[1].endPoint);
        appendFrom(result1, minIndex + 1, maxIndex, shape1);

        QList<VPoint> result2;
        result2 << VPoint::moveTo(shape1.segmentAt(maxIndex)[1].endPoint);
        appendFrom(result2, maxIndex + 1, shape1.segmentsCount(), shape1);
        appendFrom(result2, 0, minIndex, shape1);


        VectorPath minEnd = index1 > index2 ? oneEnd : otherEnd;
        VectorPath maxEnd = index1 > index2 ? otherEnd : oneEnd;

        appendFrom(result1, 0, minEnd.segmentsCount(), minEnd);
        appendFrom(result2, 0, maxEnd.segmentsCount(), maxEnd);

        VectorPath result1Vec(result1);
        VectorPath result2Vec(result2);

        QPolygonF result2Poly = result2Vec.asPainterPath().toFillPolygon();

        // TODO: assumes winding fill
        if (result2Poly.containsPoint(shape1.segmentAt(minIndex)[1].endPoint, Qt::WindingFill)) {
            // could check all
            return result2Vec;
        }
        return result1Vec;

    } else {
        appendFrom(result, 0, index1, shape1);
        appendFrom(result, 0, oneEnd.segmentsCount(), oneEnd);
        appendFrom(result, reversedIndex2 + 1, reversedShape2.segmentsCount(), reversedShape2);
        appendFrom(result, 0, reversedIndex2, reversedShape2);
        appendFrom(result, 0, otherEnd.segmentsCount(), otherEnd);
        appendFrom(result, index1 + 1, shape1.segmentsCount(), shape1);
    }

    return VectorPath(result);

}


QPainterPath removeGutterSmart(const QPainterPath &shape1, int index1, const QPainterPath &shape2, int index2, bool isSameShape)
{
    if (index1 + 1 >= shape1.elementCount() || index2 + 1 >= shape2.elementCount()) {
        return QPainterPath();
    }

    QLineF line1 = getLineFromElements(shape1, index1);
    QLineF line2 = getLineFromElements(shape2, index2);

    QVector<QPointF> poly1 = QVector<QPointF>() << line1.p1() << line1.p2() << line2.p1();
    QVector<QPointF> poly2 = QVector<QPointF>() << line1.p2() << line2.p1() << line2.p2();
    bool reverseSecondPoly = false;
    if (polygonDirection(poly1) != polygonDirection(poly2)) {
        line2 = QLineF(line2.p2(), line2.p1()); // quick fix to ensure that the shape will be correct (convex)
        reverseSecondPoly = true;
    }

    QLineF middleLine = QLineF((line1.p1() + line2.p2())/2, (line1.p2() + line2.p1())/2);

    // more difficult version:
    QPainterPath oneEnd = removeGutterOneEndSmart(shape1, index1, shape2, index2, middleLine, false, reverseSecondPoly);
    QPainterPath otherEnd = removeGutterOneEndSmart(shape2, index2, shape1, index1, middleLine, reverseSecondPoly, false);

    VectorPath vectorShape1 = VectorPath(shape1);
    VectorPath vectorShape2 = VectorPath(shape2);
    VectorPath vectorOneEnd = VectorPath(oneEnd);
    VectorPath vectorOtherEnd = VectorPath(otherEnd);

    VectorPath vectorResultHere = mergeShapesWithGutter(vectorShape1, vectorShape2, vectorOneEnd, vectorOtherEnd, index1, index2, reverseSecondPoly, isSameShape);
    return vectorResultHere.trulySimplified(1e-02).asPainterPath();



}


QList<int> getLineSegmentCrossingLineIndexes(const QLineF &line, const QPainterPath &shape)
{
    QList<int> indexes;
    for (int i = 0; i < shape.elementCount() - 1; i++) { // last element would wrap around to be the first segment again
        QLineF lineSegment = getLineFromElements(shape, i);
        QPointF intersection;
        QLineF::IntersectionType type = lineSegment.intersects(line, &intersection);
        if (type == QLineF::IntersectionType::BoundedIntersection) {
            indexes << i;
        }
    }
    return indexes;
}

VectorPath::VectorPath(const QPainterPath &path)
{
    using VPoint = VectorPath::VectorPathPoint;

    m_points = QList<VectorPath::VectorPathPoint>();
    for (int i = 0; i < path.elementCount(); i++) {
        QPainterPath::Element el = path.elementAt(i);
        switch(el.type) {
            case QPainterPath::MoveToElement:
                m_points.append(VPoint(VPoint::MoveTo, el));
            break;
            case QPainterPath::LineToElement:
                m_points.append(VPoint(VPoint::LineTo, el));
            break;
            case QPainterPath::CurveToElement:
                KIS_SAFE_ASSERT_RECOVER(i + 2 < path.elementCount()) { continue; }
                KIS_SAFE_ASSERT_RECOVER(path.elementAt(i + 1).type == QPainterPath::CurveToDataElement) { continue; }
                KIS_SAFE_ASSERT_RECOVER(path.elementAt(i + 2).type == QPainterPath::CurveToDataElement) { continue; }
                // Qt saves the data this way: [control point 1, CurveToElement] [control point 2, CurveToDataElement] [end point, CurveToDataElement]
                m_points.append(VPoint(VPoint::BezierTo, path.elementAt(i + 2), el, path.elementAt(i + 1)));
                i += 2; // skip next two points
            break;
        }
    }
}

VectorPath::VectorPath(const QList<VectorPathPoint> path)
{
    m_points = path;
}

int VectorPath::pointsCount() const
{
    return m_points.length();
}

VectorPath::VectorPathPoint VectorPath::pointAt(int i) const
{
    return m_points[i];
}

int VectorPath::segmentsCount() const
{
    return m_points.length() - 1 >= 0 ? m_points.length() - 1 : 0;
}

QList<VectorPath::VectorPathPoint> VectorPath::segmentAt(int i) const
{
    QList<VectorPath::VectorPathPoint> response;
    if (i < 0 || i >= segmentsCount()) {
        return response;
    }
    response << m_points[i] << m_points[i + 1];
    return response;
}

std::optional<VectorPath::Segment> VectorPath::segmentAtAsSegment(int i) const
{
    QList<VectorPath::VectorPathPoint> segment = segmentAt(i);
    if (segment.count() == 2) {
        return VectorPath::Segment(segment[0], segment[1]);
    }
    return std::nullopt;
}

QLineF VectorPath::segmentAtAsLine(int i) const
{
    QList<VectorPath::VectorPathPoint> points = segmentAt(i);
    if (points.length() < 1) {
        return QLineF();
    }
    return QLineF(points[0].endPoint, points[1].endPoint);
}

QList<QPointF> VectorPath::intersectSegmentWithLineBounded(const QLineF &line, const Segment &segment)
{
    qreal eps = 1e-5;
    if (segment.type != VectorPath::VectorPathPoint::BezierTo) {
        QLineF segLine = QLineF(segment.startPoint, segment.endPoint);
        QPointF intersection;
        if (segLine.intersects(line, &intersection) == QLineF::BoundedIntersection) {
            return QList<QPointF> {intersection};
        } else {
            return QList<QPointF>();
        }

    } else {
        QList<QPointF> result;
        // check for intersections
        QVector<qreal> intersections = KisBezierUtils::intersectWithLine(segment.startPoint, segment.controlPoint1, segment.controlPoint2, segment.endPoint, line, eps);
        for (int i = 0; i < intersections.count(); i++) {
            QPointF p = KisBezierUtils::bezierCurve(segment.startPoint, segment.controlPoint1, segment.controlPoint2, segment.endPoint, intersections[i]);
            bool onLine = isOnLine(line, p, eps, true, true, true);
            if (onLine) {
                result << p;
            }
        }

        return result;
    }

}

QList<QPointF> VectorPath::intersectSegmentWithLineBounded(const QLineF &line, const VectorPathPoint &p1, const VectorPathPoint &p2)
{
    return intersectSegmentWithLineBounded(line, VectorPath::Segment(p1, p2));
}

int VectorPath::pathIndexToSegmentIndex(int index)
{
    // first should be MoveTo
    int pathIndex = 0;
    for (int i = 1; i < m_points.length(); i++) {
        if (index < pathIndex) {
            // it was the previous segment, it's just a control point
            return i - 1;
        }
        if (index == pathIndex) {
            return i - 1;
        }
        if (m_points[i].type == VectorPathPoint::BezierTo) {
            pathIndex += 2; // add space for control points
        }
        pathIndex++;
    }
    return m_points.length() - 1;
}

int VectorPath::segmentIndexToPathIndex(int index)
{
    // first should be MoveTo
    int pathIndex = 0;
    for (int i = 1; i < m_points.length(); i++) {
        if (i - 1 == index) {
            if (m_points[i - 1].type == VectorPathPoint::BezierTo) {
                return pathIndex - 2;
            }
            return pathIndex;
        }
        if (m_points[i].type == VectorPathPoint::BezierTo) {
            pathIndex += 2; // add space for control points
        }
        pathIndex++;
    }
    if (m_points.length() > 0 && m_points[m_points.length() - 1].type == VectorPathPoint::BezierTo) {
        return pathIndex - 2;
    }
    return pathIndex;
}

VectorPath VectorPath::trulySimplified(qreal epsDegrees) const
{
    qreal eps = kisDegreesToRadians(epsDegrees);


    using VPoint = VectorPath::VectorPathPoint;

    if (m_points.length() < 1) {
        return VectorPath(QList<VectorPath::VectorPathPoint>());
    }

    QList<VPoint> response;
    VPoint lineBeginPoint = VPoint(VPoint::MoveTo, QPointF(0, 0));
    VPoint previousPoint = m_points[0];


    //qreal eps = 1e-05;
    auto onTheLine = [eps] (QPointF start, QPointF middle, QPointF end) {
        QLineF line1(start, end);
        QLineF line2(start, middle);

        return (qAbs(crossProduct(end - start, middle - start)/line1.length()/line2.length()) < eps);
    };

    for (int i = 1; i < m_points.length(); i++) {
        if (m_points[i].type == VPoint::MoveTo) {
            if (previousPoint.type == VPoint::MoveTo) {
                previousPoint = m_points[i]; // let's skip the previous point since they're both just moving
                continue;
            } else { // LineTo or BezierTo, both need to be added
                response << previousPoint;
                previousPoint = m_points[i];
                continue;
            }
        } else if (m_points[i].type == VPoint::LineTo) {
            if (previousPoint.type == VPoint::LineTo) {
                // check whether they're parallel
                if (!onTheLine(lineBeginPoint.endPoint, previousPoint.endPoint, m_points[i].endPoint)) {
                    response << previousPoint;
                    lineBeginPoint = previousPoint;
                }
                previousPoint = m_points[i]; // let's skip the previous point since they're parallel
                continue;
            } else { // MoveTo or BezierTo, both need to be added
                response << previousPoint;
                lineBeginPoint = previousPoint;
                previousPoint = m_points[i];
                continue;
            }
        } else { // currently in BezierTo
            response << previousPoint;
            previousPoint = m_points[i];
            continue;
        }
    }
    response << previousPoint;

    // now eliminate additional point at the beginning of the shape if it's in the middle of a line
    // and wasn't simplified only because it's at the beginning

    if (response.length() < 3) {
        return VectorPath(response);
    }

    VPoint start = response[0];
    VPoint second = response[1];

    if (start.type == VPoint::MoveTo) {
        if (fuzzyPointCompare(previousPoint.endPoint, start.endPoint)) {
            // they are the same point, now whether they're on the line
            if (second.type == VPoint::LineTo && onTheLine(response[response.length()-2].endPoint, start.endPoint, second.endPoint)) {
                response.pop_front();
                response[0].type = VPoint::MoveTo;
                response[response.length() - 1].endPoint = second.endPoint;
            }
        }

    } else if (response[0].type == VPoint::LineTo) { // first point is then QPointF(0, 0)
        if (qFuzzyIsNull(previousPoint.endPoint.x()) && qFuzzyIsNull(previousPoint.endPoint.y())) {
            // they are the same point, now whether they're on the line
            if (second.type == VPoint::LineTo && onTheLine(previousPoint.endPoint, QPointF(0, 0), start.endPoint)) {
                // note: leaving this debug for future reference
                qCritical() << "We're in this case";
                //response.pop_front();
                //response[0].type = VPoint::MoveTo;
                //response[response.length() - 1].endPoint = start.endPoint;
            }
        }
    }




    return VectorPath(response);
}

VectorPath VectorPath::reversed() const
{
    using VPoint = VectorPath::VectorPathPoint;
    QList<VPoint> response;
    response.append(VPoint(VPoint::MoveTo, m_points[m_points.length() - 1].endPoint));
    for (int i = m_points.length() - 1; i > 0; i--) {
         // to reverse a segment, we take the end VPoint and change the .endPoint to the start VPoint coords
        VPoint point = m_points[i];
        point.endPoint = m_points[i - 1].endPoint;
        response.append(point);
    }
    return VectorPath(response);
}

bool VectorPath::fuzzyComparePointsCyclic(const VectorPath &path, qreal eps) const
{
    using VPoint = VectorPath::VectorPathPoint;

    if (segmentsCount() != path.segmentsCount()) {
        return false;
    }

    bool fuzzy = qFuzzyIsNull(eps);

    auto comparePoints = [fuzzy, eps] (QPointF left, QPointF right) {
        if (fuzzy) {
            return fuzzyPointCompare(left, right);
        } else {
            return fuzzyPointCompare(left, right, eps);
        }
    };


    VPoint leftStart = path.pointAt(0);
    QList<int> rightStartingPoints;
    for (int i = 0; i < path.pointsCount(); i++) {
        if (comparePoints(leftStart.endPoint, path.pointAt(i).endPoint)) {
            rightStartingPoints << i;
        }
    }

    Q_FOREACH(int startId, rightStartingPoints) {
        bool isEqual = true;
        for (int i = 0; i < pointsCount(); i++) {
            int rightId = wrapValue(i + startId, 0, pointsCount());
            if (!comparePoints(pointAt(i).endPoint, path.pointAt(rightId).endPoint)) {
                isEqual = false;
                break;
            }
        }
        if (isEqual) {
            return true;
        }
        for (int i = 0; i < pointsCount(); i++) {
            int rightId = wrapValue(startId - i, 0, pointsCount());
            if (!comparePoints(pointAt(i).endPoint, path.pointAt(rightId).endPoint)) {
                isEqual = false;
                break;
            }
        }
        if (isEqual) {
            return true;
        }
    }

    return false;
}

QPainterPath VectorPath::asPainterPath() const
{
    using VPoint = VectorPath::VectorPathPoint;
    if (m_originalPath.elementCount() > 0) {
        return m_originalPath;
    }
    QPainterPath response;
    for (int i = 0; i < pointsCount(); i++) {
        switch(m_points[i].type) {
        case VPoint::MoveTo:
            response.moveTo(m_points[i].endPoint);
            break;
        case VPoint::LineTo:
            response.lineTo(m_points[i].endPoint);
            break;
        case VPoint::BezierTo:
            response.cubicTo(m_points[i].controlPoint1, m_points[i].controlPoint2, m_points[i].endPoint);
            break;
        }
    }
    response.closeSubpath();
    return response;
}

QDebug operator<<(QDebug debug, const VectorPath &path)
{
    debug << path.asPainterPath();
    return debug;
}

QDebug operator<<(QDebug debug, const VectorPath::VectorPathPoint &point)
{
    bool autoInsertSpaces = debug.autoInsertSpaces();
    debug.setAutoInsertSpaces(false);
    debug << (point.type == VectorPath::VectorPathPoint::MoveTo ? "(move " : (point.type == VectorPath::VectorPathPoint::BezierTo ? "(curve " : "(line "));
    debug << "(" << point.endPoint.x() << ", " << point.endPoint.y() << ")";
    if (point.type == VectorPath::VectorPathPoint::BezierTo) {
        debug << ": (";
        debug << "(" << point.controlPoint1.x() << ", " << point.controlPoint1.y() << ")";
        debug << ", ";
        debug << "(" << point.controlPoint2.x() << ", " << point.controlPoint2.y() << ")";
        debug << ")";
    }
    debug << ")";
    debug.setAutoInsertSpaces(autoInsertSpaces);
    return debug;
}

bool isInsideShape(const VectorPath &path, const QPointF &point)
{
    if (path.pointsCount() == 0) {
        return false;
    }

    QRectF boundRect;
    accumulateBounds(path.pointAt(0).endPoint, &boundRect);

    bool isPolygon = true;

    for (int i = 0; i < path.segmentsCount(); i++) {
        QList<VectorPath::VectorPathPoint> points = path.segmentAt(i);
        accumulateBounds(points[0].endPoint, &boundRect);
        accumulateBounds(points[1].endPoint, &boundRect);

        if (points[1].type == VectorPath::VectorPathPoint::BezierTo) {
            accumulateBounds(points[1].controlPoint1, &boundRect);
            accumulateBounds(points[1].controlPoint2, &boundRect);
            isPolygon = false;
        }
    }

    if (!boundRect.contains(point)) {
        return false;
    }

    if (isPolygon) {
        return path.asPainterPath().toFillPolygon().containsPoint(point, Qt::WindingFill);
    }

    boundRect = kisGrowRect(boundRect, 5); // just safety margins

    // NOTE: it would be way faster to do it taking into account the fact that the lines are vertical and horizontal
    // except for the last one maybe
    // but let's wait until it proves to be a problem
    QList<QLineF> lines;
    lines << QLineF(point, QPointF(boundRect.left(), point.y()));
    lines << QLineF(point, QPointF(boundRect.right(), point.y()));
    lines << QLineF(point, QPointF(point.x(), boundRect.top()));
    lines << QLineF(point, QPointF(point.x(), boundRect.bottom()));
    lines << QLineF(point, boundRect.topLeft()); // last resort, diagonal

    int intersectionsCount = 0;

    for (int k = 0; k < lines.length(); k++) {
        int intersections = 0;
        QLineF line = lines[k];
        bool checkNext = false;

        for (int i = 0; i < path.segmentsCount(); i++) {
            std::optional<VectorPath::Segment> segmentOpt = path.segmentAtAsSegment(i);
            KIS_SAFE_ASSERT_RECOVER(segmentOpt.has_value()) {continue;}
            VectorPath::Segment segment = segmentOpt.value();

            QList<QPointF> intersectionsHere = VectorPath::intersectSegmentWithLineBounded(line, segment);
            qreal eps = 1e-4;

            for (int i = 0; i < intersectionsHere.count(); i++) {
                if (fuzzyPointCompare(intersectionsHere[i], segment.startPoint, eps) || fuzzyPointCompare(intersectionsHere[i], segment.endPoint, eps)) {
                    checkNext = true;
                    break;
                }
            }

            if (checkNext) {
                break;
            }

            intersections += intersectionsHere.count();
        }

        intersectionsCount = intersections;
        if (!checkNext) {
            break;
        }
    }

    return intersectionsCount%2 == 1;
}

bool isInsideShape(const QPainterPath &path, const QPointF &point)
{
    for (int i = 0; i < path.elementCount(); i++) {
        if (path.elementAt(i).type == QPainterPath::CurveToDataElement) {
            // contains control points
            return isInsideShape(VectorPath(path), point);
        }
    }

    return path.toFillPolygon().containsPoint(point, Qt::WindingFill);
}

bool isOnLine(const QLineF &line, const QPointF &point, const qreal eps, bool boundedStart, bool boundedEnd, bool includeEnds)
{
    if (fuzzyPointCompare(point, line.p1(), eps)) {
        return includeEnds || !boundedStart;
    }
    if (fuzzyPointCompare(point, line.p2(), eps)) {
        return includeEnds || !boundedEnd;
    }

    if (kisDistanceToLine(point, line) > eps) {
        return false;
    }

    QPointF lineVector = line.p2() - line.p1();
    QPointF pointVector = point - line.p1();
    qreal t = -1;
    if (qAbs(pointVector.x()) < eps) {
        // use y
        t = pointVector.y()/lineVector.y();
    } else {
        t = pointVector.x()/lineVector.x();
    }

    if (t < 0) {
        return !boundedStart;
    }
    if (t > 1.0) {
        return !boundedEnd;
    }

    return true;
}


} // namespace
