/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoCompositeOpRegistry.h"

#include <QGlobalStatic>
#include <QList>

#include <klocalizedstring.h>

#include <KoID.h>
#include "KoCompositeOp.h"
#include "KoColorSpace.h"

Q_GLOBAL_STATIC(KoCompositeOpRegistry, registry)


KoCompositeOpRegistry::KoCompositeOpRegistry()
{
    m_categories
        << KoID("arithmetic", i18n("Arithmetic"))
        << KoID("binary"    , i18n("Binary"))
        << KoID("dark"      , i18n("Darken"))
        << KoID("light"     , i18n("Lighten"))
        << KoID("modulo"       , i18n("Modulo"))
        << KoID("negative"  , i18n("Negative"))
        << KoID("mix"       , i18n("Mix"))
        << KoID("misc"      , i18n("Misc"))
        << KoID("hsy"       , i18n("HSY"))
        << KoID("hsi"       , i18n("HSI"))
        << KoID("hsl"       , i18n("HSL"))
        << KoID("hsv"       , i18n("HSV"))
        << KoID("quadratic" , i18n("Quadratic"));

    m_map.insert(m_categories[0], KoID(COMPOSITE_ADD             , i18n("Addition")));
    m_map.insert(m_categories[0], KoID(COMPOSITE_SUBTRACT        , i18n("Subtract")));
    m_map.insert(m_categories[0], KoID(COMPOSITE_MULT            , i18n("Multiply")));
    m_map.insert(m_categories[0], KoID(COMPOSITE_DIVIDE          , i18n("Divide")));
    m_map.insert(m_categories[0], KoID(COMPOSITE_INVERSE_SUBTRACT, i18n("Inverse Subtract")));
    
    m_map.insert(m_categories[1], KoID(COMPOSITE_XOR             , i18n("XOR")));
    m_map.insert(m_categories[1], KoID(COMPOSITE_OR              , i18n("OR")));
    m_map.insert(m_categories[1], KoID(COMPOSITE_AND             , i18n("AND")));
    m_map.insert(m_categories[1], KoID(COMPOSITE_NAND            , i18n("NAND")));
    m_map.insert(m_categories[1], KoID(COMPOSITE_NOR             , i18n("NOR")));
    m_map.insert(m_categories[1], KoID(COMPOSITE_XNOR            , i18n("XNOR")));
    m_map.insert(m_categories[1], KoID(COMPOSITE_IMPLICATION     , i18n("IMPLICATION")));
    m_map.insert(m_categories[1], KoID(COMPOSITE_NOT_IMPLICATION , i18n("NOT IMPLICATION")));
    m_map.insert(m_categories[1], KoID(COMPOSITE_CONVERSE        , i18n("CONVERSE")));
    m_map.insert(m_categories[1], KoID(COMPOSITE_NOT_CONVERSE    , i18n("NOT CONVERSE")));

    m_map.insert(m_categories[2], KoID(COMPOSITE_BURN       , i18n("Burn")));
    m_map.insert(m_categories[2], KoID(COMPOSITE_LINEAR_BURN, i18n("Linear Burn")));
    m_map.insert(m_categories[2], KoID(COMPOSITE_DARKEN     , i18n("Darken")));
    m_map.insert(m_categories[2], KoID(COMPOSITE_GAMMA_DARK , i18n("Gamma Dark")));
    m_map.insert(m_categories[2], KoID(COMPOSITE_DARKER_COLOR     , i18n("Darker Color")));
    m_map.insert(m_categories[2], KoID(COMPOSITE_SHADE_IFS_ILLUSIONS, i18n("Shade (IFS Illusions)")));
    m_map.insert(m_categories[2], KoID(COMPOSITE_FOG_DARKEN_IFS_ILLUSIONS, i18n("Fog Darken (IFS Illusions)")));
    m_map.insert(m_categories[2], KoID(COMPOSITE_EASY_BURN       , i18n("Easy Burn")));

    m_map.insert(m_categories[3], KoID(COMPOSITE_DODGE       , i18n("Color Dodge")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_LINEAR_DODGE, i18n("Linear Dodge")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_LIGHTEN     , i18n("Lighten")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_LINEAR_LIGHT, i18n("Linear Light")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_SCREEN      , i18n("Screen")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_PIN_LIGHT   , i18n("Pin Light")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_VIVID_LIGHT , i18n("Vivid Light")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_FLAT_LIGHT  , i18n("Flat Light")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_HARD_LIGHT  , i18n("Hard Light")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_SOFT_LIGHT_IFS_ILLUSIONS, i18n("Soft Light (IFS Illusions)")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_SOFT_LIGHT_PEGTOP_DELPHI, i18n("Soft Light (Pegtop-Delphi)")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_SOFT_LIGHT_PHOTOSHOP, i18n("Soft Light (Photoshop)")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_SOFT_LIGHT_SVG, i18n("Soft Light (SVG)")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_GAMMA_LIGHT , i18n("Gamma Light")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_GAMMA_ILLUMINATION , i18n("Gamma Illumination")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_LIGHTER_COLOR     , i18n("Lighter Color")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_PNORM_A           , i18n("P-Norm A")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_PNORM_B           , i18n("P-Norm B")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_SUPER_LIGHT     , i18n("Super Light")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_TINT_IFS_ILLUSIONS, i18n("Tint (IFS Illusions)")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_FOG_LIGHTEN_IFS_ILLUSIONS, i18n("Fog Lighten (IFS Illusions)")));
    m_map.insert(m_categories[3], KoID(COMPOSITE_EASY_DODGE       , i18n("Easy Dodge")));
    
    m_map.insert(m_categories[4], KoID(COMPOSITE_MOD              , i18n("Modulo")));
    m_map.insert(m_categories[4], KoID(COMPOSITE_MOD_CON          , i18n("Modulo - Continuous")));
    m_map.insert(m_categories[4], KoID(COMPOSITE_DIVISIVE_MOD     , i18n("Divisive Modulo")));
    m_map.insert(m_categories[4], KoID(COMPOSITE_DIVISIVE_MOD_CON , i18n("Divisive Modulo - Continuous")));
    m_map.insert(m_categories[4], KoID(COMPOSITE_MODULO_SHIFT     , i18n("Modulo Shift")));
    m_map.insert(m_categories[4], KoID(COMPOSITE_MODULO_SHIFT_CON , i18n("Modulo Shift - Continuous")));

    m_map.insert(m_categories[5], KoID(COMPOSITE_DIFF                 , i18n("Difference")));
    m_map.insert(m_categories[5], KoID(COMPOSITE_EQUIVALENCE          , i18n("Equivalence")));
    m_map.insert(m_categories[5], KoID(COMPOSITE_ADDITIVE_SUBTRACTIVE , i18n("Additive Subtractive")));
    m_map.insert(m_categories[5], KoID(COMPOSITE_EXCLUSION            , i18n("Exclusion")));
    m_map.insert(m_categories[5], KoID(COMPOSITE_ARC_TANGENT          , i18n("Arcus Tangent")));
    m_map.insert(m_categories[5], KoID(COMPOSITE_NEGATION             , i18n("Negation")));

    m_map.insert(m_categories[6], KoID(COMPOSITE_OVER            , i18n("Normal")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_BEHIND          , i18n("Behind")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_GREATER         , i18n("Greater")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_OVERLAY         , i18n("Overlay")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_ERASE           , i18n("Erase")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_ALPHA_DARKEN    , i18n("Alpha Darken")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_HARD_MIX        , i18n("Hard Mix")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_HARD_MIX_PHOTOSHOP, i18n("Hard Mix (Photoshop)")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_GRAIN_MERGE     , i18n("Grain Merge")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_GRAIN_EXTRACT   , i18n("Grain Extract")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_PARALLEL        , i18n("Parallel")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_ALLANON         , i18n("Allanon")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_GEOMETRIC_MEAN  , i18n("Geometric Mean")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_DESTINATION_ATOP, i18n("Destination Atop")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_DESTINATION_IN  , i18n("Destination In")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_HARD_OVERLAY    , i18n("Hard Overlay")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_INTERPOLATION   , i18n("Interpolation")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_INTERPOLATIONB  , i18n("Interpolation - 2X")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_PENUMBRAA       , i18n("Penumbra A")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_PENUMBRAB       , i18n("Penumbra B")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_PENUMBRAC       , i18n("Penumbra C")));
    m_map.insert(m_categories[6], KoID(COMPOSITE_PENUMBRAD       , i18n("Penumbra D")));

    m_map.insert(m_categories[7], KoID(COMPOSITE_BUMPMAP   , i18n("Bumpmap")));
    m_map.insert(m_categories[7], KoID(COMPOSITE_COMBINE_NORMAL, i18n("Combine Normal Map")));
    m_map.insert(m_categories[7], KoID(COMPOSITE_DISSOLVE  , i18n("Dissolve")));
    m_map.insert(m_categories[7], KoID(COMPOSITE_COPY_RED  , i18n("Copy Red")));
    m_map.insert(m_categories[7], KoID(COMPOSITE_COPY_GREEN, i18n("Copy Green")));
    m_map.insert(m_categories[7], KoID(COMPOSITE_COPY_BLUE , i18n("Copy Blue")));
    m_map.insert(m_categories[7], KoID(COMPOSITE_COPY      , i18n("Copy")));
    m_map.insert(m_categories[7], KoID(COMPOSITE_TANGENT_NORMALMAP, i18n("Tangent Normalmap")));

    m_map.insert(m_categories[8], KoID(COMPOSITE_COLOR         , i18n("Color")));
    m_map.insert(m_categories[8], KoID(COMPOSITE_HUE           , i18n("Hue")));
    m_map.insert(m_categories[8], KoID(COMPOSITE_SATURATION    , i18n("Saturation")));
    m_map.insert(m_categories[8], KoID(COMPOSITE_LUMINIZE      , i18n("Luminosity")));
    m_map.insert(m_categories[8], KoID(COMPOSITE_DEC_SATURATION, i18n("Decrease Saturation")));
    m_map.insert(m_categories[8], KoID(COMPOSITE_INC_SATURATION, i18n("Increase Saturation")));
    m_map.insert(m_categories[8], KoID(COMPOSITE_DEC_LUMINOSITY, i18n("Decrease Luminosity")));
    m_map.insert(m_categories[8], KoID(COMPOSITE_INC_LUMINOSITY, i18n("Increase Luminosity")));

    m_map.insert(m_categories[9], KoID(COMPOSITE_COLOR_HSI         , i18n("Color HSI")));
    m_map.insert(m_categories[9], KoID(COMPOSITE_HUE_HSI           , i18n("Hue HSI")));
    m_map.insert(m_categories[9], KoID(COMPOSITE_SATURATION_HSI    , i18n("Saturation HSI")));
    m_map.insert(m_categories[9], KoID(COMPOSITE_INTENSITY         , i18n("Intensity")));
    m_map.insert(m_categories[9], KoID(COMPOSITE_DEC_SATURATION_HSI, i18n("Decrease Saturation HSI")));
    m_map.insert(m_categories[9], KoID(COMPOSITE_INC_SATURATION_HSI, i18n("Increase Saturation HSI")));
    m_map.insert(m_categories[9], KoID(COMPOSITE_DEC_INTENSITY     , i18n("Decrease Intensity")));
    m_map.insert(m_categories[9], KoID(COMPOSITE_INC_INTENSITY     , i18n("Increase Intensity")));

    m_map.insert(m_categories[10], KoID(COMPOSITE_COLOR_HSL         , i18n("Color HSL")));
    m_map.insert(m_categories[10], KoID(COMPOSITE_HUE_HSL           , i18n("Hue HSL")));
    m_map.insert(m_categories[10], KoID(COMPOSITE_SATURATION_HSL    , i18n("Saturation HSL")));
    m_map.insert(m_categories[10], KoID(COMPOSITE_LIGHTNESS         , i18n("Lightness")));
    m_map.insert(m_categories[10], KoID(COMPOSITE_DEC_SATURATION_HSL, i18n("Decrease Saturation HSL")));
    m_map.insert(m_categories[10], KoID(COMPOSITE_INC_SATURATION_HSL, i18n("Increase Saturation HSL")));
    m_map.insert(m_categories[10], KoID(COMPOSITE_DEC_LIGHTNESS     , i18n("Decrease Lightness")));
    m_map.insert(m_categories[10], KoID(COMPOSITE_INC_LIGHTNESS     , i18n("Increase Lightness")));

    m_map.insert(m_categories[11], KoID(COMPOSITE_COLOR_HSV         , i18n("Color HSV")));
    m_map.insert(m_categories[11], KoID(COMPOSITE_HUE_HSV           , i18n("Hue HSV")));
    m_map.insert(m_categories[11], KoID(COMPOSITE_SATURATION_HSV    , i18n("Saturation HSV")));
    m_map.insert(m_categories[11], KoID(COMPOSITE_VALUE             , i18nc("HSV Value", "Value")));
    m_map.insert(m_categories[11], KoID(COMPOSITE_DEC_SATURATION_HSV, i18n("Decrease Saturation HSV")));
    m_map.insert(m_categories[11], KoID(COMPOSITE_INC_SATURATION_HSV, i18n("Increase Saturation HSV")));
    m_map.insert(m_categories[11], KoID(COMPOSITE_DEC_VALUE         , i18n("Decrease Value")));
    m_map.insert(m_categories[11], KoID(COMPOSITE_INC_VALUE         , i18n("Increase Value")));
    
    m_map.insert(m_categories[12], KoID(COMPOSITE_REFLECT          , i18n("Reflect")));
    m_map.insert(m_categories[12], KoID(COMPOSITE_GLOW             , i18n("Glow")));
    m_map.insert(m_categories[12], KoID(COMPOSITE_FREEZE           , i18n("Freeze")));
    m_map.insert(m_categories[12], KoID(COMPOSITE_HEAT             , i18n("Heat")));
    m_map.insert(m_categories[12], KoID(COMPOSITE_GLEAT            , i18n("Glow-Heat")));
    m_map.insert(m_categories[12], KoID(COMPOSITE_HELOW            , i18n("Heat-Glow")));
    m_map.insert(m_categories[12], KoID(COMPOSITE_REEZE            , i18n("Reflect-Freeze")));
    m_map.insert(m_categories[12], KoID(COMPOSITE_FRECT            , i18n("Freeze-Reflect")));
    m_map.insert(m_categories[12], KoID(COMPOSITE_FHYRD            , i18n("Heat-Glow & Freeze-Reflect Hybrid")));
}

const KoCompositeOpRegistry& KoCompositeOpRegistry::instance()
{
    return *registry;
}

KoID KoCompositeOpRegistry::getDefaultCompositeOp() const
{
    return KoID(COMPOSITE_OVER, i18n("Normal"));
}

KoID KoCompositeOpRegistry::getKoID(const QString& compositeOpID) const
{
    KoIDMap::const_iterator itr = std::find(m_map.begin(), m_map.end(), KoID(compositeOpID));
    return (itr != m_map.end()) ? *itr : KoID();
}

KoCompositeOpRegistry::KoIDMap KoCompositeOpRegistry::getCompositeOps() const
{
    return m_map;
}

KoCompositeOpRegistry::KoIDList KoCompositeOpRegistry::getCategories() const
{
    return m_categories;
}

KoCompositeOpRegistry::KoIDList KoCompositeOpRegistry::getCompositeOps(const KoID& category, const KoColorSpace* colorSpace) const
{
    qint32                  num = m_map.count(category);
    KoIDMap::const_iterator beg = m_map.find(category);
    KoIDMap::const_iterator end = beg + num;

    KoIDList list;
    list.reserve(num);

    if(colorSpace) {
        for(; beg!=end; ++beg){
            if(colorSpace->hasCompositeOp(beg->id()))
                list.push_back(*beg);
        }
    }
    else {
        for(; beg!=end; ++beg)
            list.push_back(*beg);
    }

    return list;
}

KoCompositeOpRegistry::KoIDList KoCompositeOpRegistry::getCompositeOps(const KoColorSpace* colorSpace) const
{
    KoIDMap::const_iterator beg = m_map.begin();
    KoIDMap::const_iterator end = m_map.end();

    KoIDList list;
    list.reserve(m_map.size());

    if(colorSpace) {
        for(; beg!=end; ++beg){
            if(colorSpace->hasCompositeOp(beg->id()))
                list.push_back(*beg);
        }
    }
    else {
        for(; beg!=end; ++beg)
            list.push_back(*beg);
    }

    return list;
}

bool KoCompositeOpRegistry::colorSpaceHasCompositeOp(const KoColorSpace* colorSpace, const KoID& compositeOp) const
{
    return colorSpace ? colorSpace->hasCompositeOp(compositeOp.id()) : false;
}
