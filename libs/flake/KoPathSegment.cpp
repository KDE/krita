/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008-2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoPathSegment.h"
#include "KoPathPoint.h"
#include <FlakeDebug.h>
#include <QPainterPath>
#include <QTransform>
#include <math.h>

#include "kis_global.h"

#include <KisBezierUtils.h>

class Q_DECL_HIDDEN KoPathSegment::Private
{
public:
    Private(KoPathSegment *qq, KoPathPoint *p1, KoPathPoint *p2)
        : first(p1),
        second(p2),
        q(qq)
    {
    }

    /// calculates signed distance of given point from segment chord
    qreal distanceFromChord(const QPointF &point) const;

    /// Returns the chord length, i.e. the distance between first and last control point
    qreal chordLength() const;

    /// Returns intersection of lines if one exists
    QList<QPointF> linesIntersection(const KoPathSegment &segment) const;

    /// Returns parameters for curve extrema
    QList<qreal> extrema() const;

    /// Returns parameters for curve roots
    QList<qreal> roots() const;

    /**
     * The DeCasteljau algorithm for parameter t.
     * @param t the parameter to evaluate at
     * @param p1 the new control point of the segment start (for cubic curves only)
     * @param p2 the first control point at t
     * @param p3 the new point at t
     * @param p4 the second control point at t
     * @param p3 the new control point of the segment end (for cubic curbes only)
     */
    void deCasteljau(qreal t, QPointF *p1, QPointF *p2, QPointF *p3, QPointF *p4, QPointF *p5) const;

    KoPathPoint *first;
    KoPathPoint *second;
    KoPathSegment *q;
};

void KoPathSegment::Private::deCasteljau(qreal t, QPointF *p1, QPointF *p2, QPointF *p3, QPointF *p4, QPointF *p5) const
{
    if (!q->isValid())
      return;

    int deg = q->degree();
    QPointF q[4];

    q[0] = first->point();
    if (deg == 2) {
        q[1] = first->activeControlPoint2() ? first->controlPoint2() : second->controlPoint1();
    } else if (deg == 3) {
        q[1] = first->controlPoint2();
        q[2] = second->controlPoint1();
    }
    if (deg >= 0) {
        q[deg] = second->point();
    }

    // points of the new segment after the split point
    QPointF p[3];

    // the De Casteljau algorithm
    for (unsigned short j = 1; j <= deg; ++j) {
        for (unsigned short i = 0; i <= deg - j; ++i) {
            q[i] = (1.0 - t) * q[i] + t * q[i + 1];
        }
        p[j - 1] = q[0];
    }

    if (deg == 2) {
        if (p2)
            *p2 = p[0];
        if (p3)
            *p3 = p[1];
        if (p4)
            *p4 = q[1];
    } else if (deg == 3) {
        if (p1)
            *p1 = p[0];
        if (p2)
            *p2 = p[1];
        if (p3)
            *p3 = p[2];
        if (p4)
            *p4 = q[1];
        if (p5)
            *p5 = q[2];
    }
}

QList<qreal> KoPathSegment::Private::roots() const
{
    QList<qreal> rootParams;

    if (!q->isValid())
        return rootParams;

    // Calculate how often the control polygon crosses the x-axis
    // This is the upper limit for the number of roots.
    int xAxisCrossings = KisBezierUtils::controlPolygonZeros(q->controlPoints());

    if (!xAxisCrossings) {
        // No solutions.
    }
    else if (xAxisCrossings == 1 && q->isFlat(0.01 / chordLength())) {
        // Exactly one solution.
        // Calculate intersection of chord with x-axis.
        QPointF chord = second->point() - first->point();
        QPointF segStart = first->point();
        rootParams.append((chord.x() * segStart.y() - chord.y() * segStart.x()) / - chord.y());
    }
    else {
        // Many solutions. Do recursive midpoint subdivision.
        QPair<KoPathSegment, KoPathSegment> splitSegments = q->splitAt(0.5);
        rootParams += splitSegments.first.d->roots();
        rootParams += splitSegments.second.d->roots();
    }

    return rootParams;
}

QList<qreal> KoPathSegment::Private::extrema() const
{
    int deg = q->degree();
    if (deg <= 1)
        return QList<qreal>();

    QList<qreal> params;

    /*
     * The basic idea for calculating the extrama for bezier segments
     * was found in comp.graphics.algorithms:
     *
     * Both the x coordinate and the y coordinate are polynomial. Newton told
     * us that at a maximum or minimum the derivative will be zero.
     *
     * We have a helpful trick for the derivatives: use the curve r(t) defined by
     * differences of successive control points.
     * Setting r(t) to zero and using the x and y coordinates of differences of
     * successive control points lets us find the parameters t, where the original
     * bezier curve has a minimum or a maximum.
     */
    if (deg == 2) {
        /*
         * For quadratic bezier curves r(t) is a linear Bezier curve:
         *
         *  1
         * r(t) = Sum Bi,1(t) *Pi = B0,1(t) * P0 + B1,1(t) * P1
         * i=0
         *
         * r(t) = (1-t) * P0 + t * P1
         *
         * r(t) = (P1 - P0) * t + P0
         */

        // calculating the differences between successive control points
        QPointF cp = first->activeControlPoint2() ?
                     first->controlPoint2() : second->controlPoint1();
        QPointF x0 = cp - first->point();
        QPointF x1 = second->point() - cp;

        // calculating the coefficients
        QPointF a = x1 - x0;
        QPointF c = x0;

        if (a.x() != 0.0)
            params.append(-c.x() / a.x());
        if (a.y() != 0.0)
            params.append(-c.y() / a.y());
    } else if (deg == 3) {
        /*
         * For cubic bezier curves r(t) is a quadratic Bezier curve:
         *
         *  2
         * r(t) = Sum Bi,2(t) *Pi = B0,2(t) * P0 + B1,2(t) * P1 + B2,2(t) * P2
         * i=0
         *
         * r(t) = (1-t)^2 * P0 + 2t(1-t) * P1 + t^2 * P2
         *
         * r(t) = (P2 - 2*P1 + P0) * t^2 + (2*P1 - 2*P0) * t + P0
         *
         */
        // calculating the differences between successive control points
        QPointF x0 = first->controlPoint2() - first->point();
        QPointF x1 = second->controlPoint1() - first->controlPoint2();
        QPointF x2 = second->point() - second->controlPoint1();

        // calculating the coefficients
        QPointF a = x2 - 2.0 * x1 + x0;
        QPointF b = 2.0 * x1 - 2.0 * x0;
        QPointF c = x0;

        // calculating parameter t at minimum/maximum in x-direction
        if (a.x() == 0.0) {
            params.append(- c.x() / b.x());
        } else {
            qreal rx = b.x() * b.x() - 4.0 * a.x() * c.x();
            if (rx < 0.0)
                rx = 0.0;
            params.append((-b.x() + sqrt(rx)) / (2.0*a.x()));
            params.append((-b.x() - sqrt(rx)) / (2.0*a.x()));
        }

        // calculating parameter t at minimum/maximum in y-direction
        if (a.y() == 0.0) {
            params.append(- c.y() / b.y());
        } else {
            qreal ry = b.y() * b.y() - 4.0 * a.y() * c.y();
            if (ry < 0.0)
                ry = 0.0;
            params.append((-b.y() + sqrt(ry)) / (2.0*a.y()));
            params.append((-b.y() - sqrt(ry)) / (2.0*a.y()));
        }
    }

    return params;
}

qreal KoPathSegment::Private::distanceFromChord(const QPointF &point) const
{
    // the segments chord
    QPointF chord = second->point() - first->point();
    // the point relative to the segment
    QPointF relPoint = point - first->point();
    // project point to chord
    qreal scale = chord.x() * relPoint.x() + chord.y() * relPoint.y();
    scale /= chord.x() * chord.x() + chord.y() * chord.y();

    // the vector form the point to the projected point on the chord
    QPointF diffVec = scale * chord - relPoint;

    // the unsigned distance of the point to the chord
    qreal distance = sqrt(diffVec.x() * diffVec.x() + diffVec.y() * diffVec.y());

    // determine sign of the distance using the cross product
    if (chord.x()*relPoint.y() - chord.y()*relPoint.x() > 0) {
        return distance;
    } else {
        return -distance;
    }
}

qreal KoPathSegment::Private::chordLength() const
{
    QPointF chord = second->point() - first->point();
    return sqrt(chord.x() * chord.x() + chord.y() * chord.y());
}

QList<QPointF> KoPathSegment::Private::linesIntersection(const KoPathSegment &segment) const
{
    //debugFlake << "intersecting two lines";
    /*
    we have to line segments:

    s1 = A + r * (B-A), s2 = C + s * (D-C) for r,s in [0,1]

        if s1 and s2 intersect we set s1 = s2 so we get these two equations:

    Ax + r * (Bx-Ax) = Cx + s * (Dx-Cx)
    Ay + r * (By-Ay) = Cy + s * (Dy-Cy)

    which we can solve to get r and s
    */
    QList<QPointF> isects;
    QPointF A = first->point();
    QPointF B = second->point();
    QPointF C = segment.first()->point();
    QPointF D = segment.second()->point();

    qreal denom = (B.x() - A.x()) * (D.y() - C.y()) - (B.y() - A.y()) * (D.x() - C.x());
    qreal num_r = (A.y() - C.y()) * (D.x() - C.x()) - (A.x() - C.x()) * (D.y() - C.y());
    // check if lines are collinear
    if (denom == 0.0 && num_r == 0.0)
        return isects;

    qreal num_s = (A.y() - C.y()) * (B.x() - A.x()) - (A.x() - C.x()) * (B.y() - A.y());
    qreal r = num_r / denom;
    qreal s = num_s / denom;

    // check if intersection is inside our line segments
    if (r < 0.0 || r > 1.0)
        return isects;
    if (s < 0.0 || s > 1.0)
        return isects;

    // calculate the actual intersection point
    isects.append(A + r * (B - A));

    return isects;
}


///////////////////
KoPathSegment::KoPathSegment(KoPathPoint * first, KoPathPoint * second)
    : d(new Private(this, first, second))
{
}

KoPathSegment::KoPathSegment(const KoPathSegment & segment)
    : d(new Private(this, 0, 0))
{
    if (! segment.first() || segment.first()->parent())
        setFirst(segment.first());
    else
        setFirst(new KoPathPoint(*segment.first()));

    if (! segment.second() || segment.second()->parent())
        setSecond(segment.second());
    else
        setSecond(new KoPathPoint(*segment.second()));
}

KoPathSegment::KoPathSegment(const QPointF &p0, const QPointF &p1)
    : d(new Private(this, new KoPathPoint(), new KoPathPoint()))
{
    d->first->setPoint(p0);
    d->second->setPoint(p1);
}

KoPathSegment::KoPathSegment(const QPointF &p0, const QPointF &p1, const QPointF &p2)
    : d(new Private(this, new KoPathPoint(), new KoPathPoint()))
{
    d->first->setPoint(p0);
    d->first->setControlPoint2(p1);
    d->second->setPoint(p2);
}

KoPathSegment::KoPathSegment(const QPointF &p0, const QPointF &p1, const QPointF &p2, const QPointF &p3)
    : d(new Private(this, new KoPathPoint(), new KoPathPoint()))
{
    d->first->setPoint(p0);
    d->first->setControlPoint2(p1);
    d->second->setControlPoint1(p2);
    d->second->setPoint(p3);
}

KoPathSegment &KoPathSegment::operator=(const KoPathSegment &rhs)
{
    if (this == &rhs)
        return (*this);

    if (! rhs.first() || rhs.first()->parent())
        setFirst(rhs.first());
    else
        setFirst(new KoPathPoint(*rhs.first()));

    if (! rhs.second() || rhs.second()->parent())
        setSecond(rhs.second());
    else
        setSecond(new KoPathPoint(*rhs.second()));

    return (*this);
}

KoPathSegment::~KoPathSegment()
{
    if (d->first && ! d->first->parent())
        delete d->first;
    if (d->second && ! d->second->parent())
        delete d->second;
    delete d;
}

KoPathPoint *KoPathSegment::first() const
{
    return d->first;
}

void KoPathSegment::setFirst(KoPathPoint *first)
{
    if (d->first && !d->first->parent())
        delete d->first;
    d->first = first;
}

KoPathPoint *KoPathSegment::second() const
{
    return d->second;
}

void KoPathSegment::setSecond(KoPathPoint *second)
{
    if (d->second && !d->second->parent())
        delete d->second;
    d->second = second;
}

bool KoPathSegment::isValid() const
{
    return (d->first && d->second);
}

bool KoPathSegment::operator==(const KoPathSegment &rhs) const
{
    if (!isValid() && !rhs.isValid())
        return true;
    if (isValid() && !rhs.isValid())
        return false;
    if (!isValid() && rhs.isValid())
        return false;

    return (*first() == *rhs.first() && *second() == *rhs.second());
}

int KoPathSegment::degree() const
{
    if (!d->first || !d->second)
        return -1;

    bool c1 = d->first->activeControlPoint2();
    bool c2 = d->second->activeControlPoint1();
    if (!c1 && !c2)
        return 1;
    if (c1 && c2)
        return 3;
    return 2;
}

QPointF KoPathSegment::pointAt(qreal t) const
{
    if (!isValid())
        return QPointF();

    if (degree() == 1) {
        return d->first->point() + t * (d->second->point() - d->first->point());
    } else {
        QPointF splitP;

        d->deCasteljau(t, 0, 0, &splitP, 0, 0);

        return splitP;
    }
}

QRectF KoPathSegment::controlPointRect() const
{
    if (!isValid())
        return QRectF();

    QList<QPointF> points = controlPoints();
    QRectF bbox(points.first(), points.first());
    Q_FOREACH (const QPointF &p, points) {
        bbox.setLeft(qMin(bbox.left(), p.x()));
        bbox.setRight(qMax(bbox.right(), p.x()));
        bbox.setTop(qMin(bbox.top(), p.y()));
        bbox.setBottom(qMax(bbox.bottom(), p.y()));
    }

    if (degree() == 1) {
        // adjust bounding rect of horizontal and vertical lines
        if (bbox.height() == 0.0)
            bbox.setHeight(0.1);
        if (bbox.width() == 0.0)
            bbox.setWidth(0.1);
    }

    return bbox;
}

QRectF KoPathSegment::boundingRect() const
{
    if (!isValid())
        return QRectF();

    QRectF rect = QRectF(d->first->point(), d->second->point()).normalized();

    if (degree() == 1) {
        // adjust bounding rect of horizontal and vertical lines
        if (rect.height() == 0.0)
            rect.setHeight(0.1);
        if (rect.width() == 0.0)
            rect.setWidth(0.1);
    } else {
        /*
         * The basic idea for calculating the axis aligned bounding box (AABB) for bezier segments
         * was found in comp.graphics.algorithms:
         * Use the points at the extrema of the curve to calculate the AABB.
         */
        foreach (qreal t, d->extrema()) {
            if (t >= 0.0 && t <= 1.0) {
                QPointF p = pointAt(t);
                rect.setLeft(qMin(rect.left(), p.x()));
                rect.setRight(qMax(rect.right(), p.x()));
                rect.setTop(qMin(rect.top(), p.y()));
                rect.setBottom(qMax(rect.bottom(), p.y()));
            }
        }
    }

    return rect;
}

QList<QPointF> KoPathSegment::intersections(const KoPathSegment &segment) const
{
    // this function uses a technique known as bezier clipping to find the
    // intersections of the two bezier curves

    QList<QPointF> isects;

    if (!isValid() || !segment.isValid())
        return isects;

    int degree1 = degree();
    int degree2 = segment.degree();

    QRectF myBound = boundingRect();
    QRectF otherBound = segment.boundingRect();
    //debugFlake << "my boundingRect =" << myBound;
    //debugFlake << "other boundingRect =" << otherBound;
    if (!myBound.intersects(otherBound)) {
        //debugFlake << "segments do not intersect";
        return isects;
    }

    // short circuit lines intersection
    if (degree1 == 1 && degree2 == 1) {
        //debugFlake << "intersecting two lines";
        isects += d->linesIntersection(segment);
        return isects;
    }

    // first calculate the fat line L by using the signed distances
    // of the control points from the chord
    qreal dmin, dmax;
    if (degree1 == 1) {
        dmin = 0.0;
        dmax = 0.0;
    } else if (degree1 == 2) {
        qreal d1;
        if (d->first->activeControlPoint2())
            d1 = d->distanceFromChord(d->first->controlPoint2());
        else
            d1 = d->distanceFromChord(d->second->controlPoint1());
        dmin = qMin(qreal(0.0), qreal(0.5 * d1));
        dmax = qMax(qreal(0.0), qreal(0.5 * d1));
    } else {
        qreal d1 = d->distanceFromChord(d->first->controlPoint2());
        qreal d2 = d->distanceFromChord(d->second->controlPoint1());
        if (d1*d2 > 0.0) {
            dmin = 0.75 * qMin(qreal(0.0), qMin(d1, d2));
            dmax = 0.75 * qMax(qreal(0.0), qMax(d1, d2));
        } else {
            dmin = 4.0 / 9.0 * qMin(qreal(0.0), qMin(d1, d2));
            dmax = 4.0 / 9.0 * qMax(qreal(0.0), qMax(d1, d2));
        }
    }

    //debugFlake << "using fat line: dmax =" << dmax << " dmin =" << dmin;

    /*
      the other segment is given as a bezier curve of the form:
     (1) P(t) = sum_i P_i * B_{n,i}(t)
     our chord line is of the form:
     (2) ax + by + c = 0
     we can determine the distance d(t) from any point P(t) to our chord
     by substituting formula (1) into formula (2):
     d(t) = sum_i d_i B_{n,i}(t), where d_i = a*x_i + b*y_i + c
     which forms another explicit bezier curve
     D(t) = (t,d(t)) = sum_i D_i B_{n,i}(t)
     now values of t for which P(t) lies outside of our fat line L
     corresponds to values of t for which D(t) lies above d = dmax or
     below d = dmin
     we can determine parameter ranges of t for which P(t) is guaranteed
     to lie outside of L by identifying ranges of t which the convex hull
     of D(t) lies above dmax or below dmin
    */
    // now calculate the control points of D(t) by using the signed
    // distances of P_i to our chord
    KoPathSegment dt;
    if (degree2 == 1) {
        QPointF p0(0.0, d->distanceFromChord(segment.first()->point()));
        QPointF p1(1.0, d->distanceFromChord(segment.second()->point()));
        dt = KoPathSegment(p0, p1);
    } else if (degree2 == 2) {
        QPointF p0(0.0, d->distanceFromChord(segment.first()->point()));
        QPointF p1 = segment.first()->activeControlPoint2()
                     ? QPointF(0.5, d->distanceFromChord(segment.first()->controlPoint2()))
                     : QPointF(0.5, d->distanceFromChord(segment.second()->controlPoint1()));
        QPointF p2(1.0, d->distanceFromChord(segment.second()->point()));
        dt = KoPathSegment(p0, p1, p2);
    } else if (degree2 == 3) {
        QPointF p0(0.0, d->distanceFromChord(segment.first()->point()));
        QPointF p1(1. / 3., d->distanceFromChord(segment.first()->controlPoint2()));
        QPointF p2(2. / 3., d->distanceFromChord(segment.second()->controlPoint1()));
        QPointF p3(1.0, d->distanceFromChord(segment.second()->point()));
        dt = KoPathSegment(p0, p1, p2, p3);
    } else {
        //debugFlake << "invalid degree of segment -> exiting";
        return isects;
    }

    // get convex hull of the segment D(t)
    QList<QPointF> hull = dt.convexHull();

    // now calculate intersections with the line y1 = dmin, y2 = dmax
    // with the convex hull edges
    int hullCount = hull.count();
    qreal tmin = 1.0, tmax = 0.0;
    bool intersectionsFoundMax = false;
    bool intersectionsFoundMin = false;

    for (int i = 0; i < hullCount; ++i) {
        QPointF p1 = hull[i];
        QPointF p2 = hull[(i+1) % hullCount];
        //debugFlake << "intersecting hull edge (" << p1 << p2 << ")";
        // hull edge is completely above dmax
        if (p1.y() > dmax && p2.y() > dmax)
            continue;
        // hull edge is completely below dmin
        if (p1.y() < dmin && p2.y() < dmin)
            continue;
        if (p1.x() == p2.x()) {
            // vertical edge
            bool dmaxIntersection = (dmax < qMax(p1.y(), p2.y()) && dmax > qMin(p1.y(), p2.y()));
            bool dminIntersection = (dmin < qMax(p1.y(), p2.y()) && dmin > qMin(p1.y(), p2.y()));
            if (dmaxIntersection || dminIntersection) {
                tmin = qMin(tmin, p1.x());
                tmax = qMax(tmax, p1.x());
                if (dmaxIntersection) {
                    intersectionsFoundMax = true;
                    //debugFlake << "found intersection with dmax at " << p1.x() << "," << dmax;
                } else {
                    intersectionsFoundMin = true;
                    //debugFlake << "found intersection with dmin at " << p1.x() << "," << dmin;
                }
            }
        } else if (p1.y() == p2.y()) {
            // horizontal line
            if (p1.y() == dmin || p1.y() == dmax) {
                if (p1.y() == dmin) {
                    intersectionsFoundMin = true;
                    //debugFlake << "found intersection with dmin at " << p1.x() << "," << dmin;
                    //debugFlake << "found intersection with dmin at " << p2.x() << "," << dmin;
                } else {
                    intersectionsFoundMax = true;
                    //debugFlake << "found intersection with dmax at " << p1.x() << "," << dmax;
                    //debugFlake << "found intersection with dmax at " << p2.x() << "," << dmax;
                }
                tmin = qMin(tmin, p1.x());
                tmin = qMin(tmin, p2.x());
                tmax = qMax(tmax, p1.x());
                tmax = qMax(tmax, p2.x());
            }
        } else {
            qreal dx = p2.x() - p1.x();
            qreal dy = p2.y() - p1.y();
            qreal m = dy / dx;
            qreal n = p1.y() - m * p1.x();
            qreal t1 = (dmax - n) / m;
            if (t1 >= 0.0 && t1 <= 1.0) {
                tmin = qMin(tmin, t1);
                tmax = qMax(tmax, t1);
                intersectionsFoundMax = true;
                //debugFlake << "found intersection with dmax at " << t1 << "," << dmax;
            }
            qreal t2 = (dmin - n) / m;
            if (t2 >= 0.0 && t2 < 1.0) {
                tmin = qMin(tmin, t2);
                tmax = qMax(tmax, t2);
                intersectionsFoundMin = true;
                //debugFlake << "found intersection with dmin at " << t2 << "," << dmin;
            }
        }
    }

    bool intersectionsFound = intersectionsFoundMin && intersectionsFoundMax;

    //if (intersectionsFound)
    //    debugFlake << "clipping segment to interval [" << tmin << "," << tmax << "]";

    if (!intersectionsFound || (1.0 - (tmax - tmin)) <= 0.2) {
        //debugFlake << "could not clip enough -> split segment";
        // we could not reduce the interval significantly
        // so split the curve and calculate intersections
        // with the remaining parts
        QPair<KoPathSegment, KoPathSegment> parts = splitAt(0.5);
        if (d->chordLength() < 1e-5)
            isects += parts.first.second()->point();
        else {
            isects += segment.intersections(parts.first);
            isects += segment.intersections(parts.second);
        }
    } else if (qAbs(tmin - tmax) < 1e-5) {
        //debugFlake << "Yay, we found an intersection";
        // the interval is pretty small now, just calculate the intersection at this point
        isects.append(segment.pointAt(tmin));
    } else {
        QPair<KoPathSegment, KoPathSegment> clip1 = segment.splitAt(tmin);
        //debugFlake << "splitting segment at" << tmin;
        qreal t = (tmax - tmin) / (1.0 - tmin);
        QPair<KoPathSegment, KoPathSegment> clip2 = clip1.second.splitAt(t);
        //debugFlake << "splitting second part at" << t << "("<<tmax<<")";
        isects += clip2.first.intersections(*this);
    }

    return isects;
}

KoPathSegment KoPathSegment::mapped(const QTransform &matrix) const
{
    if (!isValid())
        return *this;

    KoPathPoint * p1 = new KoPathPoint(*d->first);
    KoPathPoint * p2 = new KoPathPoint(*d->second);
    p1->map(matrix);
    p2->map(matrix);

    return KoPathSegment(p1, p2);
}

KoPathSegment KoPathSegment::toCubic() const
{
    if (! isValid())
        return KoPathSegment();

    KoPathPoint * p1 = new KoPathPoint(*d->first);
    KoPathPoint * p2 = new KoPathPoint(*d->second);

    if (degree() == 1) {
        p1->setControlPoint2(p1->point() + 0.3 * (p2->point() - p1->point()));
        p2->setControlPoint1(p2->point() + 0.3 * (p1->point() - p2->point()));
    } else if (degree() == 2) {
        /* quadric bezier (a0,a1,a2) to cubic bezier (b0,b1,b2,b3):
        *
        * b0 = a0
        * b1 = a0 + 2/3 * (a1-a0)
        * b2 = a1 + 1/3 * (a2-a1)
        * b3 = a2
        */
        QPointF a1 = p1->activeControlPoint2() ? p1->controlPoint2() : p2->controlPoint1();
        QPointF b1 = p1->point() + 2.0 / 3.0 * (a1 - p1->point());
        QPointF b2 = a1 + 1.0 / 3.0 * (p2->point() - a1);
        p1->setControlPoint2(b1);
        p2->setControlPoint1(b2);
    }

    return KoPathSegment(p1, p2);
}

qreal KoPathSegment::length(qreal error) const
{
    /*
     * This algorithm is implemented based on an idea by Jens Gravesen:
     * "Adaptive subdivision and the length of Bezier curves" mat-report no. 1992-10, Mathematical Institute,
     * The Technical University of Denmark.
     *
     * By subdividing the curve at parameter value t you only have to find the length of a full Bezier curve.
     * If you denote the length of the control polygon by L1 i.e.:
     *   L1 = |P0 P1| +|P1 P2| +|P2 P3|
     *
     * and the length of the cord by L0 i.e.:
     *   L0 = |P0 P3|
     *
     * then
     *   L = 1/2*L0 + 1/2*L1
     *
     * is a good approximation to the length of the curve, and the difference
     *   ERR = L1-L0
     *
     * is a measure of the error. If the error is to large, then you just subdivide curve at parameter value
     * 1/2, and find the length of each half.
     * If m is the number of subdivisions then the error goes to zero as 2^-4m.
     * If you don't have a cubic curve but a curve of degree n then you put
     *   L = (2*L0 + (n-1)*L1)/(n+1)
     */

    int deg = degree();

    if (deg == -1)
        return 0.0;

    QList<QPointF> ctrlPoints = controlPoints();

    // calculate chord length
    qreal chordLen = d->chordLength();

    if (deg == 1) {
        return chordLen;
    }

    // calculate length of control polygon
    qreal polyLength = 0.0;

    for (int i = 0; i < deg; ++i) {
        QPointF ctrlSegment = ctrlPoints[i+1] - ctrlPoints[i];
        polyLength += sqrt(ctrlSegment.x() * ctrlSegment.x() + ctrlSegment.y() * ctrlSegment.y());
    }

    if ((polyLength - chordLen) > error) {
        // the error is still bigger than our tolerance -> split segment
        QPair<KoPathSegment, KoPathSegment> parts = splitAt(0.5);
        return parts.first.length(error) + parts.second.length(error);
    } else {
        // the error is smaller than our tolerance
        if (deg == 3)
            return 0.5 * chordLen + 0.5 * polyLength;
        else
            return (2.0 * chordLen + polyLength) / 3.0;
    }
}

qreal KoPathSegment::lengthAt(qreal t, qreal error) const
{
    if (t == 0.0)
        return 0.0;
    if (t == 1.0)
        return length(error);

    QPair<KoPathSegment, KoPathSegment> parts = splitAt(t);
    return parts.first.length(error);
}

qreal KoPathSegment::paramAtLength(qreal length, qreal tolerance) const
{
    const int deg = degree();
    // invalid degree or invalid specified length
    if (deg < 1 || length <= 0.0) {
        return 0.0;
    }

    if (deg == 1) {
        // make sure we return a maximum value of 1.0
        return qMin(qreal(1.0), length / d->chordLength());
    }

    // for curves we need to make sure, that the specified length
    // value does not exceed the actual length of the segment
    // if that happens, we return 1.0 to avoid infinite looping
    if (length >= d->chordLength() && length >= this->length(tolerance)) {
        return 1.0;
    }

    qreal startT = 0.0; // interval start
    qreal midT = 0.5;   // interval center
    qreal endT = 1.0;   // interval end

    // divide and conquer, split a midpoint and check
    // on which side of the midpoint to continue
    qreal midLength = lengthAt(0.5);
    while (qAbs(midLength - length) / length > tolerance) {
        if (midLength < length)
            startT = midT;
        else
            endT = midT;

        // new interval center
        midT = 0.5 * (startT + endT);
        // length at new interval center
        midLength = lengthAt(midT);
    }

    return midT;
}

bool KoPathSegment::isFlat(qreal tolerance) const
{
    /*
     * Calculate the height of the bezier curve.
     * This is done by rotating the curve so that then chord
     * is parallel to the x-axis and the calculating the
     * parameters t for the extrema of the curve.
     * The curve points at the extrema are then used to
     * calculate the height.
     */
    if (degree() <= 1)
        return true;

    QPointF chord = d->second->point() - d->first->point();
    // calculate angle of chord to the x-axis
    qreal chordAngle = atan2(chord.y(), chord.x());
    QTransform m;
    m.translate(d->first->point().x(), d->first->point().y());
    m.rotate(chordAngle * M_PI / 180.0);
    m.translate(-d->first->point().x(), -d->first->point().y());

    KoPathSegment s = mapped(m);

    qreal minDist = 0.0;
    qreal maxDist = 0.0;

    foreach (qreal t, s.d->extrema()) {
        if (t >= 0.0 && t <= 1.0) {
            QPointF p = pointAt(t);
            qreal dist = s.d->distanceFromChord(p);
            minDist = qMin(dist, minDist);
            maxDist = qMax(dist, maxDist);
        }
    }

    return (maxDist - minDist <= tolerance);
}

QList<QPointF> KoPathSegment::convexHull() const
{
    QList<QPointF> hull;
    int deg = degree();
    if (deg == 1) {
        // easy just the two end points
        hull.append(d->first->point());
        hull.append(d->second->point());
    } else if (deg == 2) {
        // we want to have a counter-clockwise oriented triangle
        // of the three control points
        QPointF chord = d->second->point() - d->first->point();
        QPointF cp = d->first->activeControlPoint2() ? d->first->controlPoint2() : d->second->controlPoint1();
        QPointF relP = cp - d->first->point();
        // check on which side of the chord the control point is
        bool pIsRight = (chord.x() * relP.y() - chord.y() * relP.x() > 0);
        hull.append(d->first->point());
        if (pIsRight)
            hull.append(cp);
        hull.append(d->second->point());
        if (! pIsRight)
            hull.append(cp);
    } else if (deg == 3) {
        // we want a counter-clockwise oriented polygon
        QPointF chord = d->second->point() - d->first->point();
        QPointF relP1 = d->first->controlPoint2() - d->first->point();
        // check on which side of the chord the control points are
        bool p1IsRight = (chord.x() * relP1.y() - chord.y() * relP1.x() > 0);
        hull.append(d->first->point());
        if (p1IsRight)
            hull.append(d->first->controlPoint2());
        hull.append(d->second->point());
        if (! p1IsRight)
            hull.append(d->first->controlPoint2());

        // now we have a counter-clockwise triangle with the points i,j,k
        // we have to check where the last control points lies
        bool rightOfEdge[3];
        QPointF lastPoint = d->second->controlPoint1();
        for (int i = 0; i < 3; ++i) {
            QPointF relP = lastPoint - hull[i];
            QPointF edge = hull[(i+1)%3] - hull[i];
            rightOfEdge[i] = (edge.x() * relP.y() - edge.y() * relP.x() > 0);
        }
        for (int i = 0; i < 3; ++i) {
            int prev = (3 + i - 1) % 3;
            int next = (i + 1) % 3;
            // check if point is only right of the n-th edge
            if (! rightOfEdge[prev] && rightOfEdge[i] && ! rightOfEdge[next]) {
                // insert by breaking the n-th edge
                hull.insert(i + 1, lastPoint);
                break;
            }
            // check if it is right of the n-th and right of the (n+1)-th edge
            if (rightOfEdge[i] && rightOfEdge[next]) {
                // remove both edge, insert two new edges
                hull[i+1] = lastPoint;
                break;
            }
            // check if it is right of n-th and right of (n-1)-th edge
            if (rightOfEdge[i] && rightOfEdge[prev]) {
                hull[i] = lastPoint;
                break;
            }
        }
    }

    return hull;
}

QPair<KoPathSegment, KoPathSegment> KoPathSegment::splitAt(qreal t) const
{
    QPair<KoPathSegment, KoPathSegment> results;
    if (!isValid())
        return results;

    if (degree() == 1) {
        QPointF p = d->first->point() + t * (d->second->point() - d->first->point());
        results.first = KoPathSegment(d->first->point(), p);
        results.second = KoPathSegment(p, d->second->point());
    } else {
        QPointF newCP2, newCP1, splitP, splitCP1, splitCP2;

        d->deCasteljau(t, &newCP2, &splitCP1, &splitP, &splitCP2, &newCP1);

        if (degree() == 2) {
            if (second()->activeControlPoint1()) {
                KoPathPoint *s1p1 = new KoPathPoint(0, d->first->point());
                KoPathPoint *s1p2 = new KoPathPoint(0, splitP);
                s1p2->setControlPoint1(splitCP1);
                KoPathPoint *s2p1 = new KoPathPoint(0, splitP);
                KoPathPoint *s2p2 = new KoPathPoint(0, d->second->point());
                s2p2->setControlPoint1(splitCP2);
                results.first = KoPathSegment(s1p1, s1p2);
                results.second = KoPathSegment(s2p1, s2p2);
            } else {
                results.first = KoPathSegment(d->first->point(), splitCP1, splitP);
                results.second = KoPathSegment(splitP, splitCP2, d->second->point());
            }
        } else {
            results.first = KoPathSegment(d->first->point(), newCP2, splitCP1, splitP);
            results.second = KoPathSegment(splitP, splitCP2, newCP1, d->second->point());
        }
    }

    return results;
}

QList<QPointF> KoPathSegment::controlPoints() const
{
    QList<QPointF> controlPoints;
    controlPoints.append(d->first->point());
    if (d->first->activeControlPoint2())
        controlPoints.append(d->first->controlPoint2());
    if (d->second->activeControlPoint1())
        controlPoints.append(d->second->controlPoint1());
    controlPoints.append(d->second->point());

    return controlPoints;
}

qreal KoPathSegment::nearestPoint(const QPointF &point) const
{
    if (!isValid())
        return -1.0;

    return KisBezierUtils::nearestPoint(controlPoints(), point);
}

KoPathSegment KoPathSegment::interpolate(const QPointF &p0, const QPointF &p1, const QPointF &p2, qreal t)
{
    if (t <= 0.0 || t >= 1.0)
        return KoPathSegment();

    /*
        B(t) = [x2 y2] = (1-t)^2*P0 + 2t*(1-t)*P1 + t^2*P2

               B(t) - (1-t)^2*P0 - t^2*P2
         P1 =  --------------------------
                       2t*(1-t)
    */

    QPointF c1 = p1 - (1.0-t) * (1.0-t)*p0 - t * t * p2;

    qreal denom = 2.0 * t * (1.0-t);

    c1.rx() /= denom;
    c1.ry() /= denom;

    return KoPathSegment(p0, c1, p2);
}

#if 0
void KoPathSegment::printDebug() const
{
    int deg = degree();
    debugFlake << "degree:" << deg;
    if (deg < 1)
        return;

    debugFlake << "P0:" << d->first->point();
    if (deg == 1) {
        debugFlake << "P2:" << d->second->point();
    } else if (deg == 2) {
        if (d->first->activeControlPoint2())
            debugFlake << "P1:" << d->first->controlPoint2();
        else
            debugFlake << "P1:" << d->second->controlPoint1();
        debugFlake << "P2:" << d->second->point();
    } else if (deg == 3) {
        debugFlake << "P1:" << d->first->controlPoint2();
        debugFlake << "P2:" << d->second->controlPoint1();
        debugFlake << "P3:" << d->second->point();
    }
}
#endif
