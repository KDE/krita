/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include <lcms.h>

#include <QImage>

#include <kdebug.h>
#include <klocale.h>

#include "kis_rgb_f32_colorspace.h"
#include "kis_color_conversions.h"

namespace {
    const qint32 MAX_CHANNEL_RGB = 3;
    const qint32 MAX_CHANNEL_RGBA = 4;
}

#include "KoIntegerMaths.h"

#ifndef HAVE_POWF
#undef powf
#define powf pow
#endif

#define FLOAT_MAX 1.0f //temp

#define EPSILON 1e-6

// FIXME: lcms doesn't support 32-bit float
#define F32_LCMS_TYPE TYPE_BGRA_16

// disable the lcms handling by setting profile=0
KisRgbF32ColorSpace::KisRgbF32ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile */*p*/) :
    KoColorSpace("RGBAF32", i18n("RGB (32-bit float/channel)"), parent)
    , KoF32ColorSpaceTrait(PIXEL_ALPHA * sizeof(float))
    , KoLcmsColorSpaceTrait(F32_LCMS_TYPE, icSigRgbData, 0)
{
    m_channels.push_back(new KoChannelInfo(i18n("Red"), PIXEL_RED * sizeof(float), KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float)));
    m_channels.push_back(new KoChannelInfo(i18n("Green"), PIXEL_GREEN * sizeof(float), KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float)));
    m_channels.push_back(new KoChannelInfo(i18n("Blue"), PIXEL_BLUE * sizeof(float), KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float)));
    m_channels.push_back(new KoChannelInfo(i18n("Alpha"), PIXEL_ALPHA * sizeof(float), KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32, sizeof(float)));
}

KisRgbF32ColorSpace::~KisRgbF32ColorSpace()
{
}

void KisRgbF32ColorSpace::setPixel(quint8 *dst, float red, float green, float blue, float alpha) const
{
    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->red = red;
    dstPixel->green = green;
    dstPixel->blue = blue;
    dstPixel->alpha = alpha;
}

void KisRgbF32ColorSpace::getPixel(const quint8 *src, float *red, float *green, float *blue, float *alpha) const
{
    const Pixel *srcPixel = reinterpret_cast<const Pixel *>(src);

    *red = srcPixel->red;
    *green = srcPixel->green;
    *blue = srcPixel->blue;
    *alpha = srcPixel->alpha;
}

void KisRgbF32ColorSpace::fromQColor(const QColor& c, quint8 *dstU8, KoColorProfile * /*profile*/)
{
    Pixel *dst = reinterpret_cast<Pixel *>(dstU8);

    dst->red = UINT8_TO_FLOAT(c.red());
    dst->green = UINT8_TO_FLOAT(c.green());
    dst->blue = UINT8_TO_FLOAT(c.blue());
}

void KisRgbF32ColorSpace::fromQColor(const QColor& c, quint8 opacity, quint8 *dstU8, KoColorProfile * /*profile*/)
{
    Pixel *dst = reinterpret_cast<Pixel *>(dstU8);

    dst->red = UINT8_TO_FLOAT(c.red());
    dst->green = UINT8_TO_FLOAT(c.green());
    dst->blue = UINT8_TO_FLOAT(c.blue());
    dst->alpha = UINT8_TO_FLOAT(opacity);
}

void KisRgbF32ColorSpace::toQColor(const quint8 *srcU8, QColor *c, KoColorProfile * /*profile*/)
{
    const Pixel *src = reinterpret_cast<const Pixel *>(srcU8);

    c->setRgb(FLOAT_TO_UINT8(src->red), FLOAT_TO_UINT8(src->green), FLOAT_TO_UINT8(src->blue));
}

void KisRgbF32ColorSpace::toQColor(const quint8 *srcU8, QColor *c, quint8 *opacity, KoColorProfile * /*profile*/)
{
    const Pixel *src = reinterpret_cast<const Pixel *>(srcU8);

    c->setRgb(FLOAT_TO_UINT8(src->red), FLOAT_TO_UINT8(src->green), FLOAT_TO_UINT8(src->blue));
    *opacity = FLOAT_TO_UINT8(src->alpha);
}

quint8 KisRgbF32ColorSpace::difference(const quint8 *src1U8, const quint8 *src2U8)
{
    const Pixel *src1 = reinterpret_cast<const Pixel *>(src1U8);
    const Pixel *src2 = reinterpret_cast<const Pixel *>(src2U8);

    return FLOAT_TO_UINT8(qMax(QABS(src2->red - src1->red),
                qMax(QABS(src2->green - src1->green),
                     QABS(src2->blue - src1->blue))));
}

void KisRgbF32ColorSpace::mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
{
    float totalRed = 0, totalGreen = 0, totalBlue = 0, newAlpha = 0;

    while (nColors--)
    {
        const Pixel *pixel = reinterpret_cast<const Pixel *>(*colors);

        float alpha = pixel->alpha;
        float alphaTimesWeight = alpha * UINT8_TO_FLOAT(*weights);

        totalRed += pixel->red * alphaTimesWeight;
        totalGreen += pixel->green * alphaTimesWeight;
        totalBlue += pixel->blue * alphaTimesWeight;
        newAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    Q_ASSERT(newAlpha <= F32_OPACITY_OPAQUE);

    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->alpha = newAlpha;

    if (newAlpha > EPSILON) {
        totalRed = totalRed / newAlpha;
        totalGreen = totalGreen / newAlpha;
        totalBlue = totalBlue / newAlpha;
    }

    dstPixel->red = totalRed;
    dstPixel->green = totalGreen;
    dstPixel->blue = totalBlue;
}

void KisRgbF32ColorSpace::convolveColors(quint8** colors, qint32 * kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const
{
    float totalRed = 0, totalGreen = 0, totalBlue = 0, totalAlpha = 0;

    while (nColors--)
    {
        const Pixel * pixel = reinterpret_cast<const Pixel *>( *colors );

        float weight = *kernelValues;

        if (weight != 0) {
            totalRed += pixel->red * weight;
            totalGreen += pixel->green * weight;
            totalBlue += pixel->blue * weight;
            totalAlpha += pixel->alpha * weight;
        }
        colors++;
        kernelValues++;
    }

    Pixel * p = reinterpret_cast< Pixel *>( dst );

    if (channelFlags & KoChannelInfo::FLAG_COLOR) {
        p->red = CLAMP( ( totalRed / factor) + offset, 0, FLOAT_MAX);
        p->green = CLAMP( ( totalGreen / factor) + offset, 0, FLOAT_MAX);
        p->blue = CLAMP( ( totalBlue / factor) + offset, 0, FLOAT_MAX);
    }
    if (channelFlags & KoChannelInfo::FLAG_ALPHA) {
        p->alpha = CLAMP((totalAlpha/ factor) + offset, 0, FLOAT_MAX);
    }
}


void KisRgbF32ColorSpace::invertColor(quint8 * src, qint32 nPixels)
{
    quint32 psize = pixelSize();

    while (nPixels--)
    {
        Pixel * p = reinterpret_cast< Pixel *>( src );
        p->red = FLOAT_MAX - p->red;
        p->green = FLOAT_MAX - p->green;
        p->blue = FLOAT_MAX - p->blue;
        src += psize;
    }

}

quint8 KisRgbF32ColorSpace::intensity8(const quint8 * src) const
{
    const Pixel * p = reinterpret_cast<const Pixel *>( src );

    return FLOAT_TO_UINT8((p->red * 0.30 + p->green * 0.59 + p->blue * 0.11) + 0.5);
}



Q3ValueVector<KoChannelInfo *> KisRgbF32ColorSpace::channels() const
{
    return m_channels;
}

quint32 KisRgbF32ColorSpace::nChannels() const
{
    return MAX_CHANNEL_RGBA;
}

quint32 KisRgbF32ColorSpace::nColorChannels() const
{
    return MAX_CHANNEL_RGB;
}

quint32 KisRgbF32ColorSpace::pixelSize() const
{
    return MAX_CHANNEL_RGBA * sizeof(float);
}

quint8 convertToDisplay(float value, float exposureFactor, float gamma)
{
    //value *= pow(2, exposure + 2.47393);
    value *= exposureFactor;

    value = powf(value, gamma);

    // scale middle gray to the target framebuffer value

    value *= 84.66f;

    int valueInt = (int)(value + 0.5);

    return CLAMP(valueInt, 0, 255);
}

QImage KisRgbF32ColorSpace::convertToQImage(const quint8 *dataU8, qint32 width, qint32 height,
                                            KoColorProfile *  /*dstProfile*/,
                                            qint32 /*renderingIntent*/, float exposure)

{
    const float *data = reinterpret_cast<const float *>(dataU8);

    QImage img = QImage(width, height, QImage::Format_ARGB32);

    qint32 i = 0;
    uchar *j = img.bits();

    // XXX: For now assume gamma 2.2.
    float gamma = 1 / 2.2;
    float exposureFactor = powf(2, exposure + 2.47393);

    while ( i < width * height * MAX_CHANNEL_RGBA) {
        *( j + 3)  = FLOAT_TO_UINT8(*( data + i + PIXEL_ALPHA ));
        *( j + 2 ) = convertToDisplay(*( data + i + PIXEL_RED ), exposureFactor, gamma);
        *( j + 1 ) = convertToDisplay(*( data + i + PIXEL_GREEN ), exposureFactor, gamma);
        *( j + 0 ) = convertToDisplay(*( data + i + PIXEL_BLUE ), exposureFactor, gamma);
        i += MAX_CHANNEL_RGBA;
        j += MAX_CHANNEL_RGBA;
    }

    /*
    if (srcProfile != 0 && dstProfile != 0) {
        convertPixelsTo(img.bits(), srcProfile,
                img.bits(), this, dstProfile,
                width * height, renderingIntent);
    }
    */
    return img;
}


void KisRgbF32ColorSpace::compositeOver(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    while (rows > 0) {

        const float *src = reinterpret_cast<const float *>(srcRowStart);
        float *dst = reinterpret_cast<float *>(dstRowStart);
        const quint8 *mask = maskRowStart;
        qint32 columns = numColumns;

        while (columns > 0) {

            float srcAlpha = src[PIXEL_ALPHA];

            // apply the alphamask
            if (mask != 0) {
                quint8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha *= UINT8_TO_FLOAT(U8_mask);
                }
                mask++;
            }

            if (srcAlpha > F32_OPACITY_TRANSPARENT + EPSILON) {

                if (opacity < F32_OPACITY_OPAQUE - EPSILON) {
                    srcAlpha *= opacity;
                }

                if (srcAlpha > F32_OPACITY_OPAQUE - EPSILON) {
                    memcpy(dst, src, MAX_CHANNEL_RGBA * sizeof(float));
                } else {
                    float dstAlpha = dst[PIXEL_ALPHA];

                    float srcBlend;

                    if (dstAlpha > F32_OPACITY_OPAQUE - EPSILON) {
                        srcBlend = srcAlpha;
                    } else {
                        float newAlpha = dstAlpha + (F32_OPACITY_OPAQUE - dstAlpha) * srcAlpha;
                        dst[PIXEL_ALPHA] = newAlpha;

                        if (newAlpha > EPSILON) {
                            srcBlend = srcAlpha / newAlpha;
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend > F32_OPACITY_OPAQUE - EPSILON) {
                        memcpy(dst, src, MAX_CHANNEL_RGB * sizeof(float));
                    } else {
                        dst[PIXEL_RED] = FLOAT_BLEND(src[PIXEL_RED], dst[PIXEL_RED], srcBlend);
                        dst[PIXEL_GREEN] = FLOAT_BLEND(src[PIXEL_GREEN], dst[PIXEL_GREEN], srcBlend);
                        dst[PIXEL_BLUE] = FLOAT_BLEND(src[PIXEL_BLUE], dst[PIXEL_BLUE], srcBlend);
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
        if(maskRowStart) {
            maskRowStart += maskRowStride;
        }
    }
}

#define COMMON_COMPOSITE_OP_PROLOG() \
    while (rows > 0) { \
    \
        const float *src = reinterpret_cast<const float *>(srcRowStart); \
        float *dst = reinterpret_cast<float *>(dstRowStart); \
        qint32 columns = numColumns; \
        const quint8 *mask = maskRowStart; \
    \
        while (columns > 0) { \
    \
            float srcAlpha = src[PIXEL_ALPHA]; \
            float dstAlpha = dst[PIXEL_ALPHA]; \
    \
            srcAlpha = qMin(srcAlpha, dstAlpha); \
    \
            if (mask != 0) { \
                quint8 U8_mask = *mask; \
    \
                if (U8_mask != OPACITY_OPAQUE) { \
                    srcAlpha *= UINT8_TO_FLOAT(U8_mask); \
                } \
                mask++; \
            } \
    \
            if (srcAlpha > F32_OPACITY_TRANSPARENT + EPSILON) { \
    \
                if (opacity < F32_OPACITY_OPAQUE - EPSILON) { \
                    srcAlpha *= opacity; \
                } \
    \
                float srcBlend; \
    \
                if (dstAlpha > F32_OPACITY_OPAQUE - EPSILON) { \
                    srcBlend = srcAlpha; \
                } else { \
                    float newAlpha = dstAlpha + (F32_OPACITY_OPAQUE - dstAlpha) * srcAlpha; \
                    dst[PIXEL_ALPHA] = newAlpha; \
    \
                    if (newAlpha > EPSILON) { \
                        srcBlend = srcAlpha / newAlpha; \
                    } else { \
                        srcBlend = srcAlpha; \
                    } \
                }

#define COMMON_COMPOSITE_OP_EPILOG() \
            } \
    \
            columns--; \
            src += MAX_CHANNEL_RGBA; \
            dst += MAX_CHANNEL_RGBA; \
        } \
    \
        rows--; \
        srcRowStart += srcRowStride; \
        dstRowStart += dstRowStride; \
        if(maskRowStart) { \
            maskRowStart += maskRowStride; \
        } \
    }

void KisRgbF32ColorSpace::compositeMultiply(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float srcColor = src[PIXEL_RED];
        float dstColor = dst[PIXEL_RED];

        srcColor = srcColor * dstColor;

        dst[PIXEL_RED] = FLOAT_BLEND(srcColor, dstColor, srcBlend);

        srcColor = src[PIXEL_GREEN];
        dstColor = dst[PIXEL_GREEN];

        srcColor = srcColor * dstColor;

        dst[PIXEL_GREEN] = FLOAT_BLEND(srcColor, dstColor, srcBlend);

        srcColor = src[PIXEL_BLUE];
        dstColor = dst[PIXEL_BLUE];

        srcColor = srcColor * dstColor;

        dst[PIXEL_BLUE] = FLOAT_BLEND(srcColor, dstColor, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeDivide(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            float srcColor = src[channel];
            float dstColor = dst[channel];

            srcColor = qMin((float)(dstColor / (srcColor + EPSILON)), FLOAT_MAX);

            float newColor = FLOAT_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeScreen(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            float srcColor = src[channel];
            float dstColor = dst[channel];

            srcColor = FLOAT_MAX - ((FLOAT_MAX - dstColor) * (FLOAT_MAX - srcColor));

            float newColor = FLOAT_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeOverlay(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            float srcColor = src[channel];
            float dstColor = dst[channel];

            srcColor = dstColor * (dstColor + 2 * (srcColor * (FLOAT_MAX - dstColor)));

            float newColor = FLOAT_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeDodge(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            float srcColor = src[channel];
            float dstColor = dst[channel];

            srcColor = qMin((float)(dstColor / (FLOAT_MAX + EPSILON - srcColor)), FLOAT_MAX);

            float newColor = FLOAT_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeBurn(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            float srcColor = src[channel];
            float dstColor = dst[channel];

            srcColor = qMin((float)((FLOAT_MAX - dstColor) / (srcColor + EPSILON)), FLOAT_MAX);
            srcColor = CLAMP(FLOAT_MAX - srcColor, 0, FLOAT_MAX);

            float newColor = FLOAT_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeDarken(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            float srcColor = src[channel];
            float dstColor = dst[channel];

            srcColor = qMin(srcColor, dstColor);

            float newColor = FLOAT_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeLighten(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

            float srcColor = src[channel];
            float dstColor = dst[channel];

            srcColor = qMax(srcColor, dstColor);

            float newColor = FLOAT_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeHue(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float srcRed = src[PIXEL_RED];
        float srcGreen = src[PIXEL_GREEN];
        float srcBlue = src[PIXEL_BLUE];

        float dstRed = dst[PIXEL_RED];
        float dstGreen = dst[PIXEL_GREEN];
        float dstBlue = dst[PIXEL_BLUE];

        float srcHue;
        float srcSaturation;
        float srcValue;

        float dstHue;
        float dstSaturation;
        float dstValue;

        RGBToHSV(srcRed, srcGreen, srcBlue, &srcHue, &srcSaturation, &srcValue);
        RGBToHSV(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

        HSVToRGB(srcHue, dstSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

        dst[PIXEL_RED] = FLOAT_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = FLOAT_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = FLOAT_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeSaturation(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float srcRed = src[PIXEL_RED];
        float srcGreen = src[PIXEL_GREEN];
        float srcBlue = src[PIXEL_BLUE];

        float dstRed = dst[PIXEL_RED];
        float dstGreen = dst[PIXEL_GREEN];
        float dstBlue = dst[PIXEL_BLUE];

        float srcHue;
        float srcSaturation;
        float srcValue;

        float dstHue;
        float dstSaturation;
        float dstValue;

        RGBToHSV(srcRed, srcGreen, srcBlue, &srcHue, &srcSaturation, &srcValue);
        RGBToHSV(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

        HSVToRGB(dstHue, srcSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

        dst[PIXEL_RED] = FLOAT_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = FLOAT_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = FLOAT_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeValue(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float srcRed = src[PIXEL_RED];
        float srcGreen = src[PIXEL_GREEN];
        float srcBlue = src[PIXEL_BLUE];

        float dstRed = dst[PIXEL_RED];
        float dstGreen = dst[PIXEL_GREEN];
        float dstBlue = dst[PIXEL_BLUE];

        float srcHue;
        float srcSaturation;
        float srcValue;

        float dstHue;
        float dstSaturation;
        float dstValue;

        RGBToHSV(srcRed, srcGreen, srcBlue, &srcHue, &srcSaturation, &srcValue);
        RGBToHSV(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

        HSVToRGB(dstHue, dstSaturation, srcValue, &srcRed, &srcGreen, &srcBlue);

        dst[PIXEL_RED] = FLOAT_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = FLOAT_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = FLOAT_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeColor(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, float opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        float srcRed = src[PIXEL_RED];
        float srcGreen = src[PIXEL_GREEN];
        float srcBlue = src[PIXEL_BLUE];

        float dstRed = dst[PIXEL_RED];
        float dstGreen = dst[PIXEL_GREEN];
        float dstBlue = dst[PIXEL_BLUE];

        float srcHue;
        float srcSaturation;
        float srcLightness;

        float dstHue;
        float dstSaturation;
        float dstLightness;

        RGBToHSL(srcRed, srcGreen, srcBlue, &srcHue, &srcSaturation, &srcLightness);
        RGBToHSL(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstLightness);

        HSLToRGB(srcHue, srcSaturation, dstLightness, &srcRed, &srcGreen, &srcBlue);

        dst[PIXEL_RED] = FLOAT_BLEND(srcRed, dstRed, srcBlend);
        dst[PIXEL_GREEN] = FLOAT_BLEND(srcGreen, dstGreen, srcBlend);
        dst[PIXEL_BLUE] = FLOAT_BLEND(srcBlue, dstBlue, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisRgbF32ColorSpace::compositeErase(quint8 *dst,
            qint32 dstRowSize,
            const quint8 *src,
            qint32 srcRowSize,
            const quint8 *srcAlphaMask,
            qint32 maskRowStride,
            qint32 rows,
            qint32 cols,
            float /*opacity*/)
{
    while (rows-- > 0)
    {
        const Pixel *s = reinterpret_cast<const Pixel *>(src);
        Pixel *d = reinterpret_cast<Pixel *>(dst);
        const quint8 *mask = srcAlphaMask;

        for (qint32 i = cols; i > 0; i--, s++, d++)
        {
            float srcAlpha = s->alpha;

            // apply the alphamask
            if (mask != 0) {
                quint8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = FLOAT_BLEND(srcAlpha, F32_OPACITY_OPAQUE, UINT8_TO_FLOAT(U8_mask));
                }
                mask++;
            }
            d->alpha = srcAlpha * d->alpha;
        }

        dst += dstRowSize;
        src += srcRowSize;
        if(srcAlphaMask) {
            srcAlphaMask += maskRowStride;
        }
    }
}

void KisRgbF32ColorSpace::bitBlt(quint8 *dst,
                      qint32 dstRowStride,
                      const quint8 *src,
                      qint32 srcRowStride,
                      const quint8 *mask,
                      qint32 maskRowStride,
                      quint8 U8_opacity,
                      qint32 rows,
                      qint32 cols,
                      const KoCompositeOp* op)
{
    float opacity = UINT8_TO_FLOAT(U8_opacity);

    switch (op.op()) {
    case COMPOSITE_UNDEF:
        // Undefined == no composition
        break;
    case COMPOSITE_OVER:
        compositeOver(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_IN:
        //compositeIn(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    case COMPOSITE_OUT:
        //compositeOut(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ATOP:
        //compositeAtop(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_XOR:
        //compositeXor(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_PLUS:
        //compositePlus(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_MINUS:
        //compositeMinus(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ADD:
        //compositeAdd(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SUBTRACT:
        //compositeSubtract(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DIFF:
        //compositeDiff(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_MULT:
        compositeMultiply(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DIVIDE:
        compositeDivide(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_BUMPMAP:
        //compositeBumpmap(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY:
        compositeCopy(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, U8_opacity);
        break;
    case COMPOSITE_COPY_RED:
        //compositeCopyRed(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_GREEN:
        //compositeCopyGreen(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_BLUE:
        //compositeCopyBlue(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_OPACITY:
        //compositeCopyOpacity(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_CLEAR:
        //compositeClear(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DISSOLVE:
        //compositeDissolve(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DISPLACE:
        //compositeDisplace(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
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
        //compositeColorize(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_LUMINIZE:
        //compositeLuminize(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
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

KoCompositeOpList KisRgbF32ColorSpace::userVisiblecompositeOps() const
{
    KoCompositeOpList list;

    list.append(KoCompositeOp(COMPOSITE_OVER));
    list.append(KoCompositeOp(COMPOSITE_MULT));
    list.append(KoCompositeOp(COMPOSITE_BURN));
    list.append(KoCompositeOp(COMPOSITE_DODGE));
    list.append(KoCompositeOp(COMPOSITE_DIVIDE));
    list.append(KoCompositeOp(COMPOSITE_SCREEN));
    list.append(KoCompositeOp(COMPOSITE_OVERLAY));
    list.append(KoCompositeOp(COMPOSITE_DARKEN));
    list.append(KoCompositeOp(COMPOSITE_LIGHTEN));
    list.append(KoCompositeOp(COMPOSITE_HUE));
    list.append(KoCompositeOp(COMPOSITE_SATURATION));
    list.append(KoCompositeOp(COMPOSITE_VALUE));
    list.append(KoCompositeOp(COMPOSITE_COLOR));

    return list;
}

