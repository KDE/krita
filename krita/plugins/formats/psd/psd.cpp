/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
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
#include "psd.h"

#include <KoColorModelStandardIds.h>
#include <KoCompositeOp.h>


QPair<QString, QString> psd_colormode_to_colormodelid(PSDColorMode colormode, quint16 channelDepth)
{
    QPair<QString, QString> colorSpaceId;
    switch(colormode) {
    case(Bitmap):
    case(Indexed):
    case(MultiChannel):
    case(RGB):
        colorSpaceId.first = RGBAColorModelID.id();
        break;
    case(CMYK):
        colorSpaceId.first = CMYKAColorModelID.id();
        break;
    case(Grayscale):
    case(DuoTone):
        colorSpaceId.first = GrayAColorModelID.id();
        break;
    case(Lab):
        colorSpaceId.first = LABAColorModelID.id();
        break;
    default:
        return colorSpaceId;
    }

    switch(channelDepth) {
    case(1):
    case(8):
        colorSpaceId.second =  Integer8BitsColorDepthID.id();
        break;
    case(16):
        colorSpaceId.second = Integer16BitsColorDepthID.id();
        break;
    case(32):
        colorSpaceId.second = Float32BitsColorDepthID.id();
        break;
    default:
        break;
    }
    return colorSpaceId;
}


QString psd_blendmode_to_composite_op(const QString& blendmode)
{
    if (blendmode == "norm") return COMPOSITE_OVER;    // normal
    if (blendmode == "diss") return COMPOSITE_DISSOLVE; //dissolve
    if (blendmode == "dark") return COMPOSITE_DARKEN;  // darken
    if (blendmode == "lite") return COMPOSITE_LIGHTEN; // lighten
    if (blendmode == "hue ") return COMPOSITE_HUE;     // hue
    if (blendmode == "sat ") return COMPOSITE_SATURATION; // saturation
    if (blendmode == "colr") return COMPOSITE_COLOR; //color
    if (blendmode == "lum ") return COMPOSITE_LUMINIZE; //luminosity
    if (blendmode == "mul ") return COMPOSITE_MULT; //multiply
    if (blendmode == "scrn") return COMPOSITE_SCREEN; //screen
    if (blendmode == "over") return COMPOSITE_OVERLAY; //overlay
    if (blendmode == "hLit") return COMPOSITE_HARD_LIGHT; //hard light
    if (blendmode == "sLit") return COMPOSITE_SOFT_LIGHT; //soft light
    if (blendmode == "diff") return COMPOSITE_DIFF; //difference
    if (blendmode == "smud") return COMPOSITE_EXCLUSION; //exclusion
    if (blendmode == "div ") return COMPOSITE_DIVIDE; // color dodge
    if (blendmode == "idiv") return COMPOSITE_INVERTED_DIVIDE ; //color burn
    if (blendmode == "lbrn") return COMPOSITE_BURN ; //linear burn
    if (blendmode == "lddg") return COMPOSITE_DODGE ; //linear dodge
    if (blendmode == "vLit") return COMPOSITE_VIVID_LIGHT; //vivid light
    if (blendmode == "lLit") return COMPOSITE_LINEAR_LIGHT; //linear light
    if (blendmode == "pLit") return COMPOSITE_PIN_LIGHT; //  pin light
    if (blendmode == "hMix") return COMPOSITE_HARD_MIX; //hard mix
    if (blendmode == "pass") return COMPOSITE_PASS_THROUGH; //pass through

    return COMPOSITE_UNDEF;
}

