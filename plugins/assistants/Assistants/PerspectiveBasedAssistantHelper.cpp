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
    const qreal defaultValue = 1.0;
    const qreal infinity = 0.0;

    QPolygonF poly;
    if (!PerspectiveBasedAssistantHelper::getTetragon(handles, isAssistantComplete, poly)) {
        return defaultValue;
    }

    QTransform transform;
    if (!QTransform::squareToQuad(poly, transform)) {
        return defaultValue;
    }

    bool invertible;
    QTransform inverse = transform.inverted(&invertible);

    if (!invertible) {
        return defaultValue;
    }

    if (inverse.m13() * point.x() + inverse.m23() * point.y() + inverse.m33() == 0.0) {
        return infinity; // point at infinity
    }

    return localScale(transform, inverse.map(point)) * inverseMaxLocalScale(transform);

}

qreal PerspectiveBasedAssistantHelper::pdot(const QPointF &a, const QPointF &b)
{
    return a.x() * b.y() - a.y() * b.x();
}


