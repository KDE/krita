/*
 * Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 * Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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
#ifndef KOCOMPOSITEOPREGISTRY_H
#define KOCOMPOSITEOPREGISTRY_H

#include <QString>
#include <QList>
#include <QMultiMap>
#include <QBitArray>

#include "kritapigment_export.h"

class KoColorSpace;
#include <KoID.h>

// TODO : convert this data blob into a modern design with an enum class.
// This will reduce the need for runtime string comparisons.

const QString COMPOSITE_OVER         = "normal";
const QString COMPOSITE_ERASE        = "erase";
const QString COMPOSITE_IN           = "in";
const QString COMPOSITE_OUT          = "out";
const QString COMPOSITE_ALPHA_DARKEN = "alphadarken";
const QString COMPOSITE_DESTINATION_IN = "destination-in";
const QString COMPOSITE_DESTINATION_ATOP = "destination-atop";

const QString COMPOSITE_XOR                   = "xor";
const QString COMPOSITE_OR                    = "or";
const QString COMPOSITE_AND                   = "and";
const QString COMPOSITE_NAND                  = "nand";
const QString COMPOSITE_NOR                   = "nor";
const QString COMPOSITE_XNOR                  = "xnor";
const QString COMPOSITE_IMPLICATION           = "implication";
const QString COMPOSITE_NOT_IMPLICATION       = "not_implication";
const QString COMPOSITE_CONVERSE              = "converse";
const QString COMPOSITE_NOT_CONVERSE          = "not_converse";

const QString COMPOSITE_PLUS                  = "plus";
const QString COMPOSITE_MINUS                 = "minus";
const QString COMPOSITE_ADD                   = "add";
const QString COMPOSITE_SUBTRACT              = "subtract";
const QString COMPOSITE_INVERSE_SUBTRACT      = "inverse_subtract";
const QString COMPOSITE_DIFF                  = "diff";
const QString COMPOSITE_MULT                  = "multiply";
const QString COMPOSITE_DIVIDE                = "divide";
const QString COMPOSITE_ARC_TANGENT           = "arc_tangent";
const QString COMPOSITE_GEOMETRIC_MEAN        = "geometric_mean";
const QString COMPOSITE_ADDITIVE_SUBTRACTIVE  = "additive_subtractive";
const QString COMPOSITE_NEGATION              = "negation";

const QString COMPOSITE_MOD                = "modulo";
const QString COMPOSITE_MOD_CON            = "modulo_continuous";
const QString COMPOSITE_DIVISIVE_MOD       = "divisive_modulo";
const QString COMPOSITE_DIVISIVE_MOD_CON   = "divisive_modulo_continuous";
const QString COMPOSITE_MODULO_SHIFT       = "modulo_shift";
const QString COMPOSITE_MODULO_SHIFT_CON   = "modulo_shift_continuous";

const QString COMPOSITE_EQUIVALENCE   = "equivalence";
const QString COMPOSITE_ALLANON       = "allanon";
const QString COMPOSITE_PARALLEL      = "parallel";
const QString COMPOSITE_GRAIN_MERGE   = "grain_merge";
const QString COMPOSITE_GRAIN_EXTRACT = "grain_extract";
const QString COMPOSITE_EXCLUSION     = "exclusion";
const QString COMPOSITE_HARD_MIX      = "hard mix";
const QString COMPOSITE_HARD_MIX_PHOTOSHOP = "hard_mix_photoshop";
const QString COMPOSITE_OVERLAY       = "overlay";
const QString COMPOSITE_BEHIND        = "behind";
const QString COMPOSITE_GREATER       = "greater";
const QString COMPOSITE_HARD_OVERLAY  = "hard overlay";
const QString COMPOSITE_INTERPOLATION = "interpolation";
const QString COMPOSITE_INTERPOLATIONB = "interpolation 2x";
const QString COMPOSITE_PENUMBRAA     = "penumbra a";
const QString COMPOSITE_PENUMBRAB     = "penumbra b";
const QString COMPOSITE_PENUMBRAC     = "penumbra c";
const QString COMPOSITE_PENUMBRAD     = "penumbra d";

const QString COMPOSITE_DARKEN      = "darken";
const QString COMPOSITE_BURN        = "burn";//this is also known as 'color burn'.
const QString COMPOSITE_LINEAR_BURN = "linear_burn";
const QString COMPOSITE_GAMMA_DARK  = "gamma_dark";
const QString COMPOSITE_SHADE_IFS_ILLUSIONS = "shade_ifs_illusions";
const QString COMPOSITE_FOG_DARKEN_IFS_ILLUSIONS = "fog_darken_ifs_illusions";
const QString COMPOSITE_EASY_BURN        = "easy burn";

const QString COMPOSITE_LIGHTEN      = "lighten";
const QString COMPOSITE_DODGE        = "dodge";
const QString COMPOSITE_LINEAR_DODGE = "linear_dodge";
const QString COMPOSITE_SCREEN       = "screen";
const QString COMPOSITE_HARD_LIGHT   = "hard_light";
const QString COMPOSITE_SOFT_LIGHT_IFS_ILLUSIONS = "soft_light_ifs_illusions";
const QString COMPOSITE_SOFT_LIGHT_PEGTOP_DELPHI = "soft_light_pegtop_delphi";
const QString COMPOSITE_SOFT_LIGHT_PHOTOSHOP = "soft_light";
const QString COMPOSITE_SOFT_LIGHT_SVG  = "soft_light_svg";
const QString COMPOSITE_GAMMA_LIGHT  = "gamma_light";
const QString COMPOSITE_GAMMA_ILLUMINATION  = "gamma_illumination";
const QString COMPOSITE_VIVID_LIGHT  = "vivid_light";
const QString COMPOSITE_FLAT_LIGHT   = "flat_light";
const QString COMPOSITE_LINEAR_LIGHT = "linear light";
const QString COMPOSITE_PIN_LIGHT    = "pin_light";
const QString COMPOSITE_PNORM_A        = "pnorm_a";
const QString COMPOSITE_PNORM_B        = "pnorm_b";
const QString COMPOSITE_SUPER_LIGHT  = "super_light";
const QString COMPOSITE_TINT_IFS_ILLUSIONS = "tint_ifs_illusions";
const QString COMPOSITE_FOG_LIGHTEN_IFS_ILLUSIONS = "fog_lighten_ifs_illusions";
const QString COMPOSITE_EASY_DODGE        = "easy dodge";

const QString COMPOSITE_HUE            = "hue";
const QString COMPOSITE_COLOR          = "color";
const QString COMPOSITE_SATURATION     = "saturation";
const QString COMPOSITE_INC_SATURATION = "inc_saturation";
const QString COMPOSITE_DEC_SATURATION = "dec_saturation";
const QString COMPOSITE_LUMINIZE       = "luminize";
const QString COMPOSITE_INC_LUMINOSITY = "inc_luminosity";
const QString COMPOSITE_DEC_LUMINOSITY = "dec_luminosity";

const QString COMPOSITE_HUE_HSV            = "hue_hsv";
const QString COMPOSITE_COLOR_HSV          = "color_hsv";
const QString COMPOSITE_SATURATION_HSV     = "saturation_hsv";
const QString COMPOSITE_INC_SATURATION_HSV = "inc_saturation_hsv";
const QString COMPOSITE_DEC_SATURATION_HSV = "dec_saturation_hsv";
const QString COMPOSITE_VALUE              = "value";
const QString COMPOSITE_INC_VALUE          = "inc_value";
const QString COMPOSITE_DEC_VALUE          = "dec_value";

const QString COMPOSITE_HUE_HSL            = "hue_hsl";
const QString COMPOSITE_COLOR_HSL          = "color_hsl";
const QString COMPOSITE_SATURATION_HSL     = "saturation_hsl";
const QString COMPOSITE_INC_SATURATION_HSL = "inc_saturation_hsl";
const QString COMPOSITE_DEC_SATURATION_HSL = "dec_saturation_hsl";
const QString COMPOSITE_LIGHTNESS          = "lightness";
const QString COMPOSITE_INC_LIGHTNESS      = "inc_lightness";
const QString COMPOSITE_DEC_LIGHTNESS      = "dec_lightness";

const QString COMPOSITE_HUE_HSI            = "hue_hsi";
const QString COMPOSITE_COLOR_HSI          = "color_hsi";
const QString COMPOSITE_SATURATION_HSI     = "saturation_hsi";
const QString COMPOSITE_INC_SATURATION_HSI = "inc_saturation_hsi";
const QString COMPOSITE_DEC_SATURATION_HSI = "dec_saturation_hsi";
const QString COMPOSITE_INTENSITY          = "intensity";
const QString COMPOSITE_INC_INTENSITY      = "inc_intensity";
const QString COMPOSITE_DEC_INTENSITY      = "dec_intensity";

const QString COMPOSITE_COPY         = "copy";
const QString COMPOSITE_COPY_RED     = "copy_red";
const QString COMPOSITE_COPY_GREEN   = "copy_green";
const QString COMPOSITE_COPY_BLUE    = "copy_blue";
const QString COMPOSITE_TANGENT_NORMALMAP    = "tangent_normalmap";

const QString COMPOSITE_COLORIZE     = "colorize";
const QString COMPOSITE_BUMPMAP      = "bumpmap";
const QString COMPOSITE_COMBINE_NORMAL = "combine_normal";
const QString COMPOSITE_CLEAR        = "clear";
const QString COMPOSITE_DISSOLVE     = "dissolve";
const QString COMPOSITE_DISPLACE     = "displace";
const QString COMPOSITE_NO           = "nocomposition";
const QString COMPOSITE_PASS_THROUGH = "pass through"; // XXX: not implemented anywhere yet
const QString COMPOSITE_DARKER_COLOR = "darker color";
const QString COMPOSITE_LIGHTER_COLOR = "lighter color";
const QString COMPOSITE_UNDEF        = "undefined";

const QString COMPOSITE_REFLECT   = "reflect";
const QString COMPOSITE_GLOW      = "glow";
const QString COMPOSITE_FREEZE    = "freeze";
const QString COMPOSITE_HEAT      = "heat";
const QString COMPOSITE_GLEAT     = "glow_heat";
const QString COMPOSITE_HELOW     = "heat_glow";
const QString COMPOSITE_REEZE     = "reflect_freeze";
const QString COMPOSITE_FRECT     = "freeze_reflect";
const QString COMPOSITE_FHYRD     = "heat_glow_freeze_reflect_hybrid";



class KRITAPIGMENT_EXPORT KoCompositeOpRegistry
{
    typedef QMultiMap<KoID,KoID> KoIDMap;
    typedef QList<KoID>          KoIDList;

public:
    KoCompositeOpRegistry();
    static const KoCompositeOpRegistry& instance();

    KoID     getDefaultCompositeOp() const;
    KoID     getKoID(const QString& compositeOpID) const;
    KoIDMap  getCompositeOps() const;
    KoIDList getCategories() const;
    KoIDList getCompositeOps(const KoColorSpace* colorSpace) const;
    KoIDList getCompositeOps(const KoID& category, const KoColorSpace* colorSpace=0) const;
    bool     colorSpaceHasCompositeOp(const KoColorSpace* colorSpace, const KoID& compositeOp) const;

    template<class TKoIdIterator>
    KoIDList filterCompositeOps(TKoIdIterator begin, TKoIdIterator end, const KoColorSpace* colorSpace, bool removeInvaliOps=true) const {
        KoIDList list;

        for(; begin!=end; ++begin){
            if( colorSpaceHasCompositeOp(colorSpace, *begin) == removeInvaliOps)
                list.push_back(*begin);
        }

        return list;
    }

private:
    KoIDList m_categories;
    KoIDMap  m_map;
};


#endif // KOCOMPOSITEOPREGISTRY_H
