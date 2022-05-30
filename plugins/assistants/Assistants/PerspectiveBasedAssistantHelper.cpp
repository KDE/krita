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

qreal PerspectiveBasedAssistantHelper::pdot(const QPointF &a, const QPointF &b)
{
    return a.x() * b.y() - a.y() * b.x();
}


