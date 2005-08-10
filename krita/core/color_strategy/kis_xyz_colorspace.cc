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

#include "kis_abstract_colorspace.h"
#include "kis_colorspace_registry.h"
#include "kis_image.h"
#include "kis_xyz_colorspace.h"
#include "kis_iterators_pixel.h"
#include "kis_integer_maths.h"

#define downscale(quantum)  (quantum) //((unsigned char) ((quantum)/257UL))
#define upscale(value)  (value) // ((QUANTUM) (257UL*(value)))

namespace xyz {
    const Q_INT32 MAX_CHANNEL_XYZ = 3;
    const Q_INT32 MAX_CHANNEL_XYZA = 4;
}


const Q_UINT16 KisXyzColorSpace::U16_OPACITY_OPAQUE;
const Q_UINT16 KisXyzColorSpace::U16_OPACITY_TRANSPARENT;


// XXX: Maybe use TYPE_XYZ_DBL for an extra stimulating performance hit? People shouldn't depend
//      on this fallback...

KisXyzColorSpace::KisXyzColorSpace() :
    KisAbstractColorSpace(KisID("XYZA", i18n("XYZ/Alpha")), TYPE_XYZ_16, icSigCmykData)
{
    m_channels.push_back(new KisChannelInfo(i18n("X"), 0, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Y"), 0, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Z"), 0, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), 4, ALPHA));

    cmsHPROFILE hProfile = cmsCreateXYZProfile();

    setDefaultProfile( new KisProfile(hProfile, TYPE_XYZ_16) );

    // For conversions from default rgb
    cmsHPROFILE hsRGB = cmsCreate_sRGBProfile();
    
    m_defaultFromRGB = cmsCreateTransform(hsRGB, TYPE_BGR_8,
                          hProfile, TYPE_XYZ_16,
                          INTENT_PERCEPTUAL, 0);

    m_defaultToRGB =  cmsCreateTransform(hProfile, TYPE_XYZ_16,
                         hsRGB, TYPE_BGR_8,
                         INTENT_PERCEPTUAL, 0);


    // Default pixel buffer for QColor conversion
    m_qcolordata = new Q_UINT8[3];
    Q_CHECK_PTR(m_qcolordata);

}


KisXyzColorSpace::~KisXyzColorSpace()
{
}

void KisXyzColorSpace::nativeColor(const QColor& color, Q_UINT8 *dst, KisProfileSP /*profile*/)
{
    m_qcolordata[2] = color.red();
    m_qcolordata[1] = color.green();
    m_qcolordata[0] = color.blue();

    // XXX: Use proper conversion from RGB with profiles
    cmsDoTransform(m_defaultFromRGB, m_qcolordata, dst, 1);
    dst[4] = OPACITY_OPAQUE;

}

void KisXyzColorSpace::nativeColor(const QColor& color, QUANTUM opacity, Q_UINT8 *dst, KisProfileSP /*profile*/)
{

    m_qcolordata[2] = color.red();
    m_qcolordata[1] = color.green();
    m_qcolordata[0] = color.blue();

    // XXX: Use proper conversion from RGB with profiles
    cmsDoTransform(m_defaultFromRGB, m_qcolordata, dst, 1);
    dst[4] = opacity;
}

void KisXyzColorSpace::getAlpha(const Q_UINT8 *U8_pixel, Q_UINT8 *alpha)
{
    const Pixel *pixel = reinterpret_cast<const Pixel *>(U8_pixel);
    *alpha = UINT16_TO_UINT8(pixel -> alpha);
}

void KisXyzColorSpace::setAlpha(Q_UINT8 *pixels, Q_UINT8 alpha, Q_INT32 nPixels)
{
    Pixel *pixel = reinterpret_cast<Pixel *>(pixels);

    while (nPixels > 0) {
        pixel -> alpha = UINT8_TO_UINT16(alpha);
        --nPixels;
        ++pixel;
    }
}

void KisXyzColorSpace::toQColor(const Q_UINT8 *src, QColor *c, KisProfileSP /*profile*/)
{
    // XXX: Properly convert using the rgb colorspace and the profile
    cmsDoTransform(m_defaultToRGB, const_cast <Q_UINT8 *>(src), m_qcolordata, 1);
    c -> setRgb(m_qcolordata[2], m_qcolordata[1], m_qcolordata[0]);
}

void KisXyzColorSpace::toQColor(const Q_UINT8 *src, QColor *c, QUANTUM *opacity, KisProfileSP /*profile*/)
{
    // XXX: Properly convert using the rgb colorspace and the profile
    cmsDoTransform(m_defaultToRGB, const_cast <Q_UINT8 *>(src), m_qcolordata, 1);
    c -> setRgb(m_qcolordata[2], m_qcolordata[1], m_qcolordata[0]);

     *opacity = src[4];
}

Q_INT8 KisXyzColorSpace::difference(const Q_UINT8 *src1, const Q_UINT8 *src2)
{
}

void KisXyzColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
}

vKisChannelInfoSP KisXyzColorSpace::channels() const
{
    return m_channels;
}

bool KisXyzColorSpace::hasAlpha() const
{
    return true;
}

Q_INT32 KisXyzColorSpace::nChannels() const
{
    return xyz::MAX_CHANNEL_XYZA;
}

Q_INT32 KisXyzColorSpace::nColorChannels() const
{
    return xyz::MAX_CHANNEL_XYZ;
}

Q_INT32 KisXyzColorSpace::pixelSize() const
{
    return xyz::MAX_CHANNEL_XYZA * sizeof(Q_UINT16);
}


QImage KisXyzColorSpace::convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                     KisProfileSP srcProfile, KisProfileSP dstProfile,
                     Q_INT32 renderingIntent, float /*exposure*/)
{
    QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
    memset(img.bits(), 255, width * height * sizeof(Q_UINT32));
    KisAbstractColorSpace * dstCS = KisColorSpaceRegistry::instance() -> get("RGBA");


     if (srcProfile == 0 || dstProfile == 0 || dstCS == 0) {
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                 cmsDoTransform(m_defaultToRGB,
                    const_cast<Q_UINT8 *>(&(data[xyz::MAX_CHANNEL_XYZA*(i*width+j)])),
                    &(img.scanLine(i)[j*img.bytesPerLine()/width]), 1);
     }
     else {
         // Do a nice calibrated conversion
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                convertPixelsTo(const_cast<Q_UINT8 *>
                        (&(data[xyz::MAX_CHANNEL_XYZA*(i*width+j)])),
                        srcProfile,
                        &(img.scanLine(i)[j*img.bytesPerLine()/width]),
                        dstCS, dstProfile, 1, renderingIntent);
     }

    return img;
}

void KisXyzColorSpace::adjustBrightnessContrast(const Q_UINT8 *src, Q_UINT8 *dst, Q_INT8 brightness, Q_INT8 contrast, Q_INT32 nPixels) const
{
}

void KisXyzColorSpace::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    while (rows > 0) {

        const Q_UINT16 *src = reinterpret_cast<const Q_UINT16 *>(srcRowStart);
        Q_UINT16 *dst = reinterpret_cast<Q_UINT16 *>(dstRowStart);
        const Q_UINT8 *mask = maskRowStart;
        Q_INT32 columns = numColumns;

        while (columns > 0) {

            Q_UINT16 srcAlpha = src[PIXEL_ALPHA];

            // apply the alphamask
            if (mask != 0) {
                Q_UINT8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = UINT16_MULT(srcAlpha, UINT8_TO_UINT16(U8_mask));
                }
                mask++;
            }
            
            if (srcAlpha != U16_OPACITY_TRANSPARENT) {

                if (opacity != U16_OPACITY_OPAQUE) {
                    srcAlpha = UINT16_MULT(srcAlpha, opacity);
                }

                if (srcAlpha == U16_OPACITY_OPAQUE) {
                    memcpy(dst, src, xyz::MAX_CHANNEL_XYZA * sizeof(Q_UINT16));
                } else {
                    Q_UINT16 dstAlpha = dst[PIXEL_ALPHA];

                    Q_UINT16 srcBlend;

                    if (dstAlpha == U16_OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        Q_UINT16 newAlpha = dstAlpha + UINT16_MULT(U16_OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dst[PIXEL_ALPHA] = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT16_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == U16_OPACITY_OPAQUE) {
                        memcpy(dst, src, xyz::MAX_CHANNEL_XYZ * sizeof(Q_UINT16));
                    } else {
                        dst[PIXEL_X] = UINT16_BLEND(src[PIXEL_X], dst[PIXEL_X], srcBlend);
                        dst[PIXEL_Y] = UINT16_BLEND(src[PIXEL_Y], dst[PIXEL_Y], srcBlend);
                        dst[PIXEL_Z] = UINT16_BLEND(src[PIXEL_Z], dst[PIXEL_Z], srcBlend);
                    }
                }
            }

            columns--;
            src += xyz::MAX_CHANNEL_XYZA;
            dst += xyz::MAX_CHANNEL_XYZA;
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
        const Q_UINT16 *src = reinterpret_cast<const Q_UINT16 *>(srcRowStart); \
        Q_UINT16 *dst = reinterpret_cast<Q_UINT16 *>(dstRowStart); \
        Q_INT32 columns = numColumns; \
        const Q_UINT8 *mask = maskRowStart; \
    \
        while (columns > 0) { \
    \
            Q_UINT16 srcAlpha = src[PIXEL_ALPHA]; \
            Q_UINT16 dstAlpha = dst[PIXEL_ALPHA]; \
    \
            srcAlpha = QMIN(srcAlpha, dstAlpha); \
    \
            if (mask != 0) { \
                Q_UINT8 U8_mask = *mask; \
    \
                if (U8_mask != OPACITY_OPAQUE) { \
                    srcAlpha = UINT16_MULT(srcAlpha, UINT8_TO_UINT16(U8_mask)); \
                } \
                mask++; \
            } \
    \
            if (srcAlpha != U16_OPACITY_TRANSPARENT) { \
    \
                if (opacity != U16_OPACITY_OPAQUE) { \
                    srcAlpha = UINT16_MULT(srcAlpha, opacity); \
                } \
    \
                Q_UINT16 srcBlend; \
    \
                if (dstAlpha == U16_OPACITY_OPAQUE) { \
                    srcBlend = srcAlpha; \
                } else { \
                    Q_UINT16 newAlpha = dstAlpha + UINT16_MULT(U16_OPACITY_OPAQUE - dstAlpha, srcAlpha); \
                    dst[PIXEL_ALPHA] = newAlpha; \
    \
                    if (newAlpha != 0) { \
                        srcBlend = UINT16_DIVIDE(srcAlpha, newAlpha); \
                    } else { \
                        srcBlend = srcAlpha; \
                    } \
                }

#define COMMON_COMPOSITE_OP_EPILOG() \
            } \
    \
            columns--; \
            src += xyz::MAX_CHANNEL_XYZA; \
            dst += xyz::MAX_CHANNEL_XYZA; \
        } \
    \
        rows--; \
        srcRowStart += srcRowStride; \
        dstRowStart += dstRowStride; \
        if(maskRowStart) { \
            maskRowStart += maskRowStride; \
        } \
    }

void KisXyzColorSpace::compositeMultiply(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        Q_UINT16 srcColor = src[PIXEL_X];
        Q_UINT16 dstColor = dst[PIXEL_X];

        srcColor = UINT16_MULT(srcColor, dstColor);

        dst[PIXEL_X] = UINT16_BLEND(srcColor, dstColor, srcBlend);

        srcColor = src[PIXEL_Y];
        dstColor = dst[PIXEL_Y];

        srcColor = UINT16_MULT(srcColor, dstColor);

        dst[PIXEL_Y] = UINT16_BLEND(srcColor, dstColor, srcBlend);

        srcColor = src[PIXEL_Z];
        dstColor = dst[PIXEL_Z];

        srcColor = UINT16_MULT(srcColor, dstColor);

        dst[PIXEL_Z] = UINT16_BLEND(srcColor, dstColor, srcBlend);
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisXyzColorSpace::compositeDivide(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < xyz::MAX_CHANNEL_XYZ; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = QMIN((dstColor * (UINT16_MAX + 1u) + (srcColor / 2u)) / (1u + srcColor), UINT16_MAX);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisXyzColorSpace::compositeScreen(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < xyz::MAX_CHANNEL_XYZ; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = UINT16_MAX - UINT16_MULT(UINT16_MAX - dstColor, UINT16_MAX - srcColor);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisXyzColorSpace::compositeOverlay(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < xyz::MAX_CHANNEL_XYZ; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = UINT16_MULT(dstColor, dstColor + 2u * UINT16_MULT(srcColor, UINT16_MAX - dstColor));

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisXyzColorSpace::compositeDodge(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < xyz::MAX_CHANNEL_XYZ; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = QMIN((dstColor * (UINT16_MAX + 1u)) / (UINT16_MAX + 1u - srcColor), UINT16_MAX);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisXyzColorSpace::compositeBurn(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < xyz::MAX_CHANNEL_XYZ; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = QMIN(((UINT16_MAX - dstColor) * (UINT16_MAX + 1u)) / (srcColor + 1u), UINT16_MAX);
            srcColor = CLAMP(UINT16_MAX - srcColor, 0u, UINT16_MAX);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisXyzColorSpace::compositeDarken(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < xyz::MAX_CHANNEL_XYZ; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = QMIN(srcColor, dstColor);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisXyzColorSpace::compositeLighten(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < xyz::MAX_CHANNEL_XYZ; channel++) {

            Q_UINT16 srcColor = src[channel];
            Q_UINT16 dstColor = dst[channel];

            srcColor = QMAX(srcColor, dstColor);

            Q_UINT16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}



void KisXyzColorSpace::compositeErase(Q_UINT8 *dst,
            Q_INT32 dstRowSize,
            const Q_UINT8 *src, 
            Q_INT32 srcRowSize,
            const Q_UINT8 *srcAlphaMask,
            Q_INT32 maskRowStride,
            Q_INT32 rows, 
            Q_INT32 cols, 
            Q_UINT16 /*opacity*/)
{
    while (rows-- > 0)
    {
        const Pixel *s = reinterpret_cast<const Pixel *>(src);
        Pixel *d = reinterpret_cast<Pixel *>(dst);
        const Q_UINT8 *mask = srcAlphaMask;

        for (Q_INT32 i = cols; i > 0; i--, s++, d++)
        {
            Q_UINT16 srcAlpha = s -> alpha;

            // apply the alphamask
            if (mask != 0) {
                Q_UINT8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = UINT16_BLEND(srcAlpha, U16_OPACITY_OPAQUE, UINT8_TO_UINT16(U8_mask));
                }
                mask++;
            }
            d -> alpha = UINT16_MULT(srcAlpha, d -> alpha);
        }

        dst += dstRowSize;
        src += srcRowSize;
        if(srcAlphaMask) {
            srcAlphaMask += maskRowStride;
        }
    }
}

void KisXyzColorSpace::compositeCopy(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride,
                        const Q_UINT8 */*maskRowStart*/, Q_INT32 /*maskRowStride*/, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 /*opacity*/)
{
    while (rows > 0) {
        memcpy(dstRowStart, srcRowStart, numColumns * sizeof(Pixel));
        --rows;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
    }
}

void KisXyzColorSpace::bitBlt(Q_UINT8 *dst,
                      Q_INT32 dstRowStride,
                      const Q_UINT8 *src,
                      Q_INT32 srcRowStride,
                      const Q_UINT8 *mask,
                      Q_INT32 maskRowStride,
                      QUANTUM U8_opacity,
                      Q_INT32 rows,
                      Q_INT32 cols,
                      const KisCompositeOp& op)
{
    Q_UINT16 opacity = UINT8_TO_UINT16(U8_opacity);

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
        compositeCopy(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
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
        //compositeHue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SATURATION:
        //compositeSaturation(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_VALUE:
        //compositeValue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COLOR:
        //compositeColor(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
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

KisCompositeOpList KisXyzColorSpace::userVisiblecompositeOps() const
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

    return list;
}

QString KisXyzColorSpace::channelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    const Q_UINT16 *pixel = reinterpret_cast<const Q_UINT16 *>(U8_pixel);
    Q_UINT32 channelPosition = m_channels[channelIndex] -> pos() / sizeof(Q_UINT16);

    return QString().setNum(pixel[channelPosition]);
}

QString KisXyzColorSpace::normalisedChannelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    const Q_UINT16 *pixel = reinterpret_cast<const Q_UINT16 *>(U8_pixel);
    Q_UINT32 channelPosition = m_channels[channelIndex] -> pos() / sizeof(Q_UINT16);

    return QString().setNum(static_cast<float>(pixel[channelPosition]) / UINT16_MAX);
}

