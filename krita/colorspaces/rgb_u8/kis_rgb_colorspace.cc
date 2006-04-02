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

#include <qimage.h>
#include <qcolor.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_rgb_colorspace.h"
#include "kis_u8_base_colorspace.h"
#include "kis_color_conversions.h"
#include "kis_integer_maths.h"
#include "kis_colorspace_factory_registry.h"

#include "composite.h"

#define downscale(quantum)  (quantum) //((unsigned char) ((quantum)/257UL))
#define upscale(value)  (value) // ((quint8) (257UL*(value)))

namespace {
    const qint32 MAX_CHANNEL_RGB = 3;
    const qint32 MAX_CHANNEL_RGBA = 4;
}

KisRgbColorSpace::KisRgbColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) :
    KisU8BaseColorSpace(KisID("RGBA", i18n("RGB (8-bit integer/channel)")), TYPE_BGRA_8, icSigRgbData, parent, p)
{
    m_channels.push_back(new KisChannelInfo(i18n("Red"), 2, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, QColor(255,0,0)));
    m_channels.push_back(new KisChannelInfo(i18n("Green"), 1, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, QColor(0,255,0)));
    m_channels.push_back(new KisChannelInfo(i18n("Blue"), 0, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, QColor(0,0,255)));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), 3, KisChannelInfo::ALPHA, KisChannelInfo::UINT8));

    m_alphaPos = PIXEL_ALPHA;
    init();
}

KisRgbColorSpace::~KisRgbColorSpace()
{
}

void KisRgbColorSpace::setPixel(quint8 *pixel, quint8 red, quint8 green, quint8 blue, quint8 alpha) const
{
    pixel[PIXEL_RED] = Qt::red;
    pixel[PIXEL_GREEN] = Qt::green;
    pixel[PIXEL_BLUE] = Qt::blue;
    pixel[PIXEL_ALPHA] = alpha;
}

void KisRgbColorSpace::getPixel(const quint8 *pixel, quint8 *red, quint8 *green, quint8 *blue, quint8 *alpha) const
{
    *Qt::red = pixel[PIXEL_RED];
    *Qt::green = pixel[PIXEL_GREEN];
    *Qt::blue = pixel[PIXEL_BLUE];
    *alpha = pixel[PIXEL_ALPHA];
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

void KisRgbColorSpace::convolveColors(quint8** colors, qint32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const
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


    if (channelFlags & KisChannelInfo::FLAG_COLOR) {
        dst[PIXEL_RED] = CLAMP((totalRed / factor) + offset, 0, quint8_MAX);
        dst[PIXEL_GREEN] = CLAMP((totalGreen / factor) + offset, 0, quint8_MAX);
        dst[PIXEL_BLUE] =  CLAMP((totalBlue / factor) + offset, 0, quint8_MAX);
    }
    if (channelFlags & KisChannelInfo::FLAG_ALPHA) {
        dst[PIXEL_ALPHA] = CLAMP((totalAlpha/ factor) + offset, 0, quint8_MAX);
    }
}


void KisRgbColorSpace::invertColor(quint8 * src, qint32 nPixels)
{
    quint32 psize = pixelSize();

    while (nPixels--)
    {
        src[PIXEL_RED] = quint8_MAX - src[PIXEL_RED];
        src[PIXEL_GREEN] = quint8_MAX - src[PIXEL_GREEN];
        src[PIXEL_BLUE] = quint8_MAX - src[PIXEL_BLUE];

        src += psize;
    }
}


void KisRgbColorSpace::darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const
{
    quint32 pSize = pixelSize();

    while (nPixels--) {
        if (compensate) {
            dst[PIXEL_RED]  = (qint8) qMin(255,((src[PIXEL_RED] * shade) / (compensation * 255)));
            dst[PIXEL_GREEN]  = (qint8) qMin(255,((src[PIXEL_GREEN] * shade) / (compensation * 255)));
            dst[PIXEL_BLUE]  = (qint8) qMin(255,((src[PIXEL_BLUE] * shade) / (compensation * 255)));
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

Q3ValueVector<KisChannelInfo *> KisRgbColorSpace::channels() const
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
                                         KisProfile *  dstProfile,
                                         qint32 renderingIntent, float /*exposure*/)

{
    Q_ASSERT(data);
    QImage img = QImage(const_cast<quint8 *>(data), width, height, 32, 0, 0, QImage::LittleEndian);
    img.setAlphaBuffer(true);
    // XXX: The previous version of this code used the quantum data directly
    // as an optimisation. We're introducing a copy overhead here which could
    // be factored out again if needed.
    img = img.copy();

    if (dstProfile != 0) {
        KisColorSpace *dstCS = m_parent->getColorSpace(KisID("RGBA",""),  dstProfile->productName());
        convertPixelsTo(img.bits(),
                        img.bits(), dstCS,
                        width * height, renderingIntent);
    }

    return img;
}




void KisRgbColorSpace::compositeOver(quint8 *dstRowStart, qint32 dstRowStride,
                                     const quint8 *srcRowStart, qint32 srcRowStride,
                                     const quint8 *maskRowStart, qint32 maskRowStride,
                                     qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        const quint8 *mask = maskRowStart;
        qint32 columns = numColumns;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                if (srcAlpha == OPACITY_OPAQUE) {
                    memcpy(dst, src, MAX_CHANNEL_RGBA * sizeof(quint8));
                } else {
                    quint8 dstAlpha = dst[PIXEL_ALPHA];

                    quint8 srcBlend;

                    if (dstAlpha == OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dst[PIXEL_ALPHA] = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == OPACITY_OPAQUE) {
                        memcpy(dst, src, MAX_CHANNEL_RGB * sizeof(quint8));
                    } else {
                        dst[PIXEL_RED] = UINT8_BLEND(src[PIXEL_RED], dst[PIXEL_RED], srcBlend);
                        dst[PIXEL_GREEN] = UINT8_BLEND(src[PIXEL_GREEN], dst[PIXEL_GREEN], srcBlend);
                        dst[PIXEL_BLUE] = UINT8_BLEND(src[PIXEL_BLUE], dst[PIXEL_BLUE], srcBlend);
                    }
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeMultiply(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }


            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                quint8 srcColor = src[PIXEL_RED];
                quint8 dstColor = dst[PIXEL_RED];

                srcColor = UINT8_MULT(srcColor, dstColor);

                dst[PIXEL_RED] = UINT8_BLEND(srcColor, dstColor, srcBlend);

                srcColor = src[PIXEL_GREEN];
                dstColor = dst[PIXEL_GREEN];

                srcColor = UINT8_MULT(srcColor, dstColor);

                dst[PIXEL_GREEN] = UINT8_BLEND(srcColor, dstColor, srcBlend);

                srcColor = src[PIXEL_BLUE];
                dstColor = dst[PIXEL_BLUE];

                srcColor = UINT8_MULT(srcColor, dstColor);

                dst[PIXEL_BLUE] = UINT8_BLEND(srcColor, dstColor, srcBlend);
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeDivide(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin((dstColor * (UINT8_MAX + 1u) + (srcColor / 2u)) / (1u + srcColor), UINT8_MAX);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeScreen(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = UINT8_MAX - UINT8_MULT(UINT8_MAX - dstColor, UINT8_MAX - srcColor);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeOverlay(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }


            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = UINT8_MULT(dstColor, dstColor + UINT8_MULT(2 * srcColor, UINT8_MAX - dstColor));

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeDodge(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }


            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin((dstColor * (UINT8_MAX + 1)) / (UINT8_MAX + 1 - srcColor), UINT8_MAX);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeBurn(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin(((UINT8_MAX - dstColor) * (UINT8_MAX + 1)) / (srcColor + 1), UINT8_MAX);
                    if (UINT8_MAX - srcColor > UINT8_MAX) srcColor = UINT8_MAX;

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeDarken(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin(srcColor, dstColor);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeLighten(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMax(srcColor, dstColor);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeHue(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[PIXEL_RED];
                int dstGreen = dst[PIXEL_GREEN];
                int dstBlue = dst[PIXEL_BLUE];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(srcHue, dstSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

                dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeSaturation(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[PIXEL_RED];
                int dstGreen = dst[PIXEL_GREEN];
                int dstBlue = dst[PIXEL_BLUE];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(dstHue, srcSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

                dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeValue(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[PIXEL_RED];
                int dstGreen = dst[PIXEL_GREEN];
                int dstBlue = dst[PIXEL_BLUE];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(dstHue, dstSaturation, srcValue, &srcRed, &srcGreen, &srcBlue);

                dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeColor(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[PIXEL_RED];
                int dstGreen = dst[PIXEL_GREEN];
                int dstBlue = dst[PIXEL_BLUE];

                int srcHue;
                int srcSaturation;
                int srcLightness;
                int dstHue;
                int dstSaturation;
                int dstLightness;

                rgb_to_hls(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcLightness, &srcSaturation);
                rgb_to_hls(dstRed, dstGreen, dstBlue, &dstHue, &dstLightness, &dstSaturation);

                quint8 srcRed;
                quint8 srcGreen;
                quint8 srcBlue;

                hls_to_rgb(srcHue, dstLightness, srcSaturation, &srcRed, &srcGreen, &srcBlue);

                dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbColorSpace::compositeErase(quint8 *dst,
            qint32 dstRowSize,
            const quint8 *src,
            qint32 srcRowSize,
            const quint8 *srcAlphaMask,
            qint32 maskRowStride,
            qint32 rows,
            qint32 cols,
            quint8 /*opacity*/)
{
    qint32 i;
    quint8 srcAlpha;

    while (rows-- > 0)
    {
        const quint8 *s = src;
        quint8 *d = dst;
        const quint8 *mask = srcAlphaMask;

        for (i = cols; i > 0; i--, s+=MAX_CHANNEL_RGBA, d+=MAX_CHANNEL_RGBA)
        {
            srcAlpha = s[PIXEL_ALPHA];
            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_BLEND(srcAlpha, OPACITY_OPAQUE, *mask);
                mask++;
            }
            d[PIXEL_ALPHA] = UINT8_MULT(srcAlpha, d[PIXEL_ALPHA]);
        }

        dst += dstRowSize;
        if(srcAlphaMask)
            srcAlphaMask += maskRowStride;
        src += srcRowSize;
    }
}

void KisRgbColorSpace::bitBlt(quint8 *dst,
                      qint32 dstRowStride,
                      const quint8 *src,
                      qint32 srcRowStride,
                      const quint8 *mask,
                      qint32 maskRowStride,
                      quint8 opacity,
                      qint32 rows,
                      qint32 cols,
                      const KisCompositeOp& op)
{

    switch (op.op()) {
    case COMPOSITE_UNDEF:
        // Undefined == no composition
        break;
    case COMPOSITE_OVER:
        compositeOver(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_IN:
        compositeIn(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    case COMPOSITE_OUT:
        compositeOut(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ATOP:
        compositeAtop(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_XOR:
        compositeXor(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_PLUS:
        compositePlus(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_MINUS:
        compositeMinus(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ADD:
        compositeAdd(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SUBTRACT:
        compositeSubtract(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DIFF:
        compositeDiff(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_MULT:
        compositeMultiply(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DIVIDE:
        compositeDivide(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_BUMPMAP:
        compositeBumpmap(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY:
        compositeCopy(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_RED:
        compositeCopyRed(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_GREEN:
        compositeCopyGreen(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_BLUE:
        compositeCopyBlue(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_OPACITY:
        compositeCopyOpacity(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_CLEAR:
        compositeClear(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DISSOLVE:
        compositeDissolve(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DISPLACE:
        compositeDisplace(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
#if 0
    case COMPOSITE_MODULATE:
        compositeModulate(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_THRESHOLD:
        compositeThreshold(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
#endif
    case COMPOSITE_NO:
        // No composition.
        break;
    case COMPOSITE_DARKEN:
        compositeDarken(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_LIGHTEN:
        compositeLighten(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_HUE:
        compositeHue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SATURATION:
        compositeSaturation(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_VALUE:
        compositeValue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COLOR:
        compositeColor(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COLORIZE:
        compositeColorize(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_LUMINIZE:
        compositeLuminize(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SCREEN:
        compositeScreen(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_OVERLAY:
        compositeOverlay(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ERASE:
        compositeErase(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DODGE:
        compositeDodge(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_BURN:
        compositeBurn(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    default:
        break;
    }
}

KisCompositeOpList KisRgbColorSpace::userVisiblecompositeOps() const
{
    KisCompositeOpList list;

    list.append(KisCompositeOp(COMPOSITE_OVER));
    list.append(KisCompositeOp(COMPOSITE_MULT));
    list.append(KisCompositeOp(COMPOSITE_BURN));
    list.append(KisCompositeOp(COMPOSITE_DODGE));
    list.append(KisCompositeOp(COMPOSITE_DIVIDE));
    list.append(KisCompositeOp(COMPOSITE_SCREEN));
    list.append(KisCompositeOp(COMPOSITE_OVERLAY));
    list.append(KisCompositeOp(COMPOSITE_DARKEN));
    list.append(KisCompositeOp(COMPOSITE_LIGHTEN));
    list.append(KisCompositeOp(COMPOSITE_HUE));
    list.append(KisCompositeOp(COMPOSITE_SATURATION));
    list.append(KisCompositeOp(COMPOSITE_VALUE));
    list.append(KisCompositeOp(COMPOSITE_COLOR));

    return list;
}
