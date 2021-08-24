/*
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd.h"

#include <QPoint>
#include <kis_debug.h>

#include <KoColorModelStandardIds.h>
#include <KoCompositeOpRegistry.h>

#include "kis_global.h"

QPair<QString, QString> psd_colormode_to_colormodelid(psd_color_mode colormode, quint16 channelDepth)
{
    QPair<QString, QString> colorSpaceId;
    switch (colormode) {
    case (Bitmap):
    case (Indexed):
    case (MultiChannel):
    case (RGB):
        colorSpaceId.first = RGBAColorModelID.id();
        break;
    case (CMYK):
        colorSpaceId.first = CMYKAColorModelID.id();
        break;
    case (Grayscale):
        colorSpaceId.first = GrayAColorModelID.id();
        break;
    case (DuoTone):
        colorSpaceId.first = GrayAColorModelID.id();
        break;
    case (Lab):
        colorSpaceId.first = LABAColorModelID.id();
        break;
    default:
        return colorSpaceId;
    }

    switch (channelDepth) {
    case (1):
    case (8):
        colorSpaceId.second = Integer8BitsColorDepthID.id();
        break;
    case (16):
        colorSpaceId.second = Integer16BitsColorDepthID.id();
        break;
    case (32):
        colorSpaceId.second = Float32BitsColorDepthID.id();
        break;
    default:
        break;
    }
    return colorSpaceId;
}

QString psd_blendmode_to_composite_op(const QString &blendmode)
{
    // 'pass' = pass through
    if (blendmode == "pass")
        return COMPOSITE_PASS_THROUGH;
    // 'norm' = normal
    if (blendmode == "norm")
        return COMPOSITE_OVER;
    // 'diss' = dissolve
    if (blendmode == "diss")
        return COMPOSITE_DISSOLVE;
    // 'dark' = darken
    if (blendmode == "dark")
        return COMPOSITE_DARKEN;
    // 'mul ' = multiply
    if (blendmode == "mul ")
        return COMPOSITE_MULT;
    // 'idiv' = color burn
    if (blendmode == "idiv")
        return COMPOSITE_BURN;
    // 'lbrn' = linear burn
    if (blendmode == "lbrn")
        return COMPOSITE_LINEAR_BURN;
    // 'dkCl' = darker color
    if (blendmode == "dkCl")
        return COMPOSITE_DARKER_COLOR;
    // 'lite' = lighten
    if (blendmode == "lite")
        return COMPOSITE_LIGHTEN;
    // 'scrn' = screen
    if (blendmode == "scrn")
        return COMPOSITE_SCREEN;
    // 'div ' = color dodge
    if (blendmode == "div ")
        return COMPOSITE_DODGE;
    // 'lddg' = linear dodge
    if (blendmode == "lddg")
        return COMPOSITE_LINEAR_DODGE;
    // 'lgCl' = lighter color
    if (blendmode == "lgCl")
        return COMPOSITE_LIGHTER_COLOR;
    // 'over' = overlay
    if (blendmode == "over")
        return COMPOSITE_OVERLAY;
    // 'sLit' = soft light
    if (blendmode == "sLit")
        return COMPOSITE_SOFT_LIGHT_PHOTOSHOP;
    // 'hLit' = hard light
    if (blendmode == "hLit")
        return COMPOSITE_HARD_LIGHT;
    // 'vLit' = vivid light
    if (blendmode == "vLit")
        return COMPOSITE_VIVID_LIGHT;
    // 'lLit' = linear light
    if (blendmode == "lLit")
        return COMPOSITE_LINEAR_LIGHT;
    // 'pLit' = pin light
    if (blendmode == "pLit")
        return COMPOSITE_PIN_LIGHT;
    // 'hMix' = hard mix
    if (blendmode == "hMix")
        return COMPOSITE_HARD_MIX_PHOTOSHOP;
    // 'diff' = difference
    if (blendmode == "diff")
        return COMPOSITE_DIFF;
    // 'smud' = exclusion
    if (blendmode == "smud")
        return COMPOSITE_EXCLUSION;
    // 'fsub' = subtract
    if (blendmode == "fsub")
        return COMPOSITE_SUBTRACT;
    // 'fdiv' = divide
    if (blendmode == "fdiv")
        return COMPOSITE_DIVIDE;
    // 'hue ' = hue
    if (blendmode == "hue ")
        return COMPOSITE_HUE;
    // 'sat ' = saturation
    if (blendmode == "sat ")
        return COMPOSITE_SATURATION;
    // 'colr' = color
    if (blendmode == "colr")
        return COMPOSITE_COLOR;
    // 'lum ' = luminosity
    if (blendmode == "lum ")
        return COMPOSITE_LUMINIZE;

    dbgKrita << "Unknown blendmode:" << blendmode << ". Returning Normal";
    return COMPOSITE_OVER;
}

QString composite_op_to_psd_blendmode(const QString &compositeop)
{
    // 'pass' = pass through
    if (compositeop == COMPOSITE_PASS_THROUGH)
        return "pass";
    // 'norm' = normal
    if (compositeop == COMPOSITE_OVER)
        return "norm";
    // 'diss' = dissolve
    if (compositeop == COMPOSITE_DISSOLVE)
        return "diss";
    // 'dark' = darken
    if (compositeop == COMPOSITE_DARKEN)
        return "dark";
    // 'mul ' = multiply
    if (compositeop == COMPOSITE_MULT)
        return "mul ";
    // 'idiv' = color burn
    if (compositeop == COMPOSITE_BURN)
        return "idiv";
    // 'lbrn' = linear burn
    if (compositeop == COMPOSITE_LINEAR_BURN)
        return "lbrn";
    // 'dkCl' = darker color
    if (compositeop == COMPOSITE_DARKER_COLOR)
        return "dkCl";
    // 'lite' = lighten
    if (compositeop == COMPOSITE_LIGHTEN)
        return "lite";
    // 'scrn' = screen
    if (compositeop == COMPOSITE_SCREEN)
        return "scrn";
    // 'div ' = color dodge
    if (compositeop == COMPOSITE_DODGE)
        return "div ";
    // 'lddg' = linear dodge
    if (compositeop == COMPOSITE_LINEAR_DODGE)
        return "lddg";
    // 'lgCl' = lighter color
    if (compositeop == COMPOSITE_LIGHTER_COLOR)
        return "lgCl";
    // 'over' = overlay
    if (compositeop == COMPOSITE_OVERLAY)
        return "over";
    // 'sLit' = soft light
    if (compositeop == COMPOSITE_SOFT_LIGHT_PHOTOSHOP)
        return "sLit";
    if (compositeop == COMPOSITE_SOFT_LIGHT_SVG)
        return "sLit";
    // 'hLit' = hard light
    if (compositeop == COMPOSITE_HARD_LIGHT)
        return "hLit";
    // 'vLit' = vivid light
    if (compositeop == COMPOSITE_VIVID_LIGHT)
        return "vLit";
    // 'lLit' = linear light
    if (compositeop == COMPOSITE_LINEAR_LIGHT)
        return "lLit";
    // 'pLit' = pin light
    if (compositeop == COMPOSITE_PIN_LIGHT)
        return "pLit";
    // 'hMix' = hard mix
    if (compositeop == COMPOSITE_HARD_MIX_PHOTOSHOP)
        return "hMix";
    // 'diff' = difference
    if (compositeop == COMPOSITE_DIFF)
        return "diff";
    // 'smud' = exclusion
    if (compositeop == COMPOSITE_EXCLUSION)
        return "smud";
    // 'fsub' = subtract
    if (compositeop == COMPOSITE_SUBTRACT)
        return "fsub";
    // 'fdiv' = divide
    if (compositeop == COMPOSITE_DIVIDE)
        return "fdiv";
    // 'hue ' = hue
    if (compositeop == COMPOSITE_HUE)
        return "hue ";
    // 'sat ' = saturation
    if (compositeop == COMPOSITE_SATURATION)
        return "sat ";
    // 'colr' = color
    if (compositeop == COMPOSITE_COLOR)
        return "colr";
    // 'lum ' = luminosity
    if (compositeop == COMPOSITE_LUMINIZE)
        return "lum ";

    dbgKrita << "Krita blending mode" << compositeop << "does not exist in Photoshop, returning Normal";
    return "norm";
}

QPoint psd_layer_effects_shadow_base::calculateOffset(const psd_layer_effects_context *context) const
{
    Q_UNUSED(context);

    qint32 angle = this->m_angle;

    qint32 distanceX = -qRound(this->m_distance * cos(kisDegreesToRadians(qreal(angle))));
    qint32 distanceY = qRound(this->m_distance * sin(kisDegreesToRadians(qreal(angle))));

    return QPoint(distanceX, distanceY);
}

// key functions
psd_layer_effects_shadow_base::~psd_layer_effects_shadow_base()
{
}

void psd_layer_effects_shadow_base::scaleLinearSizes(qreal scale)
{
    m_distance *= scale;
    m_size *= scale;
}

psd_layer_effects_shadow_common::~psd_layer_effects_shadow_common()
{
}

psd_layer_effects_drop_shadow::~psd_layer_effects_drop_shadow()
{
}

psd_layer_effects_glow_common::~psd_layer_effects_glow_common()
{
}

psd_layer_effects_inner_glow::~psd_layer_effects_inner_glow()
{
}

psd_layer_effects_outer_glow::~psd_layer_effects_outer_glow()
{
}

psd_layer_effects_inner_shadow::~psd_layer_effects_inner_shadow()
{
}
