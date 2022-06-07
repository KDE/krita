/*
 * SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 */

#include "PerspectiveBasedAssistantHelper.h"

#include <klocalizedstring.h>
#include "kis_debug.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTransform>

#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include "kis_algebra_2d.h"
#include <Eigen/Eigenvalues>

#include <math.h>
#include<QDebug>
#include <QtMath>

#include <functional>



bool PerspectiveBasedAssistantHelper::getTetragon(const QList<KisPaintingAssistantHandleSP>& handles, bool isAssistantComplete, QPolygonF& outPolygon)
{
    outPolygon.clear();
    for (int i = 0; i < handles.size(); ++i) {
        outPolygon.push_back(*handles[i]);
    }

    if (!isAssistantComplete) {
        return false;
    }

    int sum = 0;
    int signs[4];

    for (int i = 0; i < 4; ++i) {
        int j = (i == 3) ? 0 : (i + 1);
        int k = (j == 3) ? 0 : (j + 1);
        signs[i] = KisAlgebra2D::signZZ(pdot(outPolygon[j] - outPolygon[i], outPolygon[k] - outPolygon[j]));
        sum += signs[i];
    }

    if (sum == 0) {
        // complex (crossed)
        for (int i = 0; i < 4; ++i) {
            int j = (i == 3) ? 0 : (i + 1);
            if (signs[i] * signs[j] == -1) {
                // opposite signs: uncross
                std::swap(outPolygon[i], outPolygon[j]);
                return true;
            }
        }
        // okay, maybe it's just a line
        return false;
    } else if (sum != 4 && sum != -4) {
        // concave, or a triangle
        if (sum == 2 || sum == -2) {
            // concave, let's return a triangle instead
            for (int i = 0; i < 4; ++i) {
                int j = (i == 3) ? 0 : (i + 1);
                if (signs[i] != KisAlgebra2D::signZZ(sum)) {
                    // wrong sign: drop the inside node
                    outPolygon.remove(j);
                    return false;
                }
            }
        }
        return false;
    }
    // convex
    return true;
}

QPolygonF PerspectiveBasedAssistantHelper::getAllConnectedTetragon(const QList<KisPaintingAssistantHandleSP>& handles)
{
    QPolygonF polyAllConnected;
    if (handles.size() < 4) {
        return polyAllConnected;
    }
    polyAllConnected << *handles[0] << *handles[1] << *handles[2] << *handles[3] << *handles[0] << *handles[2] << *handles[1] << *handles[3];
    return polyAllConnected;
}

qreal PerspectiveBasedAssistantHelper::localScale(const QTransform &transform, QPointF pt)
{
    //    const qreal epsilon = 1e-5, epsilonSquared = epsilon * epsilon;
    //    qreal xSizeSquared = lengthSquared(transform.map(pt + QPointF(epsilon, 0.0)) - orig) / epsilonSquared;
    //    qreal ySizeSquared = lengthSquared(transform.map(pt + QPointF(0.0, epsilon)) - orig) / epsilonSquared;
    //    xSizeSquared /= lengthSquared(transform.map(QPointF(0.0, pt.y())) - transform.map(QPointF(1.0, pt.y())));
    //    ySizeSquared /= lengthSquared(transform.map(QPointF(pt.x(), 0.0)) - transform.map(QPointF(pt.x(), 1.0)));
    //  when taking the limit epsilon->0:
    //  xSizeSquared=((m23*y+m33)^2*(m23*y+m33+m13)^2)/(m23*y+m13*x+m33)^4
    //  ySizeSquared=((m23*y+m33)^2*(m23*y+m33+m13)^2)/(m23*y+m13*x+m33)^4
    //  xSize*ySize=(abs(m13*x+m33)*abs(m13*x+m33+m23)*abs(m23*y+m33)*abs(m23*y+m33+m13))/(m23*y+m13*x+m33)^4
    const qreal x = transform.m13() * pt.x(),
            y = transform.m23() * pt.y(),
            a = x + transform.m33(),
            b = y + transform.m33(),
            c = x + y + transform.m33(),
            d = c * c;
    return fabs(a*(a + transform.m23())*b*(b + transform.m13()))/(d * d);
}

qreal PerspectiveBasedAssistantHelper::inverseMaxLocalScale(const QTransform &transform)
{
    const qreal a = fabs((transform.m33() + transform.m13()) * (transform.m33() + transform.m23())),
            b = fabs((transform.m33()) * (transform.m13() + transform.m33() + transform.m23())),
            d00 = transform.m33() * transform.m33(),
            d11 = (transform.m33() + transform.m23() + transform.m13())*(transform.m33() + transform.m23() + transform.m13()),
            s0011 = qMin(d00, d11) / a,
            d10 = (transform.m33() + transform.m13()) * (transform.m33() + transform.m13()),
            d01 = (transform.m33() + transform.m23()) * (transform.m33() + transform.m23()),
            s1001 = qMin(d10, d01) / b;
    return qMin(s0011, s1001);
}

qreal PerspectiveBasedAssistantHelper::distanceInGrid(const QList<KisPaintingAssistantHandleSP>& handles, bool isAssistantComplete, const QPointF &point)
{
    // TODO: make it not calculate the poly max distance over and over
    qreal defaultValue = 1;
    int vertexCount = 4;

    QPolygonF poly;
    if (!PerspectiveBasedAssistantHelper::getTetragon(handles, isAssistantComplete, poly)) {
        return defaultValue;
    }

    boost::optional<QPointF> vp1;
    boost::optional<QPointF> vp2;

    PerspectiveBasedAssistantHelper::getVanishingPointsOptional(poly, vp1, vp2);
    if (!vp1 && !vp2) {
        return defaultValue; // possibly wrong shape
    } else if (!vp1 || !vp2) {
        // result should be:
        // dist from horizon / max dist from horizon
        // horizon is parallel to the parallel sides of the tetragon (two must be parallel if there is only one vp)
        QLineF horizon;
        if (vp1) {
            // that means the 0-1 line is the horizon parallel line
            horizon = QLineF(vp1.get(), vp1.get() + poly[1] - poly[0]);
        } else {
            horizon = QLineF(vp2.get(), vp2.get() + poly[2] - poly[1]);
        }

        qreal dist = kisDistanceToLine(point, horizon);
        qreal distMax = 0;
        for (int i = 0; i < vertexCount; i++) {
            qreal vertexDist = kisDistanceToLine(poly[i], horizon);
            if (vertexDist > distMax) {
                distMax = vertexDist;
            }
        }
        if (distMax == 0) {
            return defaultValue;
        }
        return dist/distMax;
    } else if (vp1 && vp2) {
        // should be:
        // dist from vp-line / max dist from vp-line
        QLineF horizon = QLineF(vp1.get(), vp2.get());

        qreal dist = kisDistanceToLine(point, horizon);
        qreal distMax = 0;
        for (int i = 0; i < vertexCount; i++) {
            qreal vertexDist = kisDistanceToLine(poly[i], horizon);
            if (vertexDist > distMax) {
                distMax = vertexDist;
            }
        }
        if (distMax == 0) {
            return defaultValue;
        }
        return dist/distMax;
    }

    return defaultValue;

}

    if (inverse.m13() * point.x() + inverse.m23() * point.y() + inverse.m33() == 0.0) {
        return infinity; // point at infinity
    }

    return localScale(transform, inverse.map(point)) * inverseMaxLocalScale(transform);

bool PerspectiveBasedAssistantHelper::getVanishingPointsOptional(const QPolygonF &poly, boost::optional<QPointF> &vp1, boost::optional<QPointF> &vp2)
{
    bool either = false;
    vp1 = boost::none;
    vp2 = boost::none;

    QPointF intersection(0, 0);
    // note: in code it seems like vp1 and vp2 are swapped, but it's all correct if you read carefully

    if (fmod(QLineF(poly[0], poly[1]).angle(), 180.0)>=fmod(QLineF(poly[2], poly[3]).angle(), 180.0)+2.0
            || fmod(QLineF(poly[0], poly[1]).angle(), 180.0)<=fmod(QLineF(poly[2], poly[3]).angle(), 180.0)-2.0) {
        if (QLineF(poly[0], poly[1]).intersect(QLineF(poly[2], poly[3]), &intersection) != QLineF::NoIntersection) {
            vp2 = intersection;
            either = true;
        }
    }
    if (fmod(QLineF(poly[1], poly[2]).angle(), 180.0)>=fmod(QLineF(poly[3], poly[0]).angle(), 180.0)+2.0
            || fmod(QLineF(poly[1], poly[2]).angle(), 180.0)<=fmod(QLineF(poly[3], poly[0]).angle(), 180.0)-2.0){
        if (QLineF(poly[1], poly[2]).intersect(QLineF(poly[3], poly[0]), &intersection) != QLineF::NoIntersection) {
            vp1 = intersection;
            either = true;
        }
    }
    return either;
}

qreal PerspectiveBasedAssistantHelper::pdot(const QPointF &a, const QPointF &b)
{
    return a.x() * b.y() - a.y() * b.x();
}


