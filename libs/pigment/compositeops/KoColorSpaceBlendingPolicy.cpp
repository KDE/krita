/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoColorSpaceBlendingPolicy.h"

#include <KoCompositeOpRegistry.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

bool useSubtractiveBlendingForCmykColorSpaces()
{
    static bool isConfigInitialized = false;
    static bool useSubtractiveBlending = true;

    if (!isConfigInitialized) {
        KConfigGroup cfg = KSharedConfig::openConfig()->group("");
        useSubtractiveBlending = cfg.readEntry("useSubtractiveBlendingForCmykColorSpaces", true);
        isConfigInitialized = true;

        if (!useSubtractiveBlending) {
            qInfo() << "INFO: requested old version of CMYK blending mode. Switching...";
        }
    }

    return useSubtractiveBlending;
}

QStringList subtractiveBlendingModesInCmyk()
{
    /**
     * Here is the list of blendmodes which are not invariant
     * to color channel inversion, therefore they cannot work in
     * CMYK properly. These modes automatically invert the channels
     * before blending when used for CMYK color space.
     *
     * This is a behavior-change in Krita 5.2
     */

    QStringList ids;

    ids << COMPOSITE_BEHIND;
    ids << COMPOSITE_GREATER;

    ids << COMPOSITE_OVERLAY;
    ids << COMPOSITE_GRAIN_MERGE;
    ids << COMPOSITE_GRAIN_EXTRACT;
    ids << COMPOSITE_HARD_MIX;
    ids << COMPOSITE_HARD_MIX_PHOTOSHOP;
    ids << COMPOSITE_HARD_MIX_SOFTER_PHOTOSHOP;
    ids << COMPOSITE_GEOMETRIC_MEAN;
    ids << COMPOSITE_PARALLEL;
    ids << COMPOSITE_ALLANON;
    ids << COMPOSITE_HARD_OVERLAY;
    ids << COMPOSITE_INTERPOLATION;
    ids << COMPOSITE_INTERPOLATIONB;
    ids << COMPOSITE_PENUMBRAA;
    ids << COMPOSITE_PENUMBRAB;
    ids << COMPOSITE_PENUMBRAC;
    ids << COMPOSITE_PENUMBRAD;
    ids << COMPOSITE_SCREEN;
    ids << COMPOSITE_DODGE;
    ids << COMPOSITE_LINEAR_DODGE;
    ids << COMPOSITE_LIGHTEN;
    ids << COMPOSITE_HARD_LIGHT;
    ids << COMPOSITE_SOFT_LIGHT_IFS_ILLUSIONS;
    ids << COMPOSITE_SOFT_LIGHT_PEGTOP_DELPHI;
    ids << COMPOSITE_SOFT_LIGHT_SVG;
    ids << COMPOSITE_SOFT_LIGHT_PHOTOSHOP;
    ids << COMPOSITE_GAMMA_LIGHT;
    ids << COMPOSITE_GAMMA_ILLUMINATION;
    ids << COMPOSITE_VIVID_LIGHT;
    ids << COMPOSITE_FLAT_LIGHT;
    ids << COMPOSITE_PIN_LIGHT;
    ids << COMPOSITE_LINEAR_LIGHT;
    ids << COMPOSITE_PNORM_A;
    ids << COMPOSITE_PNORM_B;
    ids << COMPOSITE_SUPER_LIGHT;
    ids << COMPOSITE_TINT_IFS_ILLUSIONS;
    ids << COMPOSITE_FOG_LIGHTEN_IFS_ILLUSIONS;
    ids << COMPOSITE_EASY_DODGE;
    ids << COMPOSITE_BURN;
    ids << COMPOSITE_LINEAR_BURN;
    ids << COMPOSITE_DARKEN;
    ids << COMPOSITE_GAMMA_DARK;
    ids << COMPOSITE_SHADE_IFS_ILLUSIONS;
    ids << COMPOSITE_FOG_DARKEN_IFS_ILLUSIONS;
    ids << COMPOSITE_EASY_BURN;
    ids << COMPOSITE_ADD;
    ids << COMPOSITE_SUBTRACT;
    ids << COMPOSITE_INVERSE_SUBTRACT;
    ids << COMPOSITE_MULT;
    ids << COMPOSITE_DIVIDE;
    ids << COMPOSITE_MOD;
    ids << COMPOSITE_MOD_CON;
    ids << COMPOSITE_DIVISIVE_MOD;
    ids << COMPOSITE_DIVISIVE_MOD_CON;
    ids << COMPOSITE_MODULO_SHIFT;
    ids << COMPOSITE_MODULO_SHIFT_CON;
    ids << COMPOSITE_ARC_TANGENT;
    ids << COMPOSITE_DIFF;
    ids << COMPOSITE_EXCLUSION;
    ids << COMPOSITE_EQUIVALENCE;
    ids << COMPOSITE_ADDITIVE_SUBTRACTIVE;
    ids << COMPOSITE_NEGATION;

    ids << COMPOSITE_XOR;
    ids << COMPOSITE_OR;
    ids << COMPOSITE_AND;
    ids << COMPOSITE_NAND;
    ids << COMPOSITE_NOR;
    ids << COMPOSITE_XNOR;
    ids << COMPOSITE_IMPLICATION;
    ids << COMPOSITE_NOT_IMPLICATION;
    ids << COMPOSITE_CONVERSE;
    ids << COMPOSITE_NOT_CONVERSE;

    ids << COMPOSITE_REFLECT;
    ids << COMPOSITE_GLOW;
    ids << COMPOSITE_FREEZE;
    ids << COMPOSITE_HEAT;
    ids << COMPOSITE_GLEAT;
    ids << COMPOSITE_HELOW;
    ids << COMPOSITE_REEZE;
    ids << COMPOSITE_FRECT;
    ids << COMPOSITE_FHYRD;

    ids << COMPOSITE_LUMINOSITY_SAI;

    return ids;
}
