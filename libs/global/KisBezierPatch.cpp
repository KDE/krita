/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisBezierPatch.h"

#include <QtMath>
#include <kis_algebra_2d.h>
#include "KisBezierUtils.h"

#include "kis_debug.h"

QRectF KisBezierPatch::dstBoundingRect() const {
    QRectF result;

    for (auto it = points.begin(); it != points.end(); ++it) {
        KisAlgebra2D::accumulateBounds(*it, &result);
    }

    return result;
}

QRectF KisBezierPatch::srcBoundingRect() const {
    return originalRect;
}

void KisBezierPatch::sampleIrregularGrid(QSize &gridSize, QVector<QPointF> &origPoints, QVector<QPointF> &transfPoints) const
{
    using KisAlgebra2D::lerp;
    using KisBezierUtils::bezierCurve;
    using KisBezierUtils::linearizeCurve;
    using KisBezierUtils::mergeLinearizationSteps;

    const qreal eps = 1.0; // linearize up to 1 pixel precision
    const QVector<qreal> topSteps = linearizeCurve(points[TL], points[TL_HC], points[TR_HC], points[TR], eps);
    const QVector<qreal> bottomSteps = linearizeCurve(points[BL], points[BL_HC], points[BR_HC], points[BR], eps);
    const QVector<qreal> horizontalSteps = mergeLinearizationSteps(topSteps, bottomSteps);

    const QVector<qreal> leftSteps = linearizeCurve(points[TL], points[TL_VC], points[BL_VC], points[BL], eps);
    const QVector<qreal> rightSteps = linearizeCurve(points[TR], points[TR_VC], points[BR_VC], points[BR], eps);
    const QVector<qreal> verticalSteps = mergeLinearizationSteps(leftSteps, rightSteps);

    gridSize.rwidth() = horizontalSteps.size();
    gridSize.rheight() = verticalSteps.size();

    for (int y = 0; y < gridSize.height(); y++) {
        const qreal yProportion = verticalSteps[y];

        for (int x = 0; x < gridSize.width(); x++) {
            const qreal xProportion = horizontalSteps[x];

            const QPointF orig = KisAlgebra2D::relativeToAbsolute(
                        QPointF(xProportion, yProportion), originalRect);

            const QPointF Sc =
                    lerp(bezierCurve(points[TL], points[TL_HC], points[TR_HC], points[TR], xProportion),
                         bezierCurve(points[BL], points[BL_HC], points[BR_HC], points[BR], xProportion),
                         yProportion);

            const QPointF Sd =
                    lerp(bezierCurve(points[TL], points[TL_VC], points[BL_VC], points[BL], yProportion),
                         bezierCurve(points[TR], points[TR_VC], points[BR_VC], points[BR], yProportion),
                         xProportion);

            const QPointF Sb =
                    lerp(lerp(points[TL], points[TR], xProportion),
                         lerp(points[BL], points[BR], xProportion),
                         yProportion);

            const QPointF transf = Sc + Sd - Sb;

            origPoints.append(orig);
            transfPoints.append(transf);
        }
    }

}

void KisBezierPatch::sampleRegularGrid(QSize &gridSize, QVector<QPointF> &origPoints, QVector<QPointF> &transfPoints, const QPointF &dstStep) const
{
    using KisAlgebra2D::lerp;
    using KisBezierUtils::bezierCurve;

    const QRectF bounds = dstBoundingRect();
    gridSize.rwidth() = qCeil(bounds.width() / dstStep.x());
    gridSize.rheight() = qCeil(bounds.height() / dstStep.y());

    for (int y = 0; y < gridSize.height(); y++) {
        const qreal yProportion = qreal(y) / (gridSize.height() - 1);

        for (int x = 0; x < gridSize.width(); x++) {
            const qreal xProportion = qreal(x) / (gridSize.width() - 1);

            const QPointF orig = KisAlgebra2D::relativeToAbsolute(
                        QPointF(xProportion, yProportion), originalRect);

            const QPointF Sc =
                    lerp(bezierCurve(points[TL], points[TL_HC], points[TR_HC], points[TR], xProportion),
                         bezierCurve(points[BL], points[BL_HC], points[BR_HC], points[BR], xProportion),
                         yProportion);

            const QPointF Sd =
                    lerp(bezierCurve(points[TL], points[TL_VC], points[BL_VC], points[BL], yProportion),
                         bezierCurve(points[TR], points[TR_VC], points[BR_VC], points[BR], yProportion),
                         xProportion);

            const QPointF Sb =
                    lerp(lerp(points[TL], points[TR], xProportion),
                         lerp(points[BL], points[BR], xProportion),
                         yProportion);

            const QPointF transf = Sc + Sd - Sb;

            origPoints.append(orig);
            transfPoints.append(transf);
        }
    }
}

QDebug operator<<(QDebug dbg, const KisBezierPatch &p) {
    dbg.nospace() << "Patch " << p.srcBoundingRect() << " -> " << p.dstBoundingRect() << "\n";
    dbg.nospace() << "  ( " << p.points[KisBezierPatch::TL] << " "<< p.points[KisBezierPatch::TR] << " " << p.points[KisBezierPatch::BL] << " " << p.points[KisBezierPatch::BR] << ") ";
    return dbg.nospace();
}
