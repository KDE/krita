/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#include <limits.h>
#include <stdlib.h>

#include <config.h>
#include LCMS_HEADER

#include <qimage.h>

#include <klocale.h>
#include <kdebug.h>

#include "kis_color_conversions.h"
#include "kis_abstract_colorspace.h"
#include "kis_colorspace_registry.h"
#include "kis_image.h"
#include "kis_wet_sticky_colorspace.h"
#include "kis_integer_maths.h"
#include "kis_types.h"
#include "kis_channelinfo.h"

#define NOWSDEBUG

using namespace WetAndSticky;

enum WetStickyChannelIndex {
    BLUE_CHANNEL_INDEX,
    GREEN_CHANNEL_INDEX,
    RED_CHANNEL_INDEX,
    ALPHA_CHANNEL_INDEX,
    HUE_CHANNEL_INDEX,
    SATURATION_CHANNEL_INDEX,
    LIGHTNESS_CHANNEL_INDEX,
    LIQUID_CONTENT_CHANNEL_INDEX,
    DRYING_RATE_CHANNEL_INDEX,
    MISCIBILITY_CHANNEL_INDEX,
    GRAVITATIONAL_DIRECTION_INDEX,
    GRAVITATIONAL_STRENGTH_CHANNEL_INDEX,
    ABSORBANCY_CHANNEL_INDEX,
    PAINT_VOLUME_CHANNEL_INDEX
};

KisWetStickyColorSpace::KisWetStickyColorSpace() :
    KisAbstractColorSpace(KisID("W&S", i18n("Wet & Sticky")), 0, icMaxEnumData)
{
    Q_INT32 pos = 0;

    // Basic representational definition
    m_channels.push_back(new KisChannelInfo(i18n("Blue"), "B", pos, COLOR, 1));
    m_channels.push_back(new KisChannelInfo(i18n("Green"), "G", ++pos, COLOR, 1));
    m_channels.push_back(new KisChannelInfo(i18n("Red"), "R", ++pos, COLOR, 1));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), "A", ++pos, ALPHA, 1));

    // Paint definition
    m_channels.push_back(new KisChannelInfo(i18n("Hue"), "H", ++pos, COLOR, sizeof(float)));
    m_channels.push_back(new KisChannelInfo(i18n("Saturation"), "S", pos+=sizeof(float) , COLOR, sizeof(float)));
    m_channels.push_back(new KisChannelInfo(i18n("Lightness"), "L", pos+=sizeof(float), COLOR, sizeof(float)));

    m_channels.push_back(new KisChannelInfo(i18n("Liquid Content"), "Q", pos+=sizeof(float), SUBSTANCE, 1));
    m_channels.push_back(new KisChannelInfo(i18n("Drying Rate"), "D", ++pos, SUBSTANCE, 1));
    m_channels.push_back(new KisChannelInfo(i18n("Miscibility"), "M", ++pos, SUBSTANCE, 1));

    // Substrate definition
    m_channels.push_back(new KisChannelInfo(i18n("Gravitational Direction"), "Gd", ++pos, SUBSTRATE, sizeof(enumDirection)));
    m_channels.push_back(new KisChannelInfo(i18n("Gravitational Strength"), "Gs", pos+=sizeof(enumDirection), SUBSTRATE, 1));

    m_channels.push_back(new KisChannelInfo(i18n("Absorbency"), "Ad", ++pos, SUBSTRATE, 1));
    m_channels.push_back(new KisChannelInfo(i18n("Paint Volume"), "V", ++pos, SUBSTANCE, 1));

    m_alphaPos = 3;
    m_alphaSize = 1;
    setDefaultProfile( 0 );

#ifdef WSDEBUG
    QValueVector<KisChannelInfo *>_it it;
    int i = 0;
    for (it = m_channels.begin(); it != m_channels.end(); ++it)
    {
        KisChannelInfo * ch = (*it);
        kdDebug(DBG_AREA_CMS) << "Channel: " << ch->name() << ", " << ch->pos() << ", " << i << "\n";
        ++i;
    }

    kdDebug(DBG_AREA_CMS) << "Size of cell: " << sizeof(CELL) << "\n";
#endif
}


KisWetStickyColorSpace::~KisWetStickyColorSpace()
{
}

void KisWetStickyColorSpace::fromQColor(const QColor& c, Q_UINT8 *dst, KisProfile *  profile)
{
    CELL_PTR p = (CELL_PTR) dst;
    Q_UINT8 r, g, b;

    r = c.red();
    g = c.green();
    b = c.blue();

    p -> red = r;
    p -> green = g;
    p -> blue = b;
    p -> alpha = OPACITY_OPAQUE;

    rgb_to_hls(r, g, b, &p->hue, &p->lightness, &p->saturation);

    p -> liquid_content = 0;
    p -> drying_rate = 0;
    p -> miscibility = 0;

    p -> direction = DOWN;
    p -> strength = 10;

    p -> absorbancy = 10;
    p -> volume = 0;

#ifdef WSDEBUG
    kdDebug(DBG_AREA_CMS) << "qcolor: "
        << " r: " << c.red() << " b: " << c.blue() << " g: " << c.red()
        << " native color: (" << QString().setNum(p->red) << ", "
                              << QString().setNum(p->green) << ", "
                              << QString().setNum(p->blue) << ", "
                              << QString().setNum(p->alpha) << ") "
        << ", hls: (" << p->hue << ", "
                      << p->lightness << ", "
                      << p->saturation << ")\n";
#endif
}

void KisWetStickyColorSpace::fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst, KisProfile *  profile)
{
    CELL_PTR p = (CELL_PTR) dst;
    Q_UINT8 r, g, b;

    r = c.red();
    g = c.green();
    b = c.blue();

    p -> red = r;
    p -> green = g;
    p -> blue = b;
    p -> alpha = opacity;
    rgb_to_hls(r, g, b, &p -> hue, &p -> lightness, &p -> saturation);

    p ->liquid_content = 0;
    p ->drying_rate = 0;
    p ->miscibility = 0;

    p -> direction = DOWN;
    p -> strength = 10;

    p -> absorbancy = 10;
    p -> volume = 0;

#ifdef WSDEBUG
    kdDebug(DBG_AREA_CMS) << "qcolor: "
        << " r: " << c.red() << " b: " << c.blue() << " g: " << c.red() << " opacity: " << opacity
        << " native color: (" << QString().setNum(p->red) << ", "
                              << QString().setNum(p->green) << ", "
                              << QString().setNum(p->blue) << ", "
                              << QString().setNum(p->alpha) << ") "
        << ", hls: (" << p->hue << ", "
                      << p->lightness << ", "
                      << p->saturation << ")\n";
#endif
}

void KisWetStickyColorSpace::toQColor(const Q_UINT8 *src, QColor *c, KisProfile *  profile)
{
    CELL_PTR p = (CELL_PTR) src;

    c -> setRgb(p -> red,
            p -> green,
            p -> blue);
#ifdef WSDEBUG
    kdDebug(DBG_AREA_CMS) << "Created qcolor from wet & sticky: " << " r: " << c->red() << " b: " << c->blue() << " g: " << c->red() << "\n";
#endif
}

void KisWetStickyColorSpace::toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity, KisProfile *  profile)
{

    CELL_PTR p = (CELL_PTR) src;

    c -> setRgb(p -> red,
            p -> green,
            p -> blue);

    *opacity = p -> alpha;
#ifdef WSDEBUG
    kdDebug(DBG_AREA_CMS) << "Created qcolor from wet & sticky: " << " r: " << c->red() << " b: " << c->blue() << " g: " << c->red() << "\n";
#endif
}



KisPixelRO KisWetStickyColorSpace::toKisPixelRO(const Q_UINT8 *src, KisProfile *  profile)
{
    return KisPixelRO (src, src, this, profile);
}

KisPixel KisWetStickyColorSpace::toKisPixel(Q_UINT8 *src, KisProfile *  profile)
{
    return KisPixel (src, src, this, profile);
}

void KisWetStickyColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
}

Q_UINT8 KisWetStickyColorSpace::getAlpha(const Q_UINT8 *pixel) const
{
    return ((CELL_PTR)pixel)->alpha;
}

void KisWetStickyColorSpace::setAlpha(Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels) const
{
    while (nPixels > 0) {
        ((CELL_PTR)pixels)->alpha = alpha;
        --nPixels;
        pixels+=pixelSize();
    }
}

void KisWetStickyColorSpace::applyAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels)
{
}

void KisWetStickyColorSpace::applyInverseAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels)
{
}

Q_UINT8 KisWetStickyColorSpace::scaleToU8(const Q_UINT8 * srcPixel, Q_INT32 channelPos)
{
    return 0;
}

Q_UINT16 KisWetStickyColorSpace::scaleToU16(const Q_UINT8 * srcPixel, Q_INT32 channelPos)
{
    return 0;
}


QValueVector<KisChannelInfo *> KisWetStickyColorSpace::channels() const
{
    return m_channels;
}

bool KisWetStickyColorSpace::hasAlpha() const
{
    return true;
}

Q_INT32 KisWetStickyColorSpace::nChannels() const
{
    return 14;
}

Q_INT32 KisWetStickyColorSpace::nColorChannels() const
{
    return 3;
}

Q_INT32 KisWetStickyColorSpace::nSubstanceChannels() const
{
    return 4;

}

Q_INT32 KisWetStickyColorSpace::pixelSize() const
{
    return sizeof(CELL);
}


QImage KisWetStickyColorSpace::convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                           KisProfile *  /*srcProfile*/, KisProfile *  /*dstProfile*/,
                           Q_INT32 /*renderingIntent*/, float /*exposure*/)
{

    QImage img(width, height, 32, 0, QImage::LittleEndian);

    Q_INT32 i = 0;
    uchar *j = img.bits();

    CELL_PTR p = (CELL_PTR) data;

    while ( i < width * height) {

        const Q_UINT8 PIXEL_BLUE = 0;
        const Q_UINT8 PIXEL_GREEN = 1;
        const Q_UINT8 PIXEL_RED = 2;
        const Q_UINT8 PIXEL_ALPHA = 3;

        *( j + PIXEL_ALPHA ) = p -> alpha;
        *( j + PIXEL_RED )   = p -> red;
        *( j + PIXEL_GREEN ) = p -> green;
        *( j + PIXEL_BLUE )  = p -> blue;

        p++;
        i++;
        j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
    }
    return img;
}

bool KisWetStickyColorSpace::convertPixelsTo(const Q_UINT8 * src, KisProfile *  /*srcProfile*/,
                         Q_UINT8 * dst, KisAbstractColorSpace * dstColorSpace, KisProfile *  dstProfile,
                         Q_UINT32 numPixels,
                         Q_INT32 /*renderingIntent*/)
{
    Q_INT32 dSize = dstColorSpace -> pixelSize();
    Q_INT32 sSize = pixelSize();

    Q_UINT32 j = 0;
    Q_UINT32 i = 0;
    QColor c;
    CELL_PTR cp;
    while ( i < numPixels ) {
        cp = (CELL_PTR) (src + i);

        c.setRgb(cp -> red,
             cp -> green,
             cp -> blue);

        dstColorSpace -> fromQColor(c, cp -> alpha, (dst + j), dstProfile);

        i += sSize;
        j += dSize;

    }
    return true;

}

void KisWetStickyColorSpace::bitBlt(Q_UINT8 *dst,
                      Q_INT32 dstRowStride,
                      const Q_UINT8 *src,
                      Q_INT32 srcRowStride,
                      const Q_UINT8 *mask,
                      Q_INT32 maskRowStride,
                      Q_UINT8 opacity,
                      Q_INT32 rows,
                      Q_INT32 cols,
                      const KisCompositeOp& op)
{
    switch (op.op()) {
    case COMPOSITE_UNDEF:
        // Undefined == no composition
        break;
    case COMPOSITE_OVER:
        compositeOver(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY:
    default:
        compositeCopy(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    }

}


void KisWetStickyColorSpace::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    // XXX: This is basically the same as with rgb and used to composite layers for  Composition for
    //      painting works differently


    while (rows > 0) {

        const Q_UINT8 *src = srcRowStart;
        Q_UINT8 *dst = dstRowStart;
        const Q_UINT8 *mask = maskRowStart;

        Q_INT32 columns = numColumns;

        while (columns > 0) {

            CELL_PTR dstCell = (CELL_PTR) dst;
            CELL_PTR srcCell = (CELL_PTR) src;

#ifdef WSDEBUG
            kdDebug(DBG_AREA_CMS) << "Source: " << rows << ", " << columns << " color: " <<
                srcCell->red << ", " << srcCell->blue << ", " << srcCell->green << ", " << srcCell->alpha << ", " << srcCell->volume << "\n";


            kdDebug(DBG_AREA_CMS) << "Destination: "  << rows << ", " << columns << " color: " <<
                dstCell->red << ", " << dstCell->blue << ", " << dstCell->green << ", " << dstCell->alpha << ", " << dstCell->volume << "\n";

#endif

            Q_UINT8 srcAlpha = srcCell->alpha;

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(srcCell->alpha, opacity);
                }

                if (srcAlpha == OPACITY_OPAQUE) {
                    memcpy(dst, src, 3); // XXX: First three bytes for rgb?
                } else {
                    Q_UINT8 dstAlpha = dstCell->alpha;

                    Q_UINT8 srcBlend;

                    if (dstAlpha == OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dstCell->alpha = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == OPACITY_OPAQUE) {
                        memcpy(dst, src, 3); //XXX: First three bytes for rgb?
                    } else {
                        dstCell->red = UINT8_BLEND(srcCell->red, dstCell->red, srcBlend);
                        dstCell->green = UINT8_BLEND(srcCell->green, dstCell->green, srcBlend);
                        dstCell->blue = UINT8_BLEND(srcCell->blue, dstCell->blue, srcBlend);
                    }
                }
            }
            columns--;
            src += sizeof(CELL);
            dst += sizeof(CELL);
        }
        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;

        if(maskRowStart)
            maskRowStart += maskRowStride;
    }

}

void KisWetStickyColorSpace::compositeCopy(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, Q_UINT8 opacity)
{
    Q_INT32 linesize = sizeof(CELL) * columns;
    Q_UINT8 *d;
    const Q_UINT8 *s;
    d = dst;
    s = src;

    while (rows-- > 0) {
        memcpy(d, s, linesize);
        d += dstRowStride;
        s += srcRowStride;
    }

}


KisCompositeOpList KisWetStickyColorSpace::userVisiblecompositeOps() const
{
    KisCompositeOpList list;

    list.append(KisCompositeOp(COMPOSITE_OVER));

    return list;
}

QString KisWetStickyColorSpace::channelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    const CELL *pixel = reinterpret_cast<const CELL *>(U8_pixel);

    switch (channelIndex) {
    case BLUE_CHANNEL_INDEX:
        return QString().setNum(pixel -> blue);
    case GREEN_CHANNEL_INDEX:
        return QString().setNum(pixel -> green);
    case RED_CHANNEL_INDEX:
        return QString().setNum(pixel -> red);
    case ALPHA_CHANNEL_INDEX:
        return QString().setNum(pixel -> alpha);
    case HUE_CHANNEL_INDEX:
        return QString().setNum(pixel -> hue);
    case SATURATION_CHANNEL_INDEX:
        return QString().setNum(pixel -> saturation);
    case LIGHTNESS_CHANNEL_INDEX:
        return QString().setNum(pixel -> lightness);
    case LIQUID_CONTENT_CHANNEL_INDEX:
        return QString().setNum(pixel -> liquid_content);
    case DRYING_RATE_CHANNEL_INDEX:
        return QString().setNum(pixel -> drying_rate);
    case MISCIBILITY_CHANNEL_INDEX:
        return QString().setNum(pixel -> miscibility);
    case GRAVITATIONAL_DIRECTION_INDEX:
        {
            switch (pixel -> direction) {
            case UP:
                return i18n("Up");
            case DOWN:
                return i18n("Down");
            case LEFT:
                return i18n("Left");
            case RIGHT:
                return i18n("Right");
            default:
                Q_ASSERT(false);
                return QString();
            }
        }
    case GRAVITATIONAL_STRENGTH_CHANNEL_INDEX:
        return QString().setNum(pixel -> strength);
    case ABSORBANCY_CHANNEL_INDEX:
        return QString().setNum(pixel -> absorbancy);
    case PAINT_VOLUME_CHANNEL_INDEX:
        return QString().setNum(pixel -> volume);
    default:
        Q_ASSERT(false);
        return QString();
    }
}

QString KisWetStickyColorSpace::normalisedChannelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    const CELL *pixel = reinterpret_cast<const CELL *>(U8_pixel);

    //XXX: Are these right?

    switch (channelIndex) {
    case BLUE_CHANNEL_INDEX:
        return QString().setNum(static_cast<float>(pixel -> blue) / UINT8_MAX);
    case GREEN_CHANNEL_INDEX:
        return QString().setNum(static_cast<float>(pixel -> green) / UINT8_MAX);
    case RED_CHANNEL_INDEX:
        return QString().setNum(static_cast<float>(pixel -> red) / UINT8_MAX);
    case ALPHA_CHANNEL_INDEX:
        return QString().setNum(static_cast<float>(pixel -> alpha) / UINT8_MAX);
    case HUE_CHANNEL_INDEX:
        return QString().setNum(pixel -> hue);
    case SATURATION_CHANNEL_INDEX:
        return QString().setNum(pixel -> saturation);
    case LIGHTNESS_CHANNEL_INDEX:
        return QString().setNum(pixel -> lightness);
    case LIQUID_CONTENT_CHANNEL_INDEX:
        return QString().setNum(static_cast<float>(pixel -> liquid_content) / UINT8_MAX);
    case DRYING_RATE_CHANNEL_INDEX:
        return QString().setNum(static_cast<float>(pixel -> drying_rate) / UINT8_MAX);
    case MISCIBILITY_CHANNEL_INDEX:
        return QString().setNum(static_cast<float>(pixel -> miscibility) / UINT8_MAX);
    case GRAVITATIONAL_DIRECTION_INDEX:
        {
            switch (pixel -> direction) {
            case UP:
                return i18n("Up");
            case DOWN:
                return i18n("Down");
            case LEFT:
                return i18n("Left");
            case RIGHT:
                return i18n("Right");
            default:
                Q_ASSERT(false);
                return QString();
            }
        }
    case GRAVITATIONAL_STRENGTH_CHANNEL_INDEX:
        return QString().setNum(static_cast<float>(pixel -> strength) / UINT8_MAX);
    case ABSORBANCY_CHANNEL_INDEX:
        return QString().setNum(static_cast<float>(pixel -> absorbancy) / UINT8_MAX);
    case PAINT_VOLUME_CHANNEL_INDEX:
        return QString().setNum(static_cast<float>(pixel -> volume) / UINT8_MAX);
    default:
        Q_ASSERT(false);
        return QString();
    }
}

