/*
 *  Copyright (c) 2003 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can CYANistribute it and/or modify
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

#include <kdebug.h>
#include <klocale.h>

#include "kis_cmyk_colorspace.h"
#include "kis_u8_base_colorspace.h"
#include "kis_colorspace_factory_registry.h"

#include "kis_profile.h"
#include "kis_integer_maths.h"

namespace cmyk {
    const Q_INT32 MAX_CHANNEL_CMYK = 4;
    const Q_INT32 MAX_CHANNEL_CMYKA = 5;
}

KisCmykColorSpace::KisCmykColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) :
    KisU8BaseColorSpace(KisID("CMYK", i18n("CMYK")), TYPE_CMYK5_8, icSigCmykData, parent, p)
{
    m_channels.push_back(new KisChannelInfo(i18n("Cyan"), 0, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, Qt::cyan));
    m_channels.push_back(new KisChannelInfo(i18n("Magenta"), 1, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, Qt::magenta));
    m_channels.push_back(new KisChannelInfo(i18n("Yellow"), 2, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, Qt::yellow));
    m_channels.push_back(new KisChannelInfo(i18n("Black"), 3, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, Qt::black));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), 4, KisChannelInfo::ALPHA, KisChannelInfo::UINT8, 1, Qt::white));

    m_alphaPos = PIXEL_CMYK_ALPHA;

    init();
}

KisCmykColorSpace::~KisCmykColorSpace()
{
}

void KisCmykColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
    Q_UINT32 totalCyan = 0, totalMagenta = 0, totalYellow = 0, totalK = 0, totalAlpha = 0;

    while (nColors--)
    {
        Q_UINT32 alpha = (*colors)[4];
        Q_UINT32 alphaTimesWeight = alpha * *weights;

        totalCyan += (*colors)[0] * alphaTimesWeight;
        totalMagenta += (*colors)[1] * alphaTimesWeight;
        totalYellow += (*colors)[2] * alphaTimesWeight;
        totalK += (*colors)[3] * alphaTimesWeight;
        totalAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    //Q_ASSERT(newAlpha <= 255*255);
    if (totalAlpha > 255*255) totalAlpha = 255*255;

    // Divide by 255.
    dst[4] =(((totalAlpha + 0x80)>>8)+totalAlpha) >>8;

    if (totalAlpha > 0) {
        totalCyan = totalCyan / totalAlpha;
        totalMagenta = totalMagenta / totalAlpha;
        totalYellow = totalYellow / totalAlpha;
        totalK = totalK / totalAlpha;
    } // else the values are already 0 too

    Q_UINT32 dstCyan = totalCyan;
    if (dstCyan > 255) dstCyan = 255;
    dst[0] = dstCyan;

    Q_UINT32 dstMagenta = totalMagenta;
    if (dstMagenta > 255) dstMagenta = 255;
    dst[1] = dstMagenta;

    Q_UINT32 dstYellow = totalYellow;
    if (dstYellow > 255) dstYellow = 255;
    dst[2] = dstYellow;

    Q_UINT32 dstK = totalK;
    if (dstK > 255) dstK = 255;
    dst[3] = dstK;
}


void KisCmykColorSpace::convolveColors(Q_UINT8** colors, Q_INT32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nColors) const
{
    Q_INT32 totalCyan = 0, totalMagenta = 0, totalYellow = 0, totalK = 0, totalAlpha = 0;

    while (nColors--)
    {
        Q_INT32 weight = *kernelValues;

        if (weight != 0) {
            totalCyan += (*colors)[PIXEL_CYAN] * weight;
            totalMagenta += (*colors)[PIXEL_MAGENTA] * weight;
            totalYellow += (*colors)[PIXEL_YELLOW] * weight;
            totalK += (*colors)[PIXEL_BLACK] * weight;
            totalAlpha += (*colors)[PIXEL_CMYK_ALPHA] * weight;
        }
        colors++;
        kernelValues++;
    }


    if (channelFlags & KisChannelInfo::FLAG_COLOR) {
        dst[PIXEL_CYAN] = CLAMP((totalCyan / factor) + offset, 0, Q_UINT8_MAX);
        dst[PIXEL_MAGENTA] = CLAMP((totalMagenta / factor) + offset, 0, Q_UINT8_MAX);
        dst[PIXEL_YELLOW] =  CLAMP((totalYellow / factor) + offset, 0, Q_UINT8_MAX);
        dst[PIXEL_BLACK] =  CLAMP((totalK / factor) + offset, 0, Q_UINT8_MAX);
    }
    if (channelFlags & KisChannelInfo::FLAG_ALPHA) {
        dst[PIXEL_CMYK_ALPHA] = CLAMP((totalAlpha/ factor) + offset, 0, Q_UINT8_MAX);
    }
}


void KisCmykColorSpace::invertColor(Q_UINT8 * src, Q_INT32 nPixels)
{
    Q_UINT32 psize = pixelSize();

    while (nPixels--)
    {
        src[PIXEL_CYAN] = Q_UINT8_MAX - src[PIXEL_CYAN];
        src[PIXEL_MAGENTA] = Q_UINT8_MAX - src[PIXEL_MAGENTA];
        src[PIXEL_YELLOW] = Q_UINT8_MAX - src[PIXEL_YELLOW];
        src[PIXEL_BLACK] = Q_UINT8_MAX - src[PIXEL_BLACK];
        src += psize;
    }
}

void KisCmykColorSpace::applyAdjustment(const Q_UINT8 *src, Q_UINT8 *dst, KisColorAdjustment *adj, Q_INT32 nPixels)
{
    Q_UINT32 psize = pixelSize();

    Q_UINT8 * tmp = new Q_UINT8[nPixels * psize];
    Q_UINT8 * tmpPtr = tmp;
    memcpy(tmp, dst, nPixels * psize);

    KisAbstractColorSpace::applyAdjustment(src, dst, adj, nPixels);

    // Copy the alpha, which lcms doesn't do for us, grumble.

    while (nPixels--)
    {
        dst[4] = tmpPtr[4];

        tmpPtr += psize;
        dst += psize;
    }

    delete [] tmp;
}

QValueVector<KisChannelInfo *> KisCmykColorSpace::channels() const
{
    return m_channels;
}

Q_UINT32 KisCmykColorSpace::nChannels() const
{
    return cmyk::MAX_CHANNEL_CMYKA;
}

Q_UINT32 KisCmykColorSpace::nColorChannels() const
{
    return cmyk::MAX_CHANNEL_CMYK;
}

Q_UINT32 KisCmykColorSpace::pixelSize() const
{
    return cmyk::MAX_CHANNEL_CMYKA;
}


void KisCmykColorSpace::bitBlt(Q_UINT8 *dst,
            Q_INT32 dstRowStride,
            const Q_UINT8 *src,
            Q_INT32 srcRowStride,
            const Q_UINT8 *srcAlphaMask,
            Q_INT32 maskRowStride,
            Q_UINT8 opacity,
            Q_INT32 rows,
            Q_INT32 cols,
            const KisCompositeOp& op)
{

    Q_INT32 linesize = pixelSize() * sizeof(Q_UINT8) * cols;
    Q_UINT8 *d;
    const Q_UINT8 *s;

    if (rows <= 0 || cols <= 0)
        return;

    switch (op.op()) {
    case COMPOSITE_COPY:
        compositeCopy(dst, dstRowStride, src, srcRowStride, srcAlphaMask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_CLEAR:
        d = dst;
        s = src;
        while (rows-- > 0) {
            memset(d, 0, linesize);
            d += dstRowStride;
        }
        break;
    case COMPOSITE_OVER:
    default:
        compositeOver(dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }

}


// XXX: Cut & Paste from colorspace_rgb

void KisCmykColorSpace::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride,
                         const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride,
                         Q_INT32 rows, Q_INT32 numColumns,
                         Q_UINT8 opacity)
{
    while (rows > 0) {

        const Q_UINT8 *src = srcRowStart;
        Q_UINT8 *dst = dstRowStart;
        Q_INT32 columns = numColumns;

        while (columns > 0) {
            Q_UINT8 srcAlpha = src[PIXEL_CMYK_ALPHA];

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_CMYK_ALPHA], opacity);
                }

                if (srcAlpha == OPACITY_OPAQUE) {
                    memcpy(dst, src, cmyk::MAX_CHANNEL_CMYKA * sizeof(Q_UINT8));
                } else {
                    Q_UINT8 dstAlpha = dst[PIXEL_CMYK_ALPHA];

                    Q_UINT8 srcBlend;

                    if (dstAlpha == OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dst[PIXEL_CMYK_ALPHA] = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == OPACITY_OPAQUE) {
                        memcpy(dst, src, cmyk::MAX_CHANNEL_CMYKA * sizeof(Q_UINT8));
                    } else {
                        dst[PIXEL_CYAN] = UINT8_BLEND(src[PIXEL_CYAN], dst[PIXEL_CYAN], srcBlend);
                        dst[PIXEL_MAGENTA] = UINT8_BLEND(src[PIXEL_MAGENTA], dst[PIXEL_MAGENTA], srcBlend);
                        dst[PIXEL_YELLOW] = UINT8_BLEND(src[PIXEL_YELLOW], dst[PIXEL_YELLOW], srcBlend);
                        dst[PIXEL_BLACK] = UINT8_BLEND(src[PIXEL_BLACK], dst[PIXEL_BLACK], srcBlend);
                    }
                }
            }

            columns--;
            src += cmyk::MAX_CHANNEL_CMYKA;
            dst += cmyk::MAX_CHANNEL_CMYKA;

        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
    }
}


KisCompositeOpList KisCmykColorSpace::userVisiblecompositeOps() const
{
    KisCompositeOpList list;

    list.append(KisCompositeOp(COMPOSITE_OVER));

    return list;
}
