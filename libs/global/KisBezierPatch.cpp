/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

QPointF KisBezierPatch::localToGlobal(const QPointF &pt) const
{
    return KisBezierUtils::calculateGlobalPos(points, pt);
}

QPointF KisBezierPatch::globalToLocal(const QPointF &pt) const
{
    return KisBezierUtils::calculateLocalPos(points, pt);
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

    struct Split {
        QPointF p0;
        QPointF relP1;
        QPointF relP2;
        QPointF p3;
        qreal coord1;
        qreal coord2;
        qreal proportion;
    };

    std::vector<Split> verticalSplits;
    std::vector<Split> horizontalSplits;

    for (int y = 0; y < gridSize.height(); y++) {
        const qreal yProportion = qreal(y) / (gridSize.height() - 1);

        const qreal yCoord1 = KisBezierUtils::curveLengthAtPoint(points[TL], points[TL_VC], points[BL_VC], points[BL], yProportion, 0.01) / leftLength;
        const qreal yCoord2 = KisBezierUtils::curveLengthAtPoint(points[TR], points[TR_VC], points[BR_VC], points[BR], yProportion, 0.01) / rightLength;

        const QPointF p0 = bezierCurve(points[TL], points[TL_VC], points[BL_VC], points[BL], yProportion);

        const QPointF relP1 = lerp(points[TL_HC] - points[TL], points[BL_HC] - points[BL], yProportion);
        const QPointF relP2 = lerp(points[TR_HC] - points[TR], points[BR_HC] - points[BR], yProportion);

        const QPointF p3 = bezierCurve(points[TR], points[TR_VC], points[BR_VC], points[BR], yProportion);

        verticalSplits.push_back({p0, relP1, relP2, p3, yCoord1, yCoord2, yProportion});
    }

    for (int x = 0; x < gridSize.width(); x++) {
        const qreal xProportion = qreal(x) / (gridSize.width() - 1);

        const qreal xCoord1 = KisBezierUtils::curveLengthAtPoint(points[TL], points[TL_HC], points[TR_HC], points[TR], xProportion, 0.01) / topLength;
        const qreal xCoord2 = KisBezierUtils::curveLengthAtPoint(points[BL], points[BL_HC], points[BR_HC], points[BR], xProportion, 0.01) / bottomLength;

        const QPointF q0 = bezierCurve(points[TL], points[TL_HC], points[TR_HC], points[TR], xProportion);

        const QPointF relQ1 = lerp(points[TL_VC] - points[TL], points[TR_VC] - points[TR], xProportion);
        const QPointF relQ2 = lerp(points[BL_VC] - points[BL], points[BR_VC] - points[BR], xProportion);

        const QPointF q3 = bezierCurve(points[BL], points[BL_HC], points[BR_HC], points[BR], xProportion);

        horizontalSplits.push_back({q0, relQ1, relQ2, q3, xCoord1, xCoord2, xProportion});
    }

    for (int y = 0; y < gridSize.height(); y++) {
        for (int x = 0; x < gridSize.width(); x++) {
            const Split &ySplit = verticalSplits[y];
            const Split &xSplit = horizontalSplits[x];

            const QPointF transf1 = bezierCurve(ySplit.p0,
                                                ySplit.p0 + ySplit.relP1,
                                                ySplit.p3 + ySplit.relP2,
                                                ySplit.p3,
                                                xSplit.proportion);


            const QPointF transf2 = bezierCurve(xSplit.p0,
                                                xSplit.p0 + xSplit.relP1,
                                                xSplit.p3 + xSplit.relP2,
                                                xSplit.p3,
                                                ySplit.proportion);


            const QPointF transf = 0.5 * (transf1 + transf2);

            const QPointF localPt(lerp(xSplit.coord1, xSplit.coord2, ySplit.proportion),
                                  lerp(ySplit.coord1, ySplit.coord2, xSplit.proportion));
            const QPointF orig = KisAlgebra2D::relativeToAbsolute(localPt, originalRect);

            origPoints.append(orig);
            transfPoints.append(transf);
        }
    }
}

void KisBezierPatch::sampleRegularGridSVG2(QSize &gridSize, QVector<QPointF> &origPoints, QVector<QPointF> &transfPoints, const QPointF &dstStep) const
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


    struct Split {
        QPointF p0;
        QPointF p3;
        qreal coord1;
        qreal coord2;
        qreal proportion;
    };

    std::vector<Split> verticalSplits;
    std::vector<Split> horizontalSplits;

    for (int y = 0; y < gridSize.height(); y++) {
        const qreal yProportion = qreal(y) / (gridSize.height() - 1);

        const qreal yCoord1 = KisBezierUtils::curveLengthAtPoint(points[TL], points[TL_VC], points[BL_VC], points[BL], yProportion, 0.01) / leftLength;
        const qreal yCoord2 = KisBezierUtils::curveLengthAtPoint(points[TR], points[TR_VC], points[BR_VC], points[BR], yProportion, 0.01) / rightLength;

        const QPointF p0 = bezierCurve(points[TL], points[TL_VC], points[BL_VC], points[BL], yProportion);
        const QPointF p3 = bezierCurve(points[TR], points[TR_VC], points[BR_VC], points[BR], yProportion);

        verticalSplits.push_back({p0, p3, yCoord1, yCoord2, yProportion});
    }

    for (int x = 0; x < gridSize.width(); x++) {
        const qreal xProportion = qreal(x) / (gridSize.width() - 1);

        const qreal xCoord1 = KisBezierUtils::curveLengthAtPoint(points[TL], points[TL_HC], points[TR_HC], points[TR], xProportion, 0.01) / topLength;
        const qreal xCoord2 = KisBezierUtils::curveLengthAtPoint(points[BL], points[BL_HC], points[BR_HC], points[BR], xProportion, 0.01) / bottomLength;

        const QPointF q0 = bezierCurve(points[TL], points[TL_HC], points[TR_HC], points[TR], xProportion);
        const QPointF q3 = bezierCurve(points[BL], points[BL_HC], points[BR_HC], points[BR], xProportion);

        horizontalSplits.push_back({q0, q3, xCoord1, xCoord2, xProportion});
    }

    for (int y = 0; y < gridSize.height(); y++) {
        for (int x = 0; x < gridSize.width(); x++) {
            const Split &ySplit = verticalSplits[y];
            const Split &xSplit = horizontalSplits[x];

            const QPointF Sc = lerp(xSplit.p0, xSplit.p3, ySplit.proportion);
            const QPointF Sd = lerp(ySplit.p0, ySplit.p3, xSplit.proportion);

            const QPointF Sb =
                    lerp(lerp(points[TL], points[TR], xSplit.proportion),
                         lerp(points[BL], points[BR], xSplit.proportion),
                         ySplit.proportion);

            const QPointF transf = Sc + Sd - Sb;

            const QPointF localPt(lerp(xSplit.coord1, xSplit.coord2, ySplit.proportion),
                                  lerp(ySplit.coord1, ySplit.coord2, xSplit.proportion));
            const QPointF orig = KisAlgebra2D::relativeToAbsolute(localPt, originalRect);

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
