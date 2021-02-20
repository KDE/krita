/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_green_coordinates_math.h"

#include <cmath>
#include <kis_global.h>
#include <kis_algebra_2d.h>
using namespace KisAlgebra2D;


/**
 * Implementation of the Green Coordinates transformation method
 * by Yaron Lipman, David Levin, Daniel Cohen-Or
 *
 * http://www.math.tau.ac.il/~lipmanya/GC/gc.htm
 */

struct PrecalculatedCoords
{
    QVector<qreal> psi; // for each edge
    QVector<qreal> phi; // for each vertex
};


struct Q_DECL_HIDDEN KisGreenCoordinatesMath::Private
{
    Private () : transformedCageDirection(0) {}

    QVector<qreal> originalCageEdgeSizes;
    QVector<QPointF> transformedCageNormals;
    int transformedCageDirection;

    QVector<PrecalculatedCoords> precalculatedCoords;

    void precalculateOnePoint(const QVector<QPointF> &originalCage,
                              PrecalculatedCoords *coords,
                              const QPointF &pt,
                              int polygonDirection);

    inline void precalculateOneEdge(const QPointF &pt,
                                    const QPointF &v1,
                                    const QPointF &v2,
                                    qreal *edge_psi,
                                    qreal *vertex1_phi,
                                    qreal *vertex2_phi,
                                    int polygonDirection);
};

inline void KisGreenCoordinatesMath::
Private::precalculateOneEdge(const QPointF &pt,
                             const QPointF &v1,
                             const QPointF &v2,
                             qreal *edge_psi,
                             qreal *vertex1_phi,
                             qreal *vertex2_phi,
                             int polygonDirection)
{
    QPointF a = v2 - v1;
    QPointF b = v1 - pt;
    qreal Q = dotProduct(a, a);
    qreal S = dotProduct(b, b);
    qreal R = dotProduct(2 * a, b);

    qreal BA = dotProduct(b, norm(a) * inwardUnitNormal(a, polygonDirection));
    qreal SRT = std::sqrt(4 * S * Q - pow2(R));
    qreal L0 = std::log(S);
    qreal L1 = std::log(S + Q + R);
    qreal A0 = std::atan(R / SRT) / SRT;
    qreal A1 = std::atan((2 * Q + R) / SRT) / SRT;
    qreal A10 = A1 - A0;
    qreal L10 = L1 - L0;

    /**
     * The normals in the official paper are calculated somehow
     * differently so we must flip the sign of the \psi
     * variable. Don't ask me why... (DK)
     */
    static const qreal magicMultiplier = -1.0;

    *edge_psi = -magicMultiplier * norm(a) / (4 * M_PI) *
        ((4 * S - pow2(R) / Q) * A10 + R / (2 * Q) * L10 + L1 - 2);

    *vertex2_phi += -BA / (2 * M_PI) * (L10 / (2 * Q) - A10 * R / Q);
    *vertex1_phi +=  BA / (2 * M_PI) * (L10 / (2 * Q) - A10 * (2 + R / Q));
}

void KisGreenCoordinatesMath::Private::precalculateOnePoint(const QVector<QPointF> &originalCage,
                                                            PrecalculatedCoords *coords,
                                                            const QPointF &pt,
                                                            int polygonDirection)
{
    const int numCagePoints = originalCage.size();

    // edge index is defined by the index of the start point
    // that is: v0-v1 -> e0, v1-v2 -> e1.

    for (int i = 1; i <= numCagePoints; i++) {
        int endIndex = i != numCagePoints ? i : 0;
        int startIndex = i - 1;

        precalculateOneEdge(pt,
                            originalCage[startIndex],
                            originalCage[endIndex],
                            &coords->psi[startIndex],
                            &coords->phi[startIndex],
                            &coords->phi[endIndex],
                            polygonDirection);
    }
}

KisGreenCoordinatesMath::KisGreenCoordinatesMath()
    : m_d(new Private())
{
}

KisGreenCoordinatesMath::~KisGreenCoordinatesMath()
{
}

void KisGreenCoordinatesMath::precalculateGreenCoordinates(const QVector<QPointF> &originalCage, const QVector<QPointF> &points)
{
    const int cageDirection = polygonDirection(originalCage);
    const int numPoints = points.size();
    const int numCagePoints = originalCage.size();

    m_d->originalCageEdgeSizes.resize(numCagePoints);

    for (int i = 1; i <= numCagePoints; i++) {
        int endIndex = i != numCagePoints ? i : 0;
        int startIndex = i - 1;

        m_d->originalCageEdgeSizes[startIndex] =
            norm(originalCage[endIndex] - originalCage[startIndex]);
    }

    m_d->precalculatedCoords.resize(numPoints);

    for (int i = 0; i < numPoints; i++) {
        m_d->precalculatedCoords[i].psi.resize(numCagePoints);
        m_d->precalculatedCoords[i].phi.resize(numCagePoints);

        m_d->precalculateOnePoint(originalCage,
                                  &m_d->precalculatedCoords[i],
                                  points[i],
                                  cageDirection);
    }
}

void KisGreenCoordinatesMath::generateTransformedCageNormals(const QVector<QPointF> &transformedCage)
{
    m_d->transformedCageDirection = polygonDirection(transformedCage);

    const int numCagePoints = transformedCage.size();
    m_d->transformedCageNormals.resize(numCagePoints);

    for (int i = 1; i <= numCagePoints; i++) {
        int endIndex = i != numCagePoints ? i : 0;
        int startIndex = i - 1;

        QPointF transformedEdge =
            transformedCage[endIndex] - transformedCage[startIndex];

        qreal scaleCoeff =
            norm(transformedEdge) / m_d->originalCageEdgeSizes[startIndex];

        m_d->transformedCageNormals[startIndex] =
            scaleCoeff * inwardUnitNormal(transformedEdge, m_d->transformedCageDirection);
    }
}

QPointF KisGreenCoordinatesMath::transformedPoint(int pointIndex, const QVector<QPointF> &transformedCage)
{
    QPointF result;

    const int numCagePoints = transformedCage.size();


    PrecalculatedCoords *coords = &m_d->precalculatedCoords[pointIndex];

    for (int i = 0; i < numCagePoints; i++) {
        result += coords->phi[i] * transformedCage[i];
        result += coords->psi[i] * m_d->transformedCageNormals[i];
    }

    return result;
}

