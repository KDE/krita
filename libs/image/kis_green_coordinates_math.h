/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_GREEN_COORDINATES_MATH_H
#define __KIS_GREEN_COORDINATES_MATH_H

#include <QScopedPointer>
#include <QVector>
#include <QPointF>

#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisGreenCoordinatesMath
{
public:
    KisGreenCoordinatesMath();
    ~KisGreenCoordinatesMath();

    /**
     * Prepare the transformation framework by computing internal
     * coordinates of the points in cage.
     *
     * Please note that the points in \p points will later be accessed
     * with indexes only.
     */
    void precalculateGreenCoordinates(const QVector<QPointF> &originalCage, const QVector<QPointF> &points);

    /**
     * Precalculate coefficients of the destination cage. Should be
     * called once for every cage change
     */
    void generateTransformedCageNormals(const QVector<QPointF> &transformedCage);

    /**
     * Transform one point according to its index
     */
    QPointF transformedPoint(int pointIndex, const QVector<QPointF> &transformedCage);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_GREEN_COORDINATES_MATH_H */
