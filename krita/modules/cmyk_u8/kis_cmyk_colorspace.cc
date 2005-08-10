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

#include "kis_config.h"
#include "kis_image.h"
#include "kis_cmyk_colorspace.h"
#include "kis_colorspace_registry.h"
#include "kis_iterators_pixel.h"

#include "kis_factory.h"
#include "kis_profile.h"
#include "kis_integer_maths.h"

namespace cmyk {
    const Q_INT32 MAX_CHANNEL_CMYK = 4;
    const Q_INT32 MAX_CHANNEL_CMYKA = 5;
}

KisCmykColorSpace::KisCmykColorSpace() :
    KisAbstractColorSpace(KisID("CMYK", i18n("CMYK")), TYPE_CMYK_8, icSigCmykData)
{
    m_channels.push_back(new KisChannelInfo(i18n("Cyan"), 0, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Magenta"), 1, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Yellow"), 2, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Black"), 3, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), 4, ALPHA));

    if (profileCount() == 0) {
        kdDebug(DBG_AREA_CMS) << "No profiles loaded!\n";
        return;
    }

    setDefaultProfile(  getProfileByName("Adobe CMYK") ); // XXX: Do not i18n -- this is from a data file
    if (getDefaultProfile() == 0) {
        kdDebug(DBG_AREA_CMS) << "No Adobe CMYK!\n";
        if (profileCount() != 0) {
            setDefaultProfile(  profiles()[0] );
        }
    }

    if (getDefaultProfile() == 0) {
        kdDebug(DBG_AREA_CMS) << "No default CMYK profile; CMYK will not work!\n";
        return;
    }

    // Create the default transforms from and to a QColor. Use the
    // display profile if there's one, otherwise a generic sRGB profile
    // XXX: For now, always use the generic sRGB profile.

    cmsHPROFILE hsRGB = cmsCreate_sRGBProfile();
    cmsHPROFILE hsCMYK = getDefaultProfile()->profile();

    m_defaultFromRGB = cmsCreateTransform(hsRGB, TYPE_BGR_8,
                          hsCMYK, TYPE_CMYK_8,
                          INTENT_PERCEPTUAL, 0);

    m_defaultToRGB =  cmsCreateTransform(hsCMYK, TYPE_CMYK_8,
                         hsRGB, TYPE_BGR_8,
                         INTENT_PERCEPTUAL, 0);

    // Default pixel buffer for QColor conversion
    m_qcolordata = new Q_UINT8[3];
    Q_CHECK_PTR(m_qcolordata);

}

KisCmykColorSpace::~KisCmykColorSpace()
{
    // XXX: These deletes cause a crash, but since the color strategy is a singleton
    //      that's only deleted at application close, it's no big deal.
    // delete [] m_qcolordata;
    //cmsDeleteTransform(m_defaultToRGB);
    //cmsDeleteTransform(m_defaultFromRGB);
}

void KisCmykColorSpace::nativeColor(const QColor& color, Q_UINT8 *dst, KisProfileSP profile)
{
    m_qcolordata[2] = color.red();
    m_qcolordata[1] = color.green();
    m_qcolordata[0] = color.blue();

    cmsDoTransform(m_defaultFromRGB, m_qcolordata, dst, 1);
    dst[4] = OPACITY_OPAQUE;
}

void KisCmykColorSpace::nativeColor(const QColor& color, QUANTUM opacity, Q_UINT8 *dst, KisProfileSP profile)
{
    m_qcolordata[2] = color.red();
    m_qcolordata[1] = color.green();
    m_qcolordata[0] = color.blue();

    cmsDoTransform(m_defaultFromRGB, m_qcolordata, dst, 1);
    dst[4] = opacity;
}

void KisCmykColorSpace::getAlpha(const Q_UINT8 *pixel, Q_UINT8 *alpha)
{
    *alpha = pixel[4];
}

void KisCmykColorSpace::setAlpha(Q_UINT8 *pixels, Q_UINT8 alpha, Q_INT32 nPixels)
{
    while (nPixels > 0) {
        pixels[4] = alpha;
        --nPixels;
        pixels += cmyk::MAX_CHANNEL_CMYKA;
    }
}

void KisCmykColorSpace::toQColor(const Q_UINT8 *src, QColor *c, KisProfileSP profile)
{
    cmsDoTransform(m_defaultToRGB, const_cast <Q_UINT8 *>(src), m_qcolordata, 1);
    c -> setRgb(m_qcolordata[2], m_qcolordata[1], m_qcolordata[0]);
}

void KisCmykColorSpace::toQColor(const Q_UINT8 *src, QColor *c, QUANTUM *opacity, KisProfileSP profile)
{
    cmsDoTransform(m_defaultToRGB, const_cast <Q_UINT8 *>(src), m_qcolordata, 1);
    c -> setRgb(m_qcolordata[2], m_qcolordata[1], m_qcolordata[0]);

     *opacity = src[4];
}

void KisCmykColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
}

vKisChannelInfoSP KisCmykColorSpace::channels() const
{
    return m_channels;
}

bool KisCmykColorSpace::hasAlpha() const
{
    return true;
}

Q_INT32 KisCmykColorSpace::nChannels() const
{
    return cmyk::MAX_CHANNEL_CMYKA;
}

Q_INT32 KisCmykColorSpace::nColorChannels() const
{
    return cmyk::MAX_CHANNEL_CMYK;
}

Q_INT32 KisCmykColorSpace::pixelSize() const
{
    return cmyk::MAX_CHANNEL_CMYKA;
}

QImage KisCmykColorSpace::convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
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
                    const_cast<Q_UINT8 *>(&(data[cmyk::MAX_CHANNEL_CMYKA*(i*width+j)])),
                    &(img.scanLine(i)[j*img.bytesPerLine()/width]), 1);
     }
     else {
         // Do a nice calibrated conversion
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                convertPixelsTo(const_cast<Q_UINT8 *>
                        (&(data[cmyk::MAX_CHANNEL_CMYKA*(i*width+j)])),
                        srcProfile,
                        &(img.scanLine(i)[j*img.bytesPerLine()/width]),
                        dstCS, dstProfile, 1, renderingIntent);
     }

    return img;
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
            QUANTUM opacity,
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
        d = dst;
        s = src;

        while (rows-- > 0) {
            memcpy(d, s, linesize);
            d += dstRowStride;
            s += srcRowStride;
        }
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
                         QUANTUM opacity)
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

QString KisCmykColorSpace::channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    Q_UINT32 channelPosition = m_channels[channelIndex] -> pos();

    return QString().setNum(pixel[channelPosition]);
}

QString KisCmykColorSpace::normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    Q_UINT32 channelPosition = m_channels[channelIndex] -> pos();

    return QString().setNum(static_cast<float>(pixel[channelPosition]) / UINT8_MAX);
}

