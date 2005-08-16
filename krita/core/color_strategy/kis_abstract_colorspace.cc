/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
#include <qdir.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kinstance.h>

#include "kis_abstract_colorspace.h"
#include "kis_pixel.h"
#include "kis_global.h"
#include "kis_factory.h"
#include "kis_profile.h"
#include "kis_config.h"
#include "kis_id.h"
#include "kis_integer_maths.h"
#include "kis_color_conversions.h"
#include "kis_colorspace_registry.h"
#include "kis_channelinfo.h"

namespace {

    static KisProfileSP lastUsedSrcProfile;
    static KisProfileSP lastUsedDstProfile;
    static cmsHTRANSFORM lastUsedTransform = 0;

    int simpleAdjust(int channel, int brightness, double contrast) {

        int nd = channel + brightness;
        nd = (int)(((nd - QUANTUM_MAX / 2 ) * contrast) + QUANTUM_MAX / 2);
        return QMAX( 0, QMIN( QUANTUM_MAX, nd ) );

    }

}

KisAbstractColorSpace::KisAbstractColorSpace(const KisID& id, DWORD cmType, icColorSpaceSignature colorSpaceSignature)
    : m_id(id),
      m_cmType(cmType),
      m_colorSpaceSignature(colorSpaceSignature)
{
    // Load all profiles that are suitable for this colorspace signature
    m_alphaPos = -1;
    m_alphaSize = -1;

    // Default pixel buffer for QColor conversion
    m_qcolordata = new Q_UINT8[3];
    Q_CHECK_PTR(m_qcolordata);

    m_conversionCache = 0;
    m_cachesize = 0;
    
}

KisAbstractColorSpace::~KisAbstractColorSpace()
{
    TransformMap::iterator it;
    for ( it = m_transforms.begin(); it != m_transforms.end(); ++it ) {
        cmsDeleteTransform(it.data());
        }
    m_transforms.clear();
}

void KisAbstractColorSpace::nativeColor(const QColor& color, Q_UINT8 *dst, KisProfileSP /*profile*/)
{
    if (!m_defaultFromRGB) return; 

        m_qcolordata[2] = color.red();
        m_qcolordata[1] = color.green();
        m_qcolordata[0] = color.blue();

        // XXX: Use proper conversion from RGB with profiles
        cmsDoTransform(m_defaultFromRGB, m_qcolordata, dst, 1);
        dst[4] = OPACITY_OPAQUE;

}

void KisAbstractColorSpace::nativeColor(const QColor& color, QUANTUM opacity, Q_UINT8 *dst, KisProfileSP /*profile*/)
{
    if (!m_defaultFromRGB) return; 

        m_qcolordata[2] = color.red();
        m_qcolordata[1] = color.green();
        m_qcolordata[0] = color.blue();

        // XXX: Use proper conversion from RGB with profiles
        cmsDoTransform(m_defaultFromRGB, m_qcolordata, dst, 1);
        dst[4] = opacity;
}

void KisAbstractColorSpace::toQColor(const Q_UINT8 *src, QColor *c, KisProfileSP /*profile*/)
{
    if (!m_defaultToRGB) return;

        // XXX: Properly convert using the rgb colorspace and the profile
        cmsDoTransform(m_defaultToRGB, const_cast <Q_UINT8 *>(src), m_qcolordata, 1);
        c -> setRgb(m_qcolordata[2], m_qcolordata[1], m_qcolordata[0]);
}

void KisAbstractColorSpace::toQColor(const Q_UINT8 *src, QColor *c, QUANTUM *opacity, KisProfileSP /*profile*/)
{
    if (!m_defaultToRGB) return;

        // XXX: Properly convert using the rgb colorspace and the profile
        cmsDoTransform(m_defaultToRGB, const_cast <Q_UINT8 *>(src), m_qcolordata, 1);
        c -> setRgb(m_qcolordata[2], m_qcolordata[1], m_qcolordata[0]);

        *opacity = src[4];
}

bool KisAbstractColorSpace::convertTo(KisPixel& src, KisPixel& dst, Q_INT32 renderingIntent)
{
    return convertPixelsTo(src.channels(), src.profile(),
                   dst.channels(), dst.colorStrategy(), dst.profile(),
                   renderingIntent);
}

bool KisAbstractColorSpace::convertPixelsTo(const Q_UINT8 * src, KisProfileSP srcProfile,
                        Q_UINT8 * dst, KisAbstractColorSpace * dstColorStrategy, KisProfileSP dstProfile,
                        Q_UINT32 numPixels,
                        Q_INT32 renderingIntent)
{

//     kdDebug() << "src space: " << id().name() << ", src profile " << srcProfile->productName()
//             << ", dst spcae: " << dstColorStrategy->id().name() << ", dst profile " << dstProfile->productName() << "\n";

    cmsHTRANSFORM tf = 0;

    Q_INT32 srcPixelSize = pixelSize();
    Q_INT32 dstPixelSize = dstColorStrategy -> pixelSize();

    if (!srcProfile) {
        srcProfile = getDefaultProfile();
    }

    if (!dstProfile) {
        dstProfile = dstColorStrategy->getDefaultProfile();
    }

    if (lastUsedTransform != 0) {
        if (dstProfile == lastUsedDstProfile && srcProfile == lastUsedSrcProfile)
            tf = lastUsedTransform;
    }

    if (!tf && srcProfile && dstProfile) {
    
        if (!m_transforms.contains(KisProfilePair(srcProfile, dstProfile))) {
            tf = createTransform(dstColorStrategy,
                         srcProfile,
                         dstProfile,
                         renderingIntent);
            if (tf) {
                m_transforms[KisProfilePair(srcProfile, dstProfile)] = tf;
            }
        }
        else {
            tf = m_transforms[KisProfilePair(srcProfile, dstProfile)];
        }

        lastUsedTransform = tf;
        lastUsedSrcProfile = srcProfile;
        lastUsedDstProfile = dstProfile;
    }

    if (tf) {
            
        cmsDoTransform(tf, const_cast<Q_UINT8 *>(src), dst, numPixels);

        if (dstColorStrategy -> hasAlpha())
        {
            // Lcms does nothing to the destination alpha channel so we must convert that manually.
            while (numPixels > 0) {
                Q_UINT8 alpha;
                getAlpha(src, &alpha);
                dstColorStrategy -> setAlpha(dst, alpha, 1);

                src += srcPixelSize;
                dst += dstPixelSize;
                numPixels--;
            }
        }

        return true;
    }
    
    // Couldn't get a profile. Use QColor -- this is okay here, because even if we were to use KisColor,
    // we still wouldn't be able to get a transform. That's why we're here...
    while (numPixels > 0) {
        QColor color;
        QUANTUM opacity;

        toQColor(src, &color, &opacity);
        dstColorStrategy -> nativeColor(color, opacity, dst);

        src += srcPixelSize;
        dst += dstPixelSize;
        numPixels--;
    }

    return true;
}


void KisAbstractColorSpace::setAlpha(Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels)
{

    Q_INT32 psize = pixelSize();
    
    if (m_alphaSize == -1 && m_alphaPos == -1) {
        m_alphaPos = 0;
        m_alphaSize = -1;
        
        vKisChannelInfoSP_cit it;
        for (it = channels().begin(); it != channels().end(); ++it) {
            if ((*it)->channelType() == ALPHA) {
                m_alphaSize = (*it)->size();
                break;
            }
            ++m_alphaPos;
        }
    }

    if (m_alphaSize == -1) {
        m_alphaPos = -1;
        return;
    }
    
    for (Q_INT32 i = 0; i < nPixels; ++i) {
        // XXX: Downscale for now.
        pixels[(i * psize) + m_alphaPos] = alpha;
    }
}

KisPixelOp * KisAbstractColorSpace::getPixelOp(const KisID & id)
{
    return 0;
}



void KisAbstractColorSpace::applyAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels)
{
    Q_INT32 psize = pixelSize();
    
    while (nPixels--) {
    
        // XXX: Take care -- in u16 or higher, we should upcast the alpha value!
        pixels[m_alphaPos] = UINT8_MULT(*(pixels + m_alphaPos) , *alpha);
        
        pixels += psize;
        ++alpha;
    }
}

void KisAbstractColorSpace::applyInverseAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels)
{
    Q_INT32 psize = pixelSize();
    
    while(--nPixels) {
    
            Q_UINT16 p_alpha, s_alpha;

            p_alpha = *(pixels + m_alphaPos);
            s_alpha = MAX_SELECTED - *alpha;

	    kdDebug() << "p_alpha: " << p_alpha << ", s_alpha " << s_alpha << "\n";

            // XXX: Take care -- in u16 or higher, we should upcast the alpha value!
            pixels[m_alphaPos] = UINT8_MULT(p_alpha, s_alpha);

            pixels += psize;
            ++alpha;
    }
}

KisColorAdjustment *KisAbstractColorSpace::createBrightnessContrastAdjustment(Q_UINT16 */*transferValues*/)
{
    return NULL;
}

void KisAbstractColorSpace::applyAdjustment(const Q_UINT8 *src, Q_UINT8 *dst, KisColorAdjustment *adj, Q_INT32 nPixels)
{
}


// BC: should this default be HSV-based?
Q_INT8 KisAbstractColorSpace::difference(const Q_UINT8* src1, const Q_UINT8* src2)
{
    QColor color1, color2;
    toQColor(src1, &color1);
    toQColor(src2, &color2);

    int h1, s1, v1, h2, s2, v2;
    rgb_to_hsv(color1.red(), color1.green(), color1.blue(), &h1, &s1, &v1);
    rgb_to_hsv(color2.red(), color2.green(), color2.blue(), &h2, &s2, &v2);

    return QMAX(QABS(v1 - v2), QMAX(QABS(s1 - s2), QABS(h1 - h2)));
}

void KisAbstractColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
    Q_UINT32 totalRed = 0, totalGreen = 0, totalBlue = 0, newAlpha = 0;

    QColor c;
    Q_UINT8 opacity;
    
    while (nColors--)
    {
        // Ugly hack to get around the current constness mess of the colour strategy...
        const_cast<KisAbstractColorSpace *>(this) -> toQColor(*colors, &c, &opacity);
        
        Q_UINT32 alphaTimesWeight = UINT8_MULT(opacity, *weights);

        totalRed += c.red() * alphaTimesWeight;
        totalGreen += c.green() * alphaTimesWeight;
        totalBlue += c.blue() * alphaTimesWeight;
        newAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    Q_ASSERT(newAlpha <= 255);

    if (newAlpha > 0) {
        totalRed = UINT8_DIVIDE(totalRed, newAlpha);
        totalGreen = UINT8_DIVIDE(totalGreen, newAlpha);
        totalBlue = UINT8_DIVIDE(totalBlue, newAlpha);
    }

    // Divide by 255.
    totalRed += 0x80;
    
    Q_UINT32 dstRed = ((totalRed >> 8) + totalRed) >> 8;
    Q_ASSERT(dstRed <= 255);

    totalGreen += 0x80;
    Q_UINT32 dstGreen = ((totalGreen >> 8) + totalGreen) >> 8;
    Q_ASSERT(dstGreen <= 255);

    totalBlue += 0x80;
    Q_UINT32 dstBlue = ((totalBlue >> 8) + totalBlue) >> 8;
    Q_ASSERT(dstBlue <= 255);

    const_cast<KisAbstractColorSpace *>(this) -> nativeColor(QColor(dstRed, dstGreen, dstBlue), newAlpha, dst);
}

void KisAbstractColorSpace::convolveColors(Q_UINT8** colors, Q_INT32 * kernelValues, enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nColors) const
{
    Q_INT32 totalRed = 0, totalGreen = 0, totalBlue = 0, totalAlpha = 0;

    QColor dstColor;
    Q_UINT8 dstOpacity;
    
    const_cast<KisAbstractColorSpace *>(this)->toQColor(dst, &dstColor, &dstOpacity);
    
    while (nColors--)
    {
        Q_INT32 weight = *kernelValues;
        
        if (weight != 0) {
            QColor c;
            Q_UINT8 opacity;
            const_cast<KisAbstractColorSpace *>(this)->toQColor( *colors, &c, &opacity );
            totalRed += c.red() * weight;
            totalGreen += c.green() * weight;
            totalBlue += c.blue() * weight;
            totalAlpha += opacity * weight;
        }
        colors++;
        kernelValues++;
    }


    if (channelFlags & FLAG_COLOR) {
        const_cast<KisAbstractColorSpace *>(this)->nativeColor(QColor(CLAMP((totalRed / factor) + offset, 0, QUANTUM_MAX),
                                        CLAMP((totalGreen / factor) + offset, 0, QUANTUM_MAX),
                                        CLAMP((totalBlue / factor) + offset, 0, QUANTUM_MAX)),
            dstOpacity,
            dst);
    }
    if (channelFlags & FLAG_ALPHA) {
        const_cast<KisAbstractColorSpace *>(this)->nativeColor(dstColor, CLAMP((totalAlpha/ factor) + offset, 0, QUANTUM_MAX), dst);
    }

}

void KisAbstractColorSpace::darken(const Q_UINT8 * src, Q_UINT8 * dst, Q_INT32 shade, bool compensate, double compensation, Q_INT32 nPixels) const
{
    QColor c;
    Q_INT32 psize = pixelSize();
    
    for (int i = 0; i < nPixels; ++i) {
        
        const_cast<KisAbstractColorSpace *>(this) -> toQColor(src + (i * psize), &c);
        Q_INT32 r, g, b;
        
        if (compensate) {
            r = (Q_INT32) QMIN(255, ((c.red() * shade) / (compensation * 255)));
            g = (Q_INT32) QMIN(255, ((c.green() * shade) / (compensation * 255)));
            b = (Q_INT32) QMIN(255, ((c.blue() * shade) / (compensation * 255)));
        }
        else {
            r = (Q_INT32) QMIN(255, (c.red() * shade / 255));
            g = (Q_INT32) QMIN(255, (c.green() * shade / 255));
            b = (Q_INT32) QMIN(255, (c.blue() * shade / 255));
        }
        c.setRgb(r, g, b);
    
        const_cast<KisAbstractColorSpace *>(this)->nativeColor( c, dst  + (i * psize));
    }
}

Q_UINT8 KisAbstractColorSpace::intensity8(const Q_UINT8 * src) const
{
    QColor c;
        QUANTUM opacity;
        const_cast<KisAbstractColorSpace *>(this)->toQColor(src, &c, &opacity);
        return (Q_UINT8)(c.red() * 0.30 + c.green() * 0.59 + c.blue() * 0.11) + 0.5;

}


void KisAbstractColorSpace::bitBlt(Q_UINT8 *dst,
                   Q_INT32 dststride,
                   KisAbstractColorSpace * srcSpace,
                   const Q_UINT8 *src,
                   Q_INT32 srcRowStride,
                   const Q_UINT8 *srcAlphaMask,
                   Q_INT32 maskRowStride,
                   QUANTUM opacity,
                   Q_INT32 rows,
                   Q_INT32 cols,
                   const KisCompositeOp& op,
                   KisProfileSP srcProfile,
                   KisProfileSP dstProfile)
{
    if (rows <= 0 || cols <= 0)
        return;

    if (m_id!= srcSpace -> id()) {
        int len = pixelSize() * rows * cols;

	// If our conversion cache is too small, extend it.
        if (len > m_cachesize) {
		delete m_conversionCache;
		m_conversionCache = new Q_UINT8[len];
		memset(m_conversionCache, 0, len);
		Q_CHECK_PTR(m_conversionCache);
		m_cachesize = len;
	}

        if (srcProfile && dstProfile) {
            for (Q_INT32 row = 0; row < rows; row++) {
                srcSpace -> convertPixelsTo(src + row * srcRowStride, srcProfile,
                                m_conversionCache + row * cols * pixelSize(), this, dstProfile,
                                cols);
            }
        }
        else {
            for (Q_INT32 row = 0; row < rows; row++) {
                srcSpace -> convertPixelsTo(src + row * srcRowStride, 0,
                                m_conversionCache + row * cols * pixelSize(), this, 0,
                                cols);
            }
        }
         srcRowStride = cols * pixelSize();

        bitBlt(dst,
               dststride,
               m_conversionCache,
               srcRowStride,
               srcAlphaMask,
               maskRowStride,
               opacity,
               rows,
               cols,
               op);

    }
    else {
        bitBlt(dst,
               dststride,
               src,
               srcRowStride,
               srcAlphaMask,
               maskRowStride,
               opacity,
               rows,
               cols,
               op);
    }
}

vKisProfileSP KisAbstractColorSpace::profiles()
{
    return KisColorSpaceRegistry::instance()->profilesFor( this );
}

Q_INT32 KisAbstractColorSpace::profileCount()
{
    return KisColorSpaceRegistry::instance()->profilesFor( this ).size();
}


cmsHTRANSFORM KisAbstractColorSpace::createTransform(KisAbstractColorSpace * dstColorStrategy,
                             KisProfileSP srcProfile,
                             KisProfileSP dstProfile,
                             Q_INT32 renderingIntent)
{
    KisConfig cfg;
    int flags = 0;

    if (cfg.useBlackPointCompensation()) {
        flags = cmsFLAGS_BLACKPOINTCOMPENSATION;
    }

    if (dstProfile && srcProfile ) {
        cmsHTRANSFORM tf = cmsCreateTransform(srcProfile -> profile(),
                              colorSpaceType(),
                              dstProfile -> profile(),
                              dstColorStrategy -> colorSpaceType(),
                              renderingIntent,
                              flags);

        return tf;
    }
    return 0;
}

