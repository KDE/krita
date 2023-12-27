/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISTEXTUREOPTIONDATA_H
#define KISTEXTUREOPTIONDATA_H

#include <boost/operators.hpp>
#include <kritapaintop_export.h>

#include "KisEmbeddedTextureData.h"

class KisPaintopLodLimitations;

enum KisBrushTextureFlag
{
    None = 0x0,
    SupportsLightnessMode = 0x1,
    SupportsGradientMode = 0x2,
};
Q_DECLARE_FLAGS(KisBrushTextureFlags, KisBrushTextureFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(KisBrushTextureFlags)



struct  PAINTOP_EXPORT KisTextureOptionData : boost::equality_comparable<KisTextureOptionData>
{
    enum TexturingMode {
        MULTIPLY,
        SUBTRACT,
        LIGHTNESS,
        GRADIENT,
        DARKEN,
        OVERLAY,
        COLOR_DODGE,
        COLOR_BURN,
        LINEAR_DODGE,
        LINEAR_BURN,
        HARD_MIX_PHOTOSHOP,
        HARD_MIX_SOFTER_PHOTOSHOP,
        HEIGHT,
        LINEAR_HEIGHT,
        HEIGHT_PHOTOSHOP,
        LINEAR_HEIGHT_PHOTOSHOP
    };

    inline friend bool operator==(const KisTextureOptionData &lhs, const KisTextureOptionData &rhs) {
        return lhs.isEnabled == rhs.isEnabled &&
                lhs.textureData == rhs.textureData &&
                qFuzzyCompare(lhs.scale, rhs.scale) &&
                qFuzzyCompare(lhs.brightness, rhs.brightness) &&
                qFuzzyCompare(lhs.contrast, rhs.contrast) &&
                qFuzzyCompare(lhs.neutralPoint, rhs.neutralPoint) &&
                lhs.offsetX == rhs.offsetX &&
                lhs.offsetY == rhs.offsetY &&
                lhs.maximumOffsetX == rhs.maximumOffsetX &&
                lhs.maximumOffsetY == rhs.maximumOffsetY &&
                lhs.isRandomOffsetX == rhs.isRandomOffsetX &&
                lhs.isRandomOffsetY == rhs.isRandomOffsetY &&
                lhs.texturingMode == rhs.texturingMode &&
                lhs.cutOffPolicy == rhs.cutOffPolicy &&
                lhs.cutOffLeft == rhs.cutOffLeft &&
                lhs.cutOffRight == rhs.cutOffRight &&
                lhs.invert == rhs.invert;
    }

    KisEmbeddedTextureData textureData;

    bool isEnabled {false};
    qreal scale {1.0};
    qreal brightness {0.0};
    qreal contrast {1.0};
    qreal neutralPoint {0.5};
    int offsetX {0};
    int offsetY {0};
    int maximumOffsetX {0};
    int maximumOffsetY {0};
    bool isRandomOffsetX {false};
    bool isRandomOffsetY {false};
    TexturingMode texturingMode {MULTIPLY};
    int cutOffPolicy {0};
    int cutOffLeft {0};
    int cutOffRight {255};
    bool invert {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

    KisPaintopLodLimitations lodLimitations() const;
};


#endif // KISTEXTUREOPTIONDATA_H
