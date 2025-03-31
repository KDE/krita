/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "GutterWidthsConfig.h"
#include <kis_algebra_2d.h>
#include <kis_debug.h>


GutterWidthsConfig::GutterWidthsConfig(KoUnit _baseUnit, qreal _resolution, qreal _horizontal, qreal _vertical, qreal _diagonal, qreal _angleDegrees)
    : baseUnit(_baseUnit)
    , resolution(_resolution)
    , horizontal(_horizontal)
    , vertical(_vertical)
    , diagonal(_diagonal)
    , angleDegrees(_angleDegrees)
{
}


GutterWidthsConfig::GutterWidthsConfig(KoUnit _baseUnit, qreal _resolution, qreal _all, qreal _angleDegrees)
    : baseUnit(_baseUnit)
    , resolution(_resolution)
    , horizontal(_all)
    , vertical(_all)
    , diagonal(_all)
    , angleDegrees(_angleDegrees)
{
}

bool inRangeWrapped(qreal value, qreal min, qreal max, qreal rangeMax) {
    min = KisAlgebra2D::wrapValue(min, 0.0, rangeMax);
    max = KisAlgebra2D::wrapValue(max, 0.0, rangeMax);

    qCritical() << "in range wrapped: " << ppVar(value) << ppVar(min) << ppVar(max) << ppVar(rangeMax);

    if (min > max) {
        return value > min || value < max; // range is on the edges instead of in the middle
    } else {
        return value > min && value < max;
    }



}

qreal convertToPixels(KoUnit baseUnit, qreal resolution, qreal length)
{
    KoUnit toUnit = KoUnit::fromSymbol("px");
    return KoUnit::convertFromUnitToUnit(length, baseUnit, toUnit, resolution);
}

qreal GutterWidthsConfig::widthForAngleInPixels(qreal lineAngleDegrees)
{
    qreal angleMax = 360;
    qCritical() << "~~~ Searching for line angle: " << lineAngleDegrees;
    qCritical() << "~~~ Values for gutters are: " << horizontal << vertical << diagonal << "with angle diff = " << angleDegrees;

    if (inRangeWrapped(lineAngleDegrees, -angleDegrees, angleDegrees, angleMax)) {
        qCritical() << "horizontal 1";
        return convertToPixels(baseUnit, resolution, horizontal);
    } else if (inRangeWrapped(lineAngleDegrees, 180-angleDegrees, 180+angleDegrees, angleMax)) {
        qCritical() << "horizontal 2";
        return convertToPixels(baseUnit, resolution, horizontal);
    } else if (inRangeWrapped(lineAngleDegrees, 90-angleDegrees, 90+angleDegrees, angleMax)) {
        qCritical() << "vertical 1";
        return convertToPixels(baseUnit, resolution, vertical);
    } else if (inRangeWrapped(lineAngleDegrees, 270-angleDegrees, 270+angleDegrees, angleMax)) {
        qCritical() << "vertical 2";
        return convertToPixels(baseUnit, resolution, vertical);
    } else {
        qCritical() << "diagonal";
        return convertToPixels(baseUnit, resolution, diagonal);
    }
}
