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

    const qreal topLength = KisBezierUtils::curveLength(points[TL], points[TL_HC], points[TR_HC], points[TR], 0.01);
    const qreal bottomLength = KisBezierUtils::curveLength(points[BL], points[BL_HC], points[BR_HC], points[BR], 0.01);

    const qreal leftLength = KisBezierUtils::curveLength(points[TL], points[TL_VC], points[BL_VC], points[BL], 0.01);
    const qreal rightLength = KisBezierUtils::curveLength(points[TR], points[TR_VC], points[BR_VC], points[BR], 0.01);

    for (int y = 0; y < gridSize.height(); y++) {
        const qreal yProportion = verticalSteps[y];

        const qreal yCoord1 = KisBezierUtils::curveLengthAtPoint(points[TL], points[TL_VC], points[BL_VC], points[BL], yProportion, 0.01) / leftLength;
        const qreal yCoord2 = KisBezierUtils::curveLengthAtPoint(points[TR], points[TR_VC], points[BR_VC], points[BR], yProportion, 0.01) / rightLength;

        for (int x = 0; x < gridSize.width(); x++) {
            const qreal xProportion = horizontalSteps[x];

            const qreal xCoord1 = KisBezierUtils::curveLengthAtPoint(points[TL], points[TL_HC], points[TR_HC], points[TR], xProportion, 0.01) / topLength;
            const qreal xCoord2 = KisBezierUtils::curveLengthAtPoint(points[BL], points[BL_HC], points[BR_HC], points[BR], xProportion, 0.01) / bottomLength;

            const QPointF localPt(lerp(xCoord1, xCoord2, yProportion), lerp(yCoord1, yCoord2, xProportion));
            const QPointF orig = KisAlgebra2D::relativeToAbsolute(localPt, originalRect);

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

    const qreal topLength = KisBezierUtils::curveLength(points[TL], points[TL_HC], points[TR_HC], points[TR], 0.01);
    const qreal bottomLength = KisBezierUtils::curveLength(points[BL], points[BL_HC], points[BR_HC], points[BR], 0.01);

    const qreal leftLength = KisBezierUtils::curveLength(points[TL], points[TL_VC], points[BL_VC], points[BL], 0.01);
    const qreal rightLength = KisBezierUtils::curveLength(points[TR], points[TR_VC], points[BR_VC], points[BR], 0.01);

    for (int y = 0; y < gridSize.height(); y++) {
        const qreal yProportion = qreal(y) / (gridSize.height() - 1);

        const qreal yCoord1 = KisBezierUtils::curveLengthAtPoint(points[TL], points[TL_VC], points[BL_VC], points[BL], yProportion, 0.01) / leftLength;
        const qreal yCoord2 = KisBezierUtils::curveLengthAtPoint(points[TR], points[TR_VC], points[BR_VC], points[BR], yProportion, 0.01) / rightLength;

        for (int x = 0; x < gridSize.width(); x++) {
            const qreal xProportion = qreal(x) / (gridSize.width() - 1);

            const qreal xCoord1 = KisBezierUtils::curveLengthAtPoint(points[TL], points[TL_HC], points[TR_HC], points[TR], xProportion, 0.01) / topLength;
            const qreal xCoord2 = KisBezierUtils::curveLengthAtPoint(points[BL], points[BL_HC], points[BR_HC], points[BR], xProportion, 0.01) / bottomLength;

            const QPointF localPt(lerp(xCoord1, xCoord2, yProportion), lerp(yCoord1, yCoord2, xProportion));
            const QPointF orig = KisAlgebra2D::relativeToAbsolute(localPt, originalRect);

#if 1

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
#else
            const QPointF p0 = bezierCurve(points[TL], points[TL_VC], points[BL_VC], points[BL], yProportion);

            const QPointF relP1 = lerp(points[TL_HC] - points[TL], points[BL_HC] - points[BL], yProportion);
            const QPointF relP2 = lerp(points[TR_HC] - points[TR], points[BR_HC] - points[BR], yProportion);

            const QPointF p3 = bezierCurve(points[TR], points[TR_VC], points[BR_VC], points[BR], yProportion);

            const QPointF transf1 = bezierCurve(p0, p0 + relP1, p3 + relP2, p3, xProportion);


            const QPointF q0 = bezierCurve(points[TL], points[TL_HC], points[TR_HC], points[TR], xProportion);

            const QPointF relQ1 = lerp(points[TL_VC] - points[TL], points[TR_VC] - points[TR], xProportion);
            const QPointF relQ2 = lerp(points[BL_VC] - points[BL], points[BR_VC] - points[BR], xProportion);

            const QPointF q3 = bezierCurve(points[BL], points[BL_HC], points[BR_HC], points[BR], xProportion);

            const QPointF transf2 = bezierCurve(q0, q0 + relQ1, q3 + relQ2, q3, yProportion);


            const QPointF transf = 0.5 * (transf1 + transf2);
#endif
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
