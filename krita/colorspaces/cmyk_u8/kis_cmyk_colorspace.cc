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
    m_channels.push_back(new KisChannelInfo(i18n("Cyan"), 0, KisChannelInfo::COLOR, KisChannelInfo::UINT8));
    m_channels.push_back(new KisChannelInfo(i18n("Magenta"), 1, KisChannelInfo::COLOR, KisChannelInfo::UINT8));
    m_channels.push_back(new KisChannelInfo(i18n("Yellow"), 2, KisChannelInfo::COLOR, KisChannelInfo::UINT8));
    m_channels.push_back(new KisChannelInfo(i18n("Black"), 3, KisChannelInfo::COLOR, KisChannelInfo::UINT8));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), 4, KisChannelInfo::ALPHA, KisChannelInfo::UINT8));

    m_alphaPos = PIXEL_CMYK_ALPHA;

    init();
}

KisCmykColorSpace::~KisCmykColorSpace()
{
}

void KisCmykColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
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


void KisCmykColorSpace::adjustBrightness(Q_UINT8 *src1, Q_INT8 adjust) const
{
    //XXX does nothing for now
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
            /*
            if (src[PIXEL_CYAN] == 0
                && src[PIXEL_MAGENTA] == 0
                && src[PIXEL_YELLOW] == 0
                && src[PIXEL_BLACK] == 0) {
                // Skip; we don't put any new ink over the old.
            } else if (opacity == OPACITY_OPAQUE) {
                memcpy(dst, src, cmyk::MAX_CHANNEL_CMYKA * sizeof(Q_UINT8));

            } else {

                dst[PIXEL_CYAN] = UINT8_BLEND(src[PIXEL_CYAN], dst[PIXEL_CYAN], opacity);
                dst[PIXEL_MAGENTA] = UINT8_BLEND(src[PIXEL_MAGENTA], dst[PIXEL_MAGENTA], opacity);
                dst[PIXEL_YELLOW] = UINT8_BLEND(src[PIXEL_YELLOW], dst[PIXEL_YELLOW], opacity);
                dst[PIXEL_BLACK] = UINT8_BLEND(src[PIXEL_BLACK], dst[PIXEL_BLACK], opacity);

            }

            columns--;
            src += cmyk::MAX_CHANNEL_CMYKA;
            dst += cmyk::MAX_CHANNEL_CMYKA;*/
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
