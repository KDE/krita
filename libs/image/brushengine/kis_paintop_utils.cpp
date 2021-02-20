/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_paintop_utils.h"

#include "krita_utils.h"
#include "krita_container_utils.h"
#include <KisRenderedDab.h>

#include <functional>

namespace KisPaintOpUtils {


KisSpacingInformation effectiveSpacing(qreal dabWidth, qreal dabHeight, qreal extraScale, bool distanceSpacingEnabled, bool isotropicSpacing, qreal rotation, bool axesFlipped, qreal spacingVal, bool autoSpacingActive, qreal autoSpacingCoeff, qreal lodScale)
{
    QPointF spacing;

    if (!isotropicSpacing) {
        if (autoSpacingActive) {
            spacing = calcAutoSpacing(QPointF(dabWidth, dabHeight), autoSpacingCoeff, lodScale);
        } else {
            spacing = QPointF(dabWidth, dabHeight);
            spacing *= spacingVal;
        }
    }
    else {
        qreal significantDimension = qMax(dabWidth, dabHeight);
        if (autoSpacingActive) {
            significantDimension = calcAutoSpacing(significantDimension, autoSpacingCoeff);
        } else {
            significantDimension *= spacingVal;
        }
        spacing = QPointF(significantDimension, significantDimension);
        rotation = 0.0;
        axesFlipped = false;
    }

    spacing *= extraScale;

    return KisSpacingInformation(distanceSpacingEnabled, spacing, rotation, axesFlipped);
}

KisTimingInformation effectiveTiming(bool timingEnabled, qreal timingInterval, qreal rateExtraScale)
{

    if (!timingEnabled) {
        return KisTimingInformation();
    }
    else {
        qreal scaledInterval = rateExtraScale <= 0.0 ? LONG_TIME : timingInterval / rateExtraScale;
        return KisTimingInformation(scaledInterval);
    }
}

QVector<QRect> splitAndFilterDabRect(const QRect &totalRect, const QVector<QRect> &dabRects, int idealPatchSize)
{
    QVector<QRect> rects = KritaUtils::splitRectIntoPatches(totalRect, QSize(idealPatchSize,idealPatchSize));

    KritaUtils::filterContainer(rects,
        [dabRects] (const QRect &rc) {
            Q_FOREACH (const QRect &dab, dabRects) {
                if (dab.intersects(rc)) {
                    return true;
                }
            }
            return false;
        });
    return rects;
}

QVector<QRect> splitDabsIntoRects(const QVector<QRect> &dabRects, int idealNumRects, int diameter, qreal spacing)
{
    const QRect totalRect =
        std::accumulate(dabRects.begin(), dabRects.end(), QRect(), std::bit_or<QRect>());

    constexpr int minPatchSize = 128;
    constexpr int maxPatchSize = 512;
    constexpr int patchStep = 64;
    constexpr int halfPatchStep = patchStep >> 1;


    int idealPatchSize = qBound(minPatchSize,
                                (int(diameter * (2.0 - spacing)) + halfPatchStep) & ~(patchStep - 1),
                                maxPatchSize);


    QVector<QRect> rects = splitAndFilterDabRect(totalRect, dabRects, idealPatchSize);

    while (rects.size() < idealNumRects && idealPatchSize >minPatchSize) {
        idealPatchSize = qMax(minPatchSize, idealPatchSize - patchStep);
        rects = splitAndFilterDabRect(totalRect, dabRects, idealPatchSize);
    }

    return rects;
}



}
