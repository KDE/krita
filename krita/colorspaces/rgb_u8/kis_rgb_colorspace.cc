/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include <lcms.h>

#include <QImage>
#include <QColor>

#include <kdebug.h>
#include <klocale.h>

#include <KoU8ColorSpaceTrait.h>
#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>

#include "kis_rgb_colorspace.h"
#include "kis_color_conversions.h"

#include "kis_rgb_u8_compositeop.h"



#define downscale(quantum)  (quantum) //((unsigned char) ((quantum)/257UL))
#define upscale(value)  (value) // ((quint8) (257UL*(value)))

KisRgbColorSpace::KisRgbColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) :
    KoColorSpace("RGBA", i18n("RGB (8-bit integer/channel)"), parent)
    ,KoU8ColorSpaceTrait(PIXEL_ALPHA)
    ,KoLcmsColorSpaceTrait(TYPE_BGRA_8, icSigRgbData, p)
{
    m_channels.push_back(new KoChannelInfo(i18n("Red"), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(255,0,0)));
    m_channels.push_back(new KoChannelInfo(i18n("Green"), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0,255,0)));
    m_channels.push_back(new KoChannelInfo(i18n("Blue"), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0,0,255)));
    m_channels.push_back(new KoChannelInfo(i18n("Alpha"), 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT8));

    init();

    m_compositeOps.insert(COMPOSITE_OVER, new KisRgbU8CompositeOp(this, COMPOSITE_OVER,  i18n( "Normal" )) );
    m_compositeOps.insert(COMPOSITE_MULT, new KisRgbU8CompositeOp(this, COMPOSITE_MULT,  i18n( "Multiply" )) );
    m_compositeOps.insert(COMPOSITE_BURN, new KisRgbU8CompositeOp(this, COMPOSITE_BURN,  i18n( "Burn" )) );
    m_compositeOps.insert(COMPOSITE_DODGE, new KisRgbU8CompositeOp(this, COMPOSITE_DODGE,  i18n( "Dodge" )));
    m_compositeOps.insert(COMPOSITE_DIVIDE, new KisRgbU8CompositeOp(this, COMPOSITE_DIVIDE,  i18n( "Divide" )));
    m_compositeOps.insert(COMPOSITE_SCREEN, new KisRgbU8CompositeOp(this, COMPOSITE_SCREEN,  i18n( "Screen" )));
    m_compositeOps.insert(COMPOSITE_OVERLAY, new KisRgbU8CompositeOp(this, COMPOSITE_OVERLAY,  i18n( "Overlay" )));
    m_compositeOps.insert(COMPOSITE_DARKEN, new KisRgbU8CompositeOp(this, COMPOSITE_DARKEN,  i18n( "Darken" )));
    m_compositeOps.insert(COMPOSITE_LIGHTEN, new KisRgbU8CompositeOp(this, COMPOSITE_LIGHTEN,  i18n( "Lighten" )));
    m_compositeOps.insert(COMPOSITE_HUE, new KisRgbU8CompositeOp(this, COMPOSITE_HUE,  i18n( "Hue" )));
    m_compositeOps.insert(COMPOSITE_SATURATION, new KisRgbU8CompositeOp(this, COMPOSITE_SATURATION,  i18n( "Saturation" )));
    m_compositeOps.insert(COMPOSITE_VALUE, new KisRgbU8CompositeOp(this, COMPOSITE_VALUE,  i18n( "Value" )));
    m_compositeOps.insert(COMPOSITE_COLOR, new KisRgbU8CompositeOp(this, COMPOSITE_COLOR,  i18n( "Color" )));
}

KisRgbColorSpace::~KisRgbColorSpace()
{
}

void KisRgbColorSpace::mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
{
    quint32 totalRed = 0, totalGreen = 0, totalBlue = 0, totalAlpha = 0;

    while (nColors--)
    {
        quint32 alpha = (*colors)[PIXEL_ALPHA];
        // although we only mult by weight and not by weight*256/255
        // we divide by the same amount later, so there is no need
        quint32 alphaTimesWeight = alpha * *weights;

        totalRed += (*colors)[PIXEL_RED] * alphaTimesWeight;
        totalGreen += (*colors)[PIXEL_GREEN] * alphaTimesWeight;
        totalBlue += (*colors)[PIXEL_BLUE] * alphaTimesWeight;
        totalAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    // note this is correct - if you look at the above calculation
    if (totalAlpha > 255*255) totalAlpha = 255*255;

    // Divide by 255.
    dst[PIXEL_ALPHA] =(((totalAlpha + 0x80)>>8)+totalAlpha + 0x80) >>8;

    if (totalAlpha > 0) {
        totalRed = totalRed / totalAlpha;
        totalGreen = totalGreen / totalAlpha;
        totalBlue = totalBlue / totalAlpha;
    } // else the values are already 0 too

    quint32 dstRed = totalRed;
    //Q_ASSERT(dstRed <= 255);
    if (dstRed > 255) dstRed = 255;
    dst[PIXEL_RED] = dstRed;

    quint32 dstGreen = totalGreen;
    //Q_ASSERT(dstGreen <= 255);
    if (dstGreen > 255) dstGreen = 255;
    dst[PIXEL_GREEN] = dstGreen;

    quint32 dstBlue = totalBlue;
    //Q_ASSERT(dstBlue <= 255);
    if (dstBlue > 255) dstBlue = 255;
    dst[PIXEL_BLUE] = dstBlue;
}

void KisRgbColorSpace::convolveColors(quint8** colors, qint32* kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const
{
    qint32 totalRed = 0, totalGreen = 0, totalBlue = 0, totalAlpha = 0;

    while (nColors--)
    {
        qint32 weight = *kernelValues;

        if (weight != 0) {
            totalRed += (*colors)[PIXEL_RED] * weight;
            totalGreen += (*colors)[PIXEL_GREEN] * weight;
            totalBlue += (*colors)[PIXEL_BLUE] * weight;
            totalAlpha += (*colors)[PIXEL_ALPHA] * weight;
        }
        colors++;
        kernelValues++;
    }


    if (channelFlags & KoChannelInfo::FLAG_COLOR) {
        dst[PIXEL_RED] = CLAMP((totalRed / factor) + offset, 0, UINT8_MAX);
        dst[PIXEL_GREEN] = CLAMP((totalGreen / factor) + offset, 0, UINT8_MAX);
        dst[PIXEL_BLUE] =  CLAMP((totalBlue / factor) + offset, 0, UINT8_MAX);
    }
    if (channelFlags & KoChannelInfo::FLAG_ALPHA) {
        dst[PIXEL_ALPHA] = CLAMP((totalAlpha/ factor) + offset, 0, UINT8_MAX);
    }
}


void KisRgbColorSpace::invertColor(quint8 * src, qint32 nPixels)
{
    quint32 psize = pixelSize();

    while (nPixels--)
    {
        src[PIXEL_RED] = UINT8_MAX - src[PIXEL_RED];
        src[PIXEL_GREEN] = UINT8_MAX - src[PIXEL_GREEN];
        src[PIXEL_BLUE] = UINT8_MAX - src[PIXEL_BLUE];

        src += psize;
    }
}


void KisRgbColorSpace::darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const
{
    quint32 pSize = pixelSize();

    while (nPixels--) {
        if (compensate) {
            dst[PIXEL_RED]  = (qint8) qMin(255,(int)((src[PIXEL_RED] * shade) / (compensation * 255)));
            dst[PIXEL_GREEN]  = (qint8) qMin(255,(int)((src[PIXEL_GREEN] * shade) / (compensation * 255)));
            dst[PIXEL_BLUE]  = (qint8) qMin(255,(int)((src[PIXEL_BLUE] * shade) / (compensation * 255)));
        }
        else {
            dst[PIXEL_RED]  = (qint8) qMin(255, (src[PIXEL_RED] * shade / 255));
            dst[PIXEL_BLUE]  = (qint8) qMin(255, (src[PIXEL_BLUE] * shade / 255));
            dst[PIXEL_GREEN]  = (qint8) qMin(255, (src[PIXEL_GREEN] * shade / 255));
        }
        dst += pSize;
        src += pSize;
    }
}

quint8 KisRgbColorSpace::intensity8(const quint8 * src) const
{
    return (quint8)((src[PIXEL_RED] * 0.30 + src[PIXEL_GREEN] * 0.59 + src[PIXEL_BLUE] * 0.11) + 0.5);
}

Q3ValueVector<KoChannelInfo *> KisRgbColorSpace::channels() const
{
    return m_channels;
}

quint32 KisRgbColorSpace::nChannels() const
{
    return MAX_CHANNEL_RGBA;
}

quint32 KisRgbColorSpace::nColorChannels() const
{
    return MAX_CHANNEL_RGB;
}

quint32 KisRgbColorSpace::pixelSize() const
{
    return MAX_CHANNEL_RGBA;
}

QImage KisRgbColorSpace::convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                         KoColorProfile *  dstProfile,
                                         qint32 renderingIntent, float /*exposure*/)

{
    Q_ASSERT(data);
    QImage img = QImage(const_cast<quint8 *>(data), width, height, QImage::Format_ARGB32);
    // XXX: The previous version of this code used the quantum data directly
    // as an optimization. We're introducing a copy overhead here which could
    // be factored out again if needed.
    img = img.copy();

    if (dstProfile != 0) {
        KoColorSpace *dstCS = m_parent->colorSpace("RGBA",  dstProfile);
        convertPixelsTo(img.bits(),
                        img.bits(), dstCS,
                        width * height, renderingIntent);
    }

    return img;
}

