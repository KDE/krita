/*
 *  SPDX-FileCopyrightText: 2008-2009 Jan Hambrecht <jaham@gmx.net>
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBezierUtils.h"

#include <tuple>
#include <QStack>
#include <QDebug>
#include "kis_debug.h"

#include "KisBezierPatch.h"
#include <iostream>

#include <config-gsl.h>

#ifdef HAVE_GSL
#include <gsl/gsl_multimin.h>
#endif

#include <Eigen/Dense>

namespace KisBezierUtils
{

QVector<qreal> linearizeCurve(const QPointF p0, const QPointF p1, const QPointF p2, const QPointF p3, const qreal eps)
{
    const qreal minStepSize = 2.0 / kisDistance(p0, p3);

    QVector<qreal> steps;
    steps << 0.0;


    QStack<std::tuple<QPointF, QPointF, qreal>> stackedPoints;
    stackedPoints.push(std::make_tuple(p3, 3 * (p3 - p2), 1.0));

    QPointF lastP = p0;
    QPointF lastD = 3 * (p1 - p0);
    qreal lastT = 0.0;

    while (!stackedPoints.isEmpty()) {
        QPointF p = std::get<0>(stackedPoints.top());
        QPointF d = std::get<1>(stackedPoints.top());
        qreal t = std::get<2>(stackedPoints.top());

        if (t - lastT < minStepSize ||
                isLinearSegmentByDerivatives(lastP, lastD, p, d, eps)) {

            lastP = p;
            lastD = d;
            lastT = t;
            steps << t;
            stackedPoints.pop();
        } else {
            t = 0.5 * (lastT + t);
            p = bezierCurve(p0, p1, p2, p3, t);
            d = bezierCurveDeriv(p0, p1, p2, p3, t);

            stackedPoints.push(std::make_tuple(p, d, t));
        }
    }

    return steps;
}

QVector<qreal> mergeLinearizationSteps(const QVector<qreal> &a, const QVector<qreal> &b)
{
    QVector<qreal> result;

    std::merge(a.constBegin(), a.constEnd(),
               b.constBegin(), b.constEnd(),
               std::back_inserter(result));
    result.erase(
                std::unique(result.begin(), result.end(),
                            [] (qreal x, qreal y) { return qFuzzyCompare(x, y); }),
            result.end());

    return result;
}

class BezierSegment
{
private:
    /// Maximal recursion depth for finding root params
    const int MaxRecursionDepth = 64;
    /// Flatness tolerance for finding root params
    const qreal FlatnessTolerance = ldexp(1.0,-MaxRecursionDepth-1);

public:
    BezierSegment(int degree = 0, QPointF *p = 0)
    {
        if (degree) {
            for (int i = 0; i <= degree; ++i)
                points.append(p[i]);
        }
    }

    void setDegree(int degree)
    {
        points.clear();
        if (degree) {
            for (int i = 0; i <= degree; ++i)
                points.append(QPointF());
        }
    }

    int degree() const
    {
        return points.count() - 1;
    }

    QPointF point(int index) const
    {
        if (static_cast<int>(index) > degree())
            return QPointF();

        return points[index];
    }

    void setPoint(int index, const QPointF &p)
    {
        if (static_cast<int>(index) > degree())
            return;

        points[index] = p;
    }

    QPointF evaluate(qreal t, BezierSegment *left, BezierSegment *right) const
    {
        int deg = degree();
        if (! deg)
            return QPointF();

        QVector<QVector<QPointF> > Vtemp(deg + 1);
        for (int i = 0; i <= deg; ++i)
            Vtemp[i].resize(deg + 1);

        /* Copy control points  */
        for (int j = 0; j <= deg; j++) {
            Vtemp[0][j] = points[j];
        }

        /* Triangle computation */
        for (int i = 1; i <= deg; i++) {
            for (int j =0 ; j <= deg - i; j++) {
                Vtemp[i][j].rx() = (1.0 - t) * Vtemp[i-1][j].x() + t * Vtemp[i-1][j+1].x();
                Vtemp[i][j].ry() = (1.0 - t) * Vtemp[i-1][j].y() + t * Vtemp[i-1][j+1].y();
            }
        }

        if (left) {
            left->setDegree(deg);
            for (int j = 0; j <= deg; j++) {
                left->setPoint(j, Vtemp[j][0]);
            }
        }
        if (right) {
            right->setDegree(deg);
            for (int j = 0; j <= deg; j++) {
                right->setPoint(j, Vtemp[deg-j][j]);
            }
        }

        return (Vtemp[deg][0]);
    }

    QList<qreal> roots(int depth = 0) const
    {
        QList<qreal> rootParams;

        if (! degree())
            return rootParams;

        // Calculate how often the control polygon crosses the x-axis
        // This is the upper limit for the number of roots.
        int xAxisCrossings = controlPolygonZeros(points);

        if (! xAxisCrossings) {
            // No solutions.
            return rootParams;
        }
        else if (xAxisCrossings == 1) {
            // Exactly one solution.

            // Stop recursion when the tree is deep enough
            if (depth >= MaxRecursionDepth) {
                // if deep enough, return 1 solution at midpoint
                rootParams.append((points.first().x() + points.last().x()) / 2.0);
                return rootParams;
            }
            else if (isFlat(FlatnessTolerance)) {
                // Calculate intersection of chord with x-axis.
                QPointF chord = points.last() - points.first();
                QPointF segStart = points.first();
                rootParams.append((chord.x() * segStart.y() - chord.y() * segStart.x()) / - chord.y());
                return rootParams;
            }
        }

        // Many solutions. Do recursive midpoint subdivision.
        BezierSegment left, right;
        evaluate(0.5, &left, &right);
        rootParams += left.roots(depth+1);
        rootParams += right.roots(depth+1);

        return rootParams;
    }

    static uint controlPolygonZeros(const QList<QPointF> &controlPoints)
    {
        int controlPointCount = controlPoints.count();
        if (controlPointCount < 2)
            return 0;

        int signChanges = 0;

        int currSign = controlPoints[0].y() < 0.0 ? -1 : 1;
        int oldSign;

        for (short i = 1; i < controlPointCount; ++i) {
            oldSign = currSign;
            currSign = controlPoints[i].y() < 0.0 ? -1 : 1;

            if (currSign != oldSign) {
                ++signChanges;
            }
        }


        return signChanges;
    }

    int isFlat (qreal tolerance) const
    {
        int deg = degree();

        // Find the  perpendicular distance from each interior control point to
        // the line connecting points[0] and points[degree]
        qreal *distance = new qreal[deg + 1];

        // Derive the implicit equation for line connecting first and last control points
        qreal a = points[0].y() - points[deg].y();
        qreal b = points[deg].x() - points[0].x();
        qreal c = points[0].x() * points[deg].y() - points[deg].x() * points[0].y();

        qreal abSquared = (a * a) + (b * b);

        for (int i = 1; i < deg; i++) {
            // Compute distance from each of the points to that line
            distance[i] = a * points[i].x() + b * points[i].y() + c;
            if (distance[i] > 0.0) {
                distance[i] = (distance[i] * distance[i]) / abSquared;
            }
            if (distance[i] < 0.0) {
                distance[i] = -((distance[i] * distance[i]) / abSquared);
            }
        }


        // Find the largest distance
        qreal max_distance_above = 0.0;
        qreal max_distance_below = 0.0;
        for (int i = 1; i < deg; i++) {
            if (distance[i] < 0.0) {
                max_distance_below = qMin(max_distance_below, distance[i]);
            }
            if (distance[i] > 0.0) {
                max_distance_above = qMax(max_distance_above, distance[i]);
            }
        }
        delete [] distance;

        // Implicit equation for zero line
        qreal a1 = 0.0;
        qreal b1 = 1.0;
        qreal c1 = 0.0;

        // Implicit equation for "above" line
        qreal a2 = a;
        qreal b2 = b;
        qreal c2 = c + max_distance_above;

        qreal det = a1 * b2 - a2 * b1;
        qreal dInv = 1.0/det;

        qreal intercept_1 = (b1 * c2 - b2 * c1) * dInv;

        // Implicit equation for "below" line
        a2 = a;
        b2 = b;
        c2 = c + max_distance_below;

        det = a1 * b2 - a2 * b1;
        dInv = 1.0/det;

        qreal intercept_2 = (b1 * c2 - b2 * c1) * dInv;

        // Compute intercepts of bounding box
        qreal left_intercept = qMin(intercept_1, intercept_2);
        qreal right_intercept = qMax(intercept_1, intercept_2);

        qreal error = 0.5 * (right_intercept-left_intercept);

        return (error < tolerance);
    }

#ifndef NDEBUG
    void printDebug() const
    {
        int index = 0;
        Q_FOREACH (const QPointF &p, points) {
            qDebug() << QString("P%1 ").arg(index++) << p;
        }
    }
#endif

private:
    QList<QPointF> points;
};

qreal nearestPoint(const QList<QPointF> controlPoints, const QPointF &point, qreal *resultDistance, QPointF *resultPoint)
{
    const int deg = controlPoints.size() - 1;

    // use shortcut for line segments
    if (deg == 1) {
        // the segments chord
        QPointF chord = controlPoints.last() - controlPoints.first();
        // the point relative to the segment
        QPointF relPoint = point - controlPoints.first();
        // project point to chord (dot product)
        qreal scale = chord.x() * relPoint.x() + chord.y() * relPoint.y();
        // normalize using the chord length
        scale /= chord.x() * chord.x() + chord.y() * chord.y();

        if (scale < 0.0) {
            return 0.0;
        } else if (scale > 1.0) {
            return 1.0;
        } else {
            return scale;
        }
    }

    /* This function solves the "nearest point on curve" problem. That means, it
    * calculates the point q (to be precise: it's parameter t) on this segment, which
    * is located nearest to the input point P.
    * The basic idea is best described (because it is freely available) in "Phoenix:
    * An Interactive Curve Design System Based on the Automatic Fitting of
    * Hand-Sketched Curves", Philip J. Schneider (Master thesis, University of
    * Washington).
    *
    * For the nearest point q = C(t) on this segment, the first derivative is
    * orthogonal to the distance vector "C(t) - P". In other words we are looking for
    * solutions of f(t) = (C(t) - P) * C'(t) = 0.
    * (C(t) - P) is a nth degree curve, C'(t) a n-1th degree curve => f(t) is a
    * (2n - 1)th degree curve and thus has up to 2n - 1 distinct solutions.
    * We solve the problem f(t) = 0 by using something called "Approximate Inversion Method".
    * Let's write f(t) explicitly (with c_i = p_i - P and d_j = p_{j+1} - p_j):
    *
    *         n                     n-1
    * f(t) = SUM c_i * B^n_i(t)  *  SUM d_j * B^{n-1}_j(t)
    *        i=0                    j=0
    *
    *         n  n-1
    *      = SUM SUM w_{ij} * B^{2n-1}_{i+j}(t)
    *        i=0 j=0
    *
    * with w_{ij} = c_i * d_j * z_{ij} and
    *
    *          BinomialCoeff(n, i) * BinomialCoeff(n - i ,j)
    * z_{ij} = -----------------------------------------------
    *                   BinomialCoeff(2n - 1, i + j)
    *
    * This Bernstein-Bezier polynom representation can now be solved for its roots.
    */

    QList<QPointF> ctlPoints = controlPoints;

    // Calculate the c_i = point(i) - P.
    QPointF * c_i = new QPointF[ deg + 1 ];

    for (int i = 0; i <= deg; ++i) {
        c_i[ i ] = ctlPoints[ i ] - point;
    }

    // Calculate the d_j = point(j + 1) - point(j).
    QPointF *d_j = new QPointF[deg];

    for (int j = 0; j <= deg - 1; ++j) {
        d_j[j] = 3.0 * (ctlPoints[j+1] - ctlPoints[j]);
    }

    // Calculate the dot products of c_i and d_i.
    qreal *products = new qreal[deg * (deg + 1)];

    for (int j = 0; j <= deg - 1; ++j) {
        for (int i = 0; i <= deg; ++i) {
            products[j * (deg + 1) + i] = d_j[j].x() * c_i[i].x() + d_j[j].y() * c_i[i].y();
        }
    }

    // We don't need the c_i and d_i anymore.
    delete[] d_j ;
    delete[] c_i ;

    // Calculate the control points of the new 2n-1th degree curve.
    BezierSegment newCurve;
    newCurve.setDegree(2 * deg - 1);
    // Set up control points in the (u, f(u))-plane.
    for (unsigned short u = 0; u <= 2 * deg - 1; ++u) {
        newCurve.setPoint(u, QPointF(static_cast<qreal>(u) / static_cast<qreal>(2 * deg - 1), 0.0));
    }

    // Precomputed "z" for cubics
    static const qreal z3[3*4] = {1.0, 0.6, 0.3, 0.1, 0.4, 0.6, 0.6, 0.4, 0.1, 0.3, 0.6, 1.0};
    // Precomputed "z" for quadrics
    static const qreal z2[2*3] = {1.0, 2./3., 1./3., 1./3., 2./3., 1.0};

    const qreal *z = deg == 3 ? z3 : z2;

    // Set f(u)-values.
    for (int k = 0; k <= 2 * deg - 1; ++k) {
        int min = qMin(k, deg);

        for (unsigned short i = qMax(0, k - (deg - 1)); i <= min; ++i) {
            unsigned short j = k - i;

            // p_k += products[j][i] * z[j][i].
            QPointF currentPoint = newCurve.point(k);
            currentPoint.ry() += products[j * (deg + 1) + i] * z[j * (deg + 1) + i];
            newCurve.setPoint(k, currentPoint);
        }
    }

    // We don't need the c_i/d_i dot products and the z_{ij} anymore.
    delete[] products;

    // Find roots.
    QList<qreal> rootParams = newCurve.roots();

    // Now compare the distances of the candidate points.

    // First candidate is the previous knot.
    qreal distanceSquared = kisSquareDistance(point, controlPoints.first());
    qreal minDistanceSquared = distanceSquared;
    qreal resultParam = 0.0;
    if (resultDistance) {
        *resultDistance = std::sqrt(distanceSquared);
    }
    if (resultPoint) {
        *resultPoint = controlPoints.first();
    }

    // Iterate over the found candidate params.
    Q_FOREACH (qreal root, rootParams) {
        const QPointF rootPoint = bezierCurve(controlPoints, root);
        distanceSquared = kisSquareDistance(point, rootPoint);

        if (distanceSquared < minDistanceSquared) {
            minDistanceSquared = distanceSquared;
            resultParam = root;
            if (resultDistance) {
                *resultDistance = std::sqrt(distanceSquared);
            }
            if (resultPoint) {
                *resultPoint = rootPoint;
            }
        }
    }

    // Last candidate is the knot.
    distanceSquared = kisSquareDistance(point, controlPoints.last());
    if (distanceSquared < minDistanceSquared) {
        minDistanceSquared = distanceSquared;
        resultParam = 1.0;
        if (resultDistance) {
            *resultDistance = std::sqrt(distanceSquared);
        }
        if (resultPoint) {
            *resultPoint = controlPoints.last();
        }
    }

    return resultParam;
}

int controlPolygonZeros(const QList<QPointF> &controlPoints)
{
    return static_cast<int>(BezierSegment::controlPolygonZeros(controlPoints));
}

namespace {

struct Params2D {
    QPointF p0, p1, p2, p3; // top curve
    QPointF q0, q1, q2, q3; // bottom curve
    QPointF r0, r1, r2, r3; // left curve
    QPointF s0, s1, s2, s3; // right curve

    QPointF dstPoint;
};

struct LevelBasedPatchMethod
{
    LevelBasedPatchMethod(qreal u, qreal v, const Params2D &p)
    {
        M_3 << 1, 0, 0, 0,
             -3, 3, 0, 0,
              3, -6, 3, 0,
             -1, 3, -3, 1;

        M_3rel2abs << 1, 0, 0, 0,
                      1, 1, 0, 0,
                      0, 0, 1, 1,
                      0, 0, 0, 1;

        M_1 << -1,  1,  0, 0,
                1, -1, -1, 1;

        PQ_left << p.p0.x(), p.p0.y(),
                   p.p1.x(), p.p1.y(),
                   p.q0.x(), p.q0.y(),
                   p.q1.x(), p.q1.y();

        PQ_right << p.p3.x(), p.p3.y(),
                    p.p2.x(), p.p2.y(),
                    p.q3.x(), p.q3.y(),
                    p.q2.x(), p.q2.y();

        RS_top << p.r0.x(), p.r0.y(),
                  p.r1.x(), p.r1.y(),
                  p.s0.x(), p.s0.y(),
                  p.s1.x(), p.s1.y();

        RS_bottom << p.r3.x(), p.r3.y(),
                     p.r2.x(), p.r2.y(),
                     p.s3.x(), p.s3.y(),
                     p.s2.x(), p.s2.y();

        R << p.r0.x(), p.r0.y(),
             p.r1.x(), p.r1.y(),
             p.r2.x(), p.r2.y(),
             p.r3.x(), p.r3.y();

        S << p.s0.x(), p.s0.y(),
             p.s1.x(), p.s1.y(),
             p.s2.x(), p.s2.y(),
             p.s3.x(), p.s3.y();

        P << p.p0.x(), p.p0.y(),
             p.p1.x(), p.p1.y(),
             p.p2.x(), p.p2.y(),
             p.p3.x(), p.p3.y();

        Q << p.q0.x(), p.q0.y(),
             p.q1.x(), p.q1.y(),
             p.q2.x(), p.q2.y(),
             p.q3.x(), p.q3.y();

        T_u3 << 1, u, pow2(u), pow3(u);
        T_v3 << 1, v, pow2(v), pow3(v);
        T_dot_u3 << 0, 1, 2 * u, 3 * pow2(u);
        T_dot_v3 << 0, 1, 2 * v, 3 * pow2(v);

        T_u1 << 1, u;
        T_v1 << 1, v;
        T_dot_u1 << 0, 1;
        T_dot_v1 << 0, 1;
    }

    Eigen::Matrix4d M_3;
    Eigen::Matrix4d M_3rel2abs;
    Eigen::Matrix<double, 2, 4> M_1;


    Eigen::Matrix<double, 4, 2> PQ_left;
    Eigen::Matrix<double, 4, 2> PQ_right;
    Eigen::Matrix<double, 4, 2> RS_top;
    Eigen::Matrix<double, 4, 2> RS_bottom;

    Eigen::Matrix<double, 4, 2> R;
    Eigen::Matrix<double, 4, 2> S;
    Eigen::Matrix<double, 4, 2> P;
    Eigen::Matrix<double, 4, 2> Q;

    Eigen::Matrix<double, 1, 4> T_u3;
    Eigen::Matrix<double, 1, 4> T_v3;
    Eigen::Matrix<double, 1, 4> T_dot_u3;
    Eigen::Matrix<double, 1, 4> T_dot_v3;

    Eigen::Matrix<double, 1, 2> T_u1;
    Eigen::Matrix<double, 1, 2> T_v1;
    Eigen::Matrix<double, 1, 2> T_dot_u1;
    Eigen::Matrix<double, 1, 2> T_dot_v1;

    QPointF value() const {
        Eigen::Matrix<double, 1, 2> L_1 = T_v3 * M_3 * R;
        Eigen::Matrix<double, 1, 2> L_2 = T_v1 * M_1 * PQ_left;
        Eigen::Matrix<double, 1, 2> L_3 = T_v1 * M_1 * PQ_right;
        Eigen::Matrix<double, 1, 2> L_4 = T_v3 * M_3 * S;

        Eigen::Matrix<double, 4, 2> L_controls;
        L_controls << L_1, L_2, L_3, L_4;

        Eigen::Matrix<double, 1, 2> L = T_u3 * M_3 * M_3rel2abs * L_controls;


        Eigen::Matrix<double, 1, 2> H_1 = T_u3 * M_3 * P;
        Eigen::Matrix<double, 1, 2> H_2 = T_u1 * M_1 * RS_top;
        Eigen::Matrix<double, 1, 2> H_3 = T_u1 * M_1 * RS_bottom;
        Eigen::Matrix<double, 1, 2> H_4 = T_u3 * M_3 * Q;

        Eigen::Matrix<double, 4, 2> H_controls;
        H_controls << H_1, H_2, H_3, H_4;

        Eigen::Matrix<double, 1, 2> H = T_v3 * M_3 * M_3rel2abs * H_controls;

        Eigen::Matrix<double, 1, 2> result = 0.5 * (L + H);

        return QPointF(result(0,0), result(0,1));
    }

    QPointF diffU() const {
        Eigen::Matrix<double, 1, 2> L_1 = T_v3 * M_3 * R;
        Eigen::Matrix<double, 1, 2> L_2 = T_v1 * M_1 * PQ_left;
        Eigen::Matrix<double, 1, 2> L_3 = T_v1 * M_1 * PQ_right;
        Eigen::Matrix<double, 1, 2> L_4 = T_v3 * M_3 * S;

        Eigen::Matrix<double, 4, 2> L_controls;
        L_controls << L_1, L_2, L_3, L_4;

        Eigen::Matrix<double, 1, 2> L = T_dot_u3 * M_3 * M_3rel2abs * L_controls;


        Eigen::Matrix<double, 1, 2> H_1 = T_dot_u3 * M_3 * P;
        Eigen::Matrix<double, 1, 2> H_2 = T_dot_u1 * M_1 * RS_top;
        Eigen::Matrix<double, 1, 2> H_3 = T_dot_u1 * M_1 * RS_bottom;
        Eigen::Matrix<double, 1, 2> H_4 = T_dot_u3 * M_3 * Q;

        Eigen::Matrix<double, 4, 2> H_controls;
        H_controls << H_1, H_2, H_3, H_4;

        Eigen::Matrix<double, 1, 2> H = T_v3 * M_3 * M_3rel2abs * H_controls;

        Eigen::Matrix<double, 1, 2> result = 0.5 * (L + H);
        return QPointF(result(0,0), result(0,1));
    }

    QPointF diffV() const {
        Eigen::Matrix<double, 1, 2> L_1 = T_dot_v3 * M_3 * R;
        Eigen::Matrix<double, 1, 2> L_2 = T_dot_v1 * M_1 * PQ_left;
        Eigen::Matrix<double, 1, 2> L_3 = T_dot_v1 * M_1 * PQ_right;
        Eigen::Matrix<double, 1, 2> L_4 = T_dot_v3 * M_3 * S;

        Eigen::Matrix<double, 4, 2> L_controls;
        L_controls << L_1, L_2, L_3, L_4;

        Eigen::Matrix<double, 1, 2> L = T_u3 * M_3 * M_3rel2abs * L_controls;

        Eigen::Matrix<double, 1, 2> H_1 = T_u3 * M_3 * P;
        Eigen::Matrix<double, 1, 2> H_2 = T_u1 * M_1 * RS_top;
        Eigen::Matrix<double, 1, 2> H_3 = T_u1 * M_1 * RS_bottom;
        Eigen::Matrix<double, 1, 2> H_4 = T_u3 * M_3 * Q;

        Eigen::Matrix<double, 4, 2> H_controls;
        H_controls << H_1, H_2, H_3, H_4;

        Eigen::Matrix<double, 1, 2> H = T_dot_v3 * M_3 * M_3rel2abs * H_controls;

        Eigen::Matrix<double, 1, 2> result = 0.5 * (L + H);

        return QPointF(result(0,0), result(0,1));
    }

};

struct SvgPatchMethod
{
private:

    /**
     * TODO: optimize these function somehow!
     */

    static QPointF meshForwardMapping(qreal u, qreal v, const Params2D &p) {
        return p.r0 + pow3(u)*v*(p.p0 - 3*p.p1 + 3*p.p2 - p.p3 - p.q0 + 3*p.q1 - 3*p.q2 + p.q3) + pow3(u)*(-p.p0 + 3*p.p1 - 3*p.p2 + p.p3) + pow2(u)*v*(-3*p.p0 + 6*p.p1 - 3*p.p2 + 3*p.q0 - 6*p.q1 + 3*p.q2) + pow2(u)*(3*p.p0 - 6*p.p1 + 3*p.p2) + u*pow3(v)*(p.r0 - 3*p.r1 + 3*p.r2 - p.r3 - p.s0 + 3*p.s1 - 3*p.s2 + p.s3) + u*pow2(v)*(-3*p.r0 + 6*p.r1 - 3*p.r2 + 3*p.s0 - 6*p.s1+ 3*p.s2) + u*v*(2*p.p0 - 3*p.p1 + p.p3 - 2*p.q0 + 3*p.q1 - p.q3 + 3*p.r0 - 3*p.r1 - 3*p.s0 + 3*p.s1) + u*(-2*p.p0 + 3*p.p1 - p.p3 - p.r0 + p.s0) + pow3(v)*(-p.r0 + 3*p.r1 - 3*p.r2 + p.r3) + pow2(v)*(3*p.r0 - 6*p.r1 + 3*p.r2) + v*(-3*p.r0 + 3*p.r1);
    }

    static QPointF meshForwardMappingDiffU(qreal u, qreal v, const Params2D &p) {
        return -2*p.p0 + 3*p.p1 - p.p3 - p.r0 + p.s0 + pow2(u)*v*(3*p.p0 - 9*p.p1 + 9*p.p2 - 3*p.p3 - 3*p.q0 + 9*p.q1 - 9*p.q2 + 3*p.q3) + pow2(u)*(-3*p.p0 + 9*p.p1 - 9*p.p2 + 3*p.p3) + u*v*(-6*p.p0 + 12*p.p1 - 6*p.p2 + 6*p.q0 - 12*p.q1 + 6*p.q2) + u*(6*p.p0 - 12*p.p1 + 6*p.p2) + pow3(v)*(p.r0 - 3*p.r1 + 3*p.r2 - p.r3 - p.s0 + 3*p.s1 - 3*p.s2 + p.s3) + pow2(v)*(-3*p.r0 + 6*p.r1 - 3*p.r2 + 3*p.s0 - 6*p.s1 + 3*p.s2) + v*(2*p.p0 - 3*p.p1 + p.p3 - 2*p.q0 + 3*p.q1 - p.q3 + 3*p.r0 - 3*p.r1 - 3*p.s0 + 3*p.s1);
    }

    static QPointF meshForwardMappingDiffV(qreal u, qreal v, const Params2D &p) {
        return -3*p.r0 + 3*p.r1 + pow3(u)*(p.p0 - 3*p.p1 + 3*p.p2 - p.p3 - p.q0 + 3*p.q1 - 3*p.q2 + p.q3) + pow2(u)*(-3*p.p0 + 6*p.p1 - 3*p.p2 + 3*p.q0 - 6*p.q1 + 3*p.q2) + u*pow2(v)*(3*p.r0 - 9*p.r1 + 9*p.r2 - 3*p.r3 - 3*p.s0 + 9*p.s1 - 9*p.s2 + 3*p.s3) + u*v*(-6*p.r0 + 12*p.r1 - 6*p.r2 + 6*p.s0 - 12*p.s1 + 6*p.s2) + u*(2*p.p0 - 3*p.p1 + p.p3 - 2*p.q0 + 3*p.q1 - p.q3 + 3*p.r0 - 3*p.r1 - 3*p.s0 + 3*p.s1) + pow2(v)*(-3*p.r0 + 9*p.r1 - 9*p.r2 + 3*p.r3) + v*(6*p.r0 - 12*p.r1 + 6*p.r2);
    }

    qreal u = 0.0;
    qreal v = 0.0;
    const Params2D p;

public:
    SvgPatchMethod(qreal _u, qreal _v, const Params2D &_p)
        : u(_u), v(_v), p(_p)
    {
    }

    QPointF value() const {
        return meshForwardMapping(u, v, p);
    }

    QPointF diffU() const {
        return meshForwardMappingDiffU(u, v, p);
    }

    QPointF diffV() const {
        return meshForwardMappingDiffV(u, v, p);
    }

};

template <class PatchMethod>
double my_f(const gsl_vector * x, void *paramsPtr)
{
    const Params2D *params = static_cast<const Params2D*>(paramsPtr);
    const QPointF pos(gsl_vector_get(x, 0), gsl_vector_get(x, 1));

    PatchMethod mat(pos.x(), pos.y(), *params);
    const QPointF S = mat.value();

    return kisSquareDistance(S, params->dstPoint);
}

template <class PatchMethod>
void my_fdf (const gsl_vector *x, void *paramsPtr, double *f, gsl_vector *df)
{
    const Params2D *params = static_cast<const Params2D*>(paramsPtr);
    const QPointF pos(gsl_vector_get(x, 0), gsl_vector_get(x, 1));

    PatchMethod mat(pos.x(), pos.y(), *params);
    const QPointF S = mat.value();
    const QPointF dU = mat.diffU();
    const QPointF dV = mat.diffV();

    *f = kisSquareDistance(S, params->dstPoint);

    gsl_vector_set(df, 0,
                   2 * (S.x() - params->dstPoint.x()) * dU.x() +
                   2 * (S.y() - params->dstPoint.y()) * dU.y());
    gsl_vector_set(df, 1,
                   2 * (S.x() - params->dstPoint.x()) * dV.x() +
                   2 * (S.y() - params->dstPoint.y()) * dV.y());
}

template <class PatchMethod>
void my_df (const gsl_vector *x, void *paramsPtr,
            gsl_vector *df)
{
    const Params2D *params = static_cast<const Params2D*>(paramsPtr);
    const QPointF pos(gsl_vector_get(x, 0), gsl_vector_get(x, 1));

    PatchMethod mat(pos.x(), pos.y(), *params);
    const QPointF S = mat.value();
    const QPointF dU = mat.diffU();
    const QPointF dV = mat.diffV();

    gsl_vector_set(df, 0,
                   2 * (S.x() - params->dstPoint.x()) * dU.x() +
                   2 * (S.y() - params->dstPoint.y()) * dU.y());
    gsl_vector_set(df, 1,
                   2 * (S.x() - params->dstPoint.x()) * dV.x() +
                   2 * (S.y() - params->dstPoint.y()) * dV.y());
}
}

template <class PatchMethod>
QPointF calculateLocalPosImpl(const std::array<QPointF, 12> &points, const QPointF &globalPoint)
{
    QRectF patchBounds;

    for (auto it = points.begin(); it != points.end(); ++it) {
        KisAlgebra2D::accumulateBounds(*it, &patchBounds);
    }

    const QPointF approxStart = KisAlgebra2D::absoluteToRelative(globalPoint, patchBounds);
    KIS_SAFE_ASSERT_RECOVER_NOOP(QRectF(0,0,1.0,1.0).contains(approxStart));

#ifdef HAVE_GSL
    const gsl_multimin_fdfminimizer_type *T =
        gsl_multimin_fdfminimizer_vector_bfgs2;
    gsl_multimin_fdfminimizer *s = 0;
    gsl_vector *x;
    gsl_multimin_function_fdf minex_func;

    size_t iter = 0;
    int status;

    /* Starting point */
    x = gsl_vector_alloc (2);
    gsl_vector_set (x, 0, approxStart.x());
    gsl_vector_set (x, 1, approxStart.y());

    Params2D p;

    p.p0 = points[KisBezierPatch::TL];
    p.p1 = points[KisBezierPatch::TL_HC];
    p.p2 = points[KisBezierPatch::TR_HC];
    p.p3 = points[KisBezierPatch::TR];

    p.q0 = points[KisBezierPatch::BL];
    p.q1 = points[KisBezierPatch::BL_HC];
    p.q2 = points[KisBezierPatch::BR_HC];
    p.q3 = points[KisBezierPatch::BR];

    p.r0 = points[KisBezierPatch::TL];
    p.r1 = points[KisBezierPatch::TL_VC];
    p.r2 = points[KisBezierPatch::BL_VC];
    p.r3 = points[KisBezierPatch::BL];

    p.s0 = points[KisBezierPatch::TR];
    p.s1 = points[KisBezierPatch::TR_VC];
    p.s2 = points[KisBezierPatch::BR_VC];
    p.s3 = points[KisBezierPatch::BR];

    p.dstPoint = globalPoint;

    /* Initialize method and iterate */
    minex_func.n = 2;
    minex_func.f = my_f<PatchMethod>;
    minex_func.params = (void*)&p;
    minex_func.df = my_df<PatchMethod>;
    minex_func.fdf = my_fdf<PatchMethod>;

    s = gsl_multimin_fdfminimizer_alloc (T, 2);
    gsl_multimin_fdfminimizer_set (s, &minex_func, x, 0.01, 0.1);

    QPointF result;


    result.rx() = gsl_vector_get (s->x, 0);
    result.ry() = gsl_vector_get (s->x, 1);

    do
    {
        iter++;
        status = gsl_multimin_fdfminimizer_iterate(s);

        if (status)
            break;

        status = gsl_multimin_test_gradient (s->gradient, 1e-4);

        result.rx() = gsl_vector_get (s->x, 0);
        result.ry() = gsl_vector_get (s->x, 1);

        if (status == GSL_SUCCESS)
        {
            result.rx() = gsl_vector_get (s->x, 0);
            result.ry() = gsl_vector_get (s->x, 1);
            //qDebug() << "******* Converged to minimum" << ppVar(result);

        }
    }
    while (status == GSL_CONTINUE && iter < 10000);

//    ENTER_FUNCTION()<< ppVar(iter) << ppVar(globalPoint) << ppVar(result);
//    ENTER_FUNCTION() << ppVar(meshForwardMapping(result.x(), result.y(), p));

    gsl_vector_free(x);
    gsl_multimin_fdfminimizer_free (s);

    return result;
#else
    return approxStart;
#endif
}

template <class PatchMethod>
QPointF calculateGlobalPosImpl(const std::array<QPointF, 12> &points, const QPointF &localPoint)
{
    Params2D p;

    p.p0 = points[KisBezierPatch::TL];
    p.p1 = points[KisBezierPatch::TL_HC];
    p.p2 = points[KisBezierPatch::TR_HC];
    p.p3 = points[KisBezierPatch::TR];

    p.q0 = points[KisBezierPatch::BL];
    p.q1 = points[KisBezierPatch::BL_HC];
    p.q2 = points[KisBezierPatch::BR_HC];
    p.q3 = points[KisBezierPatch::BR];

    p.r0 = points[KisBezierPatch::TL];
    p.r1 = points[KisBezierPatch::TL_VC];
    p.r2 = points[KisBezierPatch::BL_VC];
    p.r3 = points[KisBezierPatch::BL];

    p.s0 = points[KisBezierPatch::TR];
    p.s1 = points[KisBezierPatch::TR_VC];
    p.s2 = points[KisBezierPatch::BR_VC];
    p.s3 = points[KisBezierPatch::BR];

    PatchMethod f(localPoint.x(), localPoint.y(), p);
    return f.value();
}

QPointF calculateLocalPos(const std::array<QPointF, 12> &points, const QPointF &globalPoint)
{
   return calculateLocalPosImpl<LevelBasedPatchMethod>(points, globalPoint);
}

QPointF calculateGlobalPos(const std::array<QPointF, 12> &points, const QPointF &localPoint)
{
    return calculateGlobalPosImpl<LevelBasedPatchMethod>(points, localPoint);
}

QPointF calculateLocalPosSVG2(const std::array<QPointF, 12> &points, const QPointF &globalPoint)
{
   return calculateLocalPosImpl<SvgPatchMethod>(points, globalPoint);
}

QPointF calculateGlobalPosSVG2(const std::array<QPointF, 12> &points, const QPointF &localPoint)
{
    return calculateGlobalPosImpl<SvgPatchMethod>(points, localPoint);
}

QPointF interpolateQuadric(const QPointF &p0, const QPointF &p2, const QPointF &pt, qreal t)
{
    if (t <= 0.0 || t >= 1.0)
        return lerp(p0, p2, 0.5);

    /*
        B(t) = [x2 y2] = (1-t)^2*P0 + 2t*(1-t)*P1 + t^2*P2

               B(t) - (1-t)^2*P0 - t^2*P2
         P1 =  --------------------------
                       2t*(1-t)
    */

    QPointF c1 = pt - (1.0-t) * (1.0-t)*p0 - t * t * p2;

    qreal denom = 2.0 * t * (1.0-t);

    c1.rx() /= denom;
    c1.ry() /= denom;

    return c1;
}

std::pair<QPointF, QPointF> offsetSegment(qreal t, const QPointF &offset)
{
    /*
    * method from inkscape, original method and idea borrowed from Simon Budig
    * <simon@gimp.org> and the GIMP
    * cf. app/vectors/gimpbezierstroke.c, gimp_bezier_stroke_point_move_relative()
    *
    * feel good is an arbitrary parameter that distributes the delta between handles
    * if t of the drag point is less than 1/6 distance form the endpoint only
    * the corresponding handle is adjusted. This matches the behavior in GIMP
    */
    qreal feel_good;
    if (t <= 1.0 / 6.0)
        feel_good = 0;
    else if (t <= 0.5)
        feel_good = (pow((6 * t - 1) / 2.0, 3)) / 2;
    else if (t <= 5.0 / 6.0)
        feel_good = (1 - pow((6 * (1-t) - 1) / 2.0, 3)) / 2 + 0.5;
    else
        feel_good = 1;

    const QPointF moveP1 = ((1-feel_good)/(3*t*(1-t)*(1-t))) * offset;
    const QPointF moveP2 = (feel_good/(3*t*t*(1-t))) * offset;

    return std::make_pair(moveP1, moveP2);
}

qreal curveLength(const QPointF p0, const QPointF p1, const QPointF p2, const QPointF p3, const qreal error)
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

    const int deg = bezierDegree(p0, p1, p2, p3);

    if (deg == -1)
        return 0.0;

    // calculate chord length
    const qreal chordLen = kisDistance(p0, p3);

    if (deg == 1) {
        return chordLen;
    }

    // calculate length of control polygon
    qreal polyLength = 0.0;

    polyLength += kisDistance(p0, p1);
    polyLength += kisDistance(p1, p2);
    polyLength += kisDistance(p2, p3);

    if ((polyLength - chordLen) > error) {
        QPointF q0, q1, q2, q3, q4;
        deCasteljau(p0, p1, p2, p3, 0.5, &q0, &q1, &q2, &q3, &q4);

        return curveLength(p0, q0, q1, q2, error) +
                curveLength(q2, q3, q4, p3, error);
    } else {
        // the error is smaller than our tolerance
        if (deg == 3)
            return 0.5 * chordLen + 0.5 * polyLength;
        else
            return (2.0 * chordLen + polyLength) / 3.0;
    }
}

qreal curveLengthAtPoint(const QPointF p0, const QPointF p1, const QPointF p2, const QPointF p3, qreal t, const qreal error)
{
    QPointF q0, q1, q2, q3, q4;
    deCasteljau(p0, p1, p2, p3, t, &q0, &q1, &q2, &q3, &q4);

    return curveLength(p0, q0, q1, q2, error);
}

qreal curveParamBySegmentLength(const QPointF p0, const QPointF p1, const QPointF p2, const QPointF p3, qreal expectedLength, qreal totalLength, const qreal error)
{
    const qreal splitAtParam = expectedLength / totalLength;

    QPointF q0, q1, q2, q3, q4;
    deCasteljau(p0, p1, p2, p3, splitAtParam, &q0, &q1, &q2, &q3, &q4);

    const qreal portionLength = curveLength(p0, q0, q1, q2, error);

    if (std::abs(portionLength - expectedLength) < error) {
        return splitAtParam;
    } else if (portionLength < expectedLength) {
        return splitAtParam + (1.0 - splitAtParam) * curveParamBySegmentLength(q2, q3, q4, p3, expectedLength - portionLength, totalLength - portionLength, error);
    } else {
        return splitAtParam * curveParamBySegmentLength(p0, q0, q1, q2, expectedLength, portionLength, error);
    }
}


qreal curveParamByProportion(const QPointF p0, const QPointF p1, const QPointF p2, const QPointF p3, qreal proportion, const qreal error)
{
    const qreal totalLength = curveLength(p0, p1, p2, p3, error);
    const qreal expectedLength = proportion * totalLength;

    return curveParamBySegmentLength(p0, p1, p2, p3, expectedLength, totalLength, error);
}

qreal curveProportionByParam(const QPointF p0, const QPointF p1, const QPointF p2, const QPointF p3, qreal t, const qreal error)
{
    return curveLengthAtPoint(p0, p1, p2, p3, t, error) / curveLength(p0, p1, p2, p3, error);
}

std::pair<QPointF, QPointF> removeBezierNode(const QPointF &p0,
                                             const QPointF &p1,
                                             const QPointF &p2,
                                             const QPointF &p3,
                                             const QPointF &q1,
                                             const QPointF &q2,
                                             const QPointF &q3)
{
    /**
     * Calculates the curve control point after removal of a node
     * by minimizing squared error for the folloing problem:
     *
     * Given two consequent 3rd order curves P and Q with lengths Lp and Lq,
     * find 3rd order curve B, so that when splitting it at t = Lp / (Lp + Lq)
     * (using de Casteljau algorithm) the control points of the resulting
     * curves will have least square errors, compared to the corresponding
     * control points of curves P and Q.
     *
     * First we represent curves in matrix form:
     *
     * B(t) = T * M * B,
     * P(t) = T * Z1 * M * P,
     * Q(t) = T * Z2 * M * Q,
     *
     * where
     *    T = [1, t, t^2, t^3]
     *    M --- 4x4 matrix of Bezier coefficients
     *    B, P, Q --- vector of control points for the curves
     *
     *    Z1 --- conversion matrix that splits the curve into range [0.0...t]
     *    Z2 --- conversion matrix that splits the curve into range [t...1.0]
     *
     * Then we represent vectors P and Q via B:
     *
     * P = M^(-1) * Z1 * M * B
     * Q = M^(-1) * Z2 * M * B
     *
     * which in block matrix form looks like:
     *
     * [ P ]   [ M^(-1) * Z1 * M ]
     * |   | = |                 | * B
     * [ Q ]   [ M^(-1) * Z2 * M ]
     *
     *
     *         [ M^(-1) * Z1 * M ]
     * let C = |                 |,
     *         [ M^(-1) * Z2 * M ]
     *
     *     [ P ]
     * R = |   |
     *     [ Q ]
     *
     *
     * then
     *
     * R = C * B
     *
     * applying normal equation to find a solution with least square error,
     * get the final result:
     *
     * B = (C'C)^(-1) * C' * R
     */

    const qreal lenP = KisBezierUtils::curveLength(p0, p1, p2, p3, 0.001);
    const qreal lenQ = KisBezierUtils::curveLength(p3, q1, q2, q3, 0.001);

    const qreal z = lenP / (lenP + lenQ);

    Eigen::Matrix4f M;
    M << 1, 0, 0, 0,
         -3, 3, 0, 0,
          3, -6, 3, 0,
         -1, 3, -3, 1;

    Eigen::DiagonalMatrix<float, 4> Z_1;
    Z_1.diagonal() << 1, z, pow2(z), pow3(z);

    Eigen::Matrix4f Z_2;
    Z_2 << 1,     z,         pow2(z),               pow3(z),
           0, 1 - z, 2 * z * (1 - z), 3 * pow2(z) * (1 - z),
           0,     0,     pow2(1 - z),   3 * z * pow2(1 - z),
           0,     0,               0,           pow3(1 - z);

    Eigen::Matrix<float, 8, 2> R;
    R << p0.x(), p0.y(),
         p1.x(), p1.y(),
         p2.x(), p2.y(),
         p3.x(), p3.y(),
         p3.x(), p3.y(),
         q1.x(), q1.y(),
         q2.x(), q2.y(),
         q3.x(), q3.y();

    Eigen::Matrix<float, 2, 2> B_const;
    B_const << p0.x(), p0.y(),
               q3.x(), q3.y();


    Eigen::Matrix4f M1 = M.inverse() * Z_1 * M;
    Eigen::Matrix4f M2 = M.inverse() * Z_2 * M;

    Eigen::Matrix<float, 8, 4> C;
    C << M1, M2;

    Eigen::Matrix<float, 8, 2> C_const;
    C_const << C.col(0), C.col(3);

    Eigen::Matrix<float, 8, 2> C_var;
    C_var << C.col(1), C.col(2);

    Eigen::Matrix<float, 8, 2> R_var;
    R_var = R - C_const * B_const;

    Eigen::Matrix<float, 6, 2> R_reduced;
    R_reduced = R_var.block(1, 0, 6, 2);

    Eigen::Matrix<float, 6, 2> C_reduced;
    C_reduced = C_var.block(1, 0, 6, 2);

    Eigen::Matrix<float, 2, 2> result;
    result = (C_reduced.transpose() * C_reduced).inverse() * C_reduced.transpose() * R_reduced;

    QPointF resultP0(result(0, 0), result(0, 1));
    QPointF resultP1(result(1, 0), result(1, 1));

    return std::make_pair(resultP0, resultP1);
}
QVector<qreal> intersectWithLineImpl(const QPointF &p0, const QPointF &p1, const QPointF &p2, const QPointF &p3, const QLineF &line, qreal eps, qreal alpha, qreal beta)
{
    using KisAlgebra2D::intersectLines;

    QVector<qreal> result;

    const qreal length =
            kisDistance(p0, p1) +
            kisDistance(p1, p2) +
            kisDistance(p2, p3);

    if (length < eps) {
        if (intersectLines(p0, p3, line.p1(), line.p2())) {
            result << alpha * 0.5 + beta;
        }
    } else {

        const bool hasIntersections =
                intersectLines(p0, p1, line.p1(), line.p2()) ||
                intersectLines(p1, p2, line.p1(), line.p2()) ||
                intersectLines(p2, p3, line.p1(), line.p2());

        if (hasIntersections) {
            QPointF q0, q1, q2, q3, q4;

            deCasteljau(p0, p1, p2, p3, 0.5, &q0, &q1, &q2, &q3, &q4);

            result << intersectWithLineImpl(p0, q0, q1, q2, line, eps, 0.5 * alpha, beta);
            result << intersectWithLineImpl(q2, q3, q4, p3, line, eps, 0.5 * alpha, beta + 0.5 * alpha);
        }
    }
    return result;
}

QVector<qreal> intersectWithLine(const QPointF &p0, const QPointF &p1, const QPointF &p2, const QPointF &p3, const QLineF &line, qreal eps)
{
    return intersectWithLineImpl(p0, p1, p2, p3, line, eps, 1.0, 0.0);
}

boost::optional<qreal> intersectWithLineNearest(const QPointF &p0, const QPointF &p1, const QPointF &p2, const QPointF &p3, const QLineF &line, const QPointF &nearestAnchor, qreal eps)
{
    QVector<qreal> result = intersectWithLine(p0, p1, p2, p3, line, eps);

    qreal minDistance = std::numeric_limits<qreal>::max();
    boost::optional<qreal> nearestRoot;

    Q_FOREACH (qreal root, result) {
        const QPointF pt = bezierCurve(p0, p1, p2, p3, root);
        const qreal distance = kisDistance(pt, nearestAnchor);

        if (distance < minDistance) {
            minDistance = distance;
            nearestRoot = root;
        }
    }

    return nearestRoot;
}

}
