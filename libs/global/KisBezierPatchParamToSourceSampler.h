/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISBEZIERPATCHPARAMTOSOURCESAMPLER_H
#define KISBEZIERPATCHPARAMTOSOURCESAMPLER_H

#include "KisBezierPatch.h"
#include "KisBezierUtils.h"
#include "KisBezierPatchParamSpaceUtils.h"

/**
 * A simple class that maps param-space point of a bezier patch into the source-range
 */
struct KisBezierPatchParamToSourceSampler
{
    using Range = KisBezierUtils::Range;

    KisBezierPatchParamToSourceSampler(const KisBezierPatch &_patch)
        : patch(_patch)
        , topLength(KisBezierUtils::curveLength(patch.points[KisBezierPatch::TL], patch.points[KisBezierPatch::TL_HC], patch.points[KisBezierPatch::TR_HC], patch.points[KisBezierPatch::TR], 0.01))
        , bottomLength(KisBezierUtils::curveLength(patch.points[KisBezierPatch::BL], patch.points[KisBezierPatch::BL_HC], patch.points[KisBezierPatch::BR_HC], patch.points[KisBezierPatch::BR], 0.01))
        , leftLength(KisBezierUtils::curveLength(patch.points[KisBezierPatch::TL], patch.points[KisBezierPatch::TL_VC], patch.points[KisBezierPatch::BL_VC], patch.points[KisBezierPatch::BL], 0.01))
        , rightLength(KisBezierUtils::curveLength(patch.points[KisBezierPatch::TR], patch.points[KisBezierPatch::TR_VC], patch.points[KisBezierPatch::BR_VC], patch.points[KisBezierPatch::BR], 0.01))
    {}

    KisBezierPatch patch;

    qreal topLength;
    qreal bottomLength;

    qreal leftLength;
    qreal rightLength ;

    Range xRange(qreal xParam) const
    {
        qreal xCoord1 = KisBezierUtils::curveLengthAtPoint(patch.points[KisBezierPatch::TL], patch.points[KisBezierPatch::TL_HC], patch.points[KisBezierPatch::TR_HC], patch.points[KisBezierPatch::TR], xParam, 0.01) / topLength;
        qreal xCoord2 = KisBezierUtils::curveLengthAtPoint(patch.points[KisBezierPatch::BL], patch.points[KisBezierPatch::BL_HC], patch.points[KisBezierPatch::BR_HC], patch.points[KisBezierPatch::BR], xParam, 0.01) / bottomLength;

        xCoord1 = patch.originalRect.left() + xCoord1 * patch.originalRect.width();
        xCoord2 = patch.originalRect.left() + xCoord2 * patch.originalRect.width();

        if (xCoord1 > xCoord2) {
            std::swap(xCoord1, xCoord2);
        }

        return {xCoord1, xCoord2};
    };

    Range yRange(qreal yParam) const
    {
        qreal yCoord1 = KisBezierUtils::curveLengthAtPoint(patch.points[KisBezierPatch::TL], patch.points[KisBezierPatch::TL_VC], patch.points[KisBezierPatch::BL_VC], patch.points[KisBezierPatch::BL], yParam, 0.01) / leftLength;
        qreal yCoord2 = KisBezierUtils::curveLengthAtPoint(patch.points[KisBezierPatch::TR], patch.points[KisBezierPatch::TR_VC], patch.points[KisBezierPatch::BR_VC], patch.points[KisBezierPatch::BR], yParam, 0.01) / rightLength;

        yCoord1 = patch.originalRect.top() + yCoord1 * patch.originalRect.height();
        yCoord2 = patch.originalRect.top() + yCoord2 * patch.originalRect.height();

        if (yCoord1 > yCoord2) {
            std::swap(yCoord1, yCoord2);
        }

        return {yCoord1, yCoord2};
    };

    QPointF point(qreal xParam, qreal yParam) const
    {
        using KisAlgebra2D::lerp;

        const Range xRange = this->xRange(xParam);
        const Range yRange = this->yRange(yParam);

        return QPointF(lerp(xRange.start, xRange.end, yParam),
                       lerp(yRange.start, yRange.end, xParam));
    }

    QPointF point(const QPointF &pt) const {
        return point(pt.x(), pt.y());
    }
};

#endif // KISBEZIERPATCHPARAMTOSOURCESAMPLER_H
