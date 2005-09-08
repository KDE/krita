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


KisAbstractColorSpace::KisAbstractColorSpace(const KisID& id, DWORD cmType, icColorSpaceSignature colorSpaceSignature)
    : m_id(id),
      m_cmType(cmType),
      m_colorSpaceSignature(colorSpaceSignature)
{
    m_alphaPos = -1;
    m_alphaSize = -1;
    m_qcolordata = 0;
    m_lastUsedDstProfile = 0;
    m_lastUsedSrcProfile = 0;
    m_lastUsedTransform = 0;

}

void KisAbstractColorSpace::init()
{
    // Only create the defaul transforms if
    if (m_defaultProfile == 0) return;


    // Default pixel buffer for QColor conversion
    m_qcolordata = new Q_UINT8[3];
    Q_CHECK_PTR(m_qcolordata);

    cmsHPROFILE hProfile = m_defaultProfile->profile();

    if (m_cmType != TYPE_BGR_8) {
        // For conversions from default rgb
        cmsHPROFILE hsRGB = cmsCreate_sRGBProfile();

        m_defaultFromRGB = cmsCreateTransform(hsRGB, TYPE_BGR_8,
                                              hProfile, m_cmType,
                                              INTENT_PERCEPTUAL, 0);

        m_defaultToRGB =  cmsCreateTransform(hProfile, m_cmType,
                                             hsRGB, TYPE_BGR_8,
                                             INTENT_PERCEPTUAL, 0);
    }

    if (m_cmType != TYPE_XYZ_16) {
    	// For conversion from default 16 bit xyz for default pixel ops
    	cmsHPROFILE hsXYZ = cmsCreateXYZProfile();

	    m_defaultFromXYZ = cmsCreateTransform(hsXYZ, TYPE_XYZ_16,
		    	                		      hProfile, m_cmType,
                			    		      INTENT_PERCEPTUAL, 0);

      	m_defaultToXYZ = cmsCreateTransform(hProfile, m_cmType,
	                    				    hsXYZ, TYPE_XYZ_16,
					                        INTENT_PERCEPTUAL, 0);

    }
}

KisAbstractColorSpace::~KisAbstractColorSpace()
{
}


bool KisAbstractColorSpace::convertTo(KisPixel& src, KisPixel& dst, Q_INT32 renderingIntent)
{
    return convertPixelsTo(src.channels(), src.profile(),
			   dst.channels(), dst.colorSpace(), dst.profile(),
			   renderingIntent);
}

bool KisAbstractColorSpace::convertPixelsTo(const Q_UINT8 * src, KisProfileSP srcProfile,
					    Q_UINT8 * dst, KisAbstractColorSpace * dstColorSpace, KisProfileSP dstProfile,
					    Q_UINT32 numPixels,
					    Q_INT32 renderingIntent)
{

    cmsHTRANSFORM tf = 0;

    Q_INT32 srcPixelSize = pixelSize();
    Q_INT32 dstPixelSize = dstColorSpace -> pixelSize();

    if (!srcProfile) {
        srcProfile = getDefaultProfile();
    }

    if (!dstProfile) {
        dstProfile = dstColorSpace->getDefaultProfile();
    }

//    kdDebug() << "src space: " << id().name() << ", src profile " << srcProfile->productName()
//              << ", dst space: " << dstColorSpace->id().name() << ", dst profile " << dstProfile->productName()
//              << ", number of pixels: " << numPixels << "\n";

    if (m_lastUsedTransform != 0) {
        if (dstProfile == m_lastUsedDstProfile && srcProfile == m_lastUsedSrcProfile)
            tf = m_lastUsedTransform;
    }

    if (!tf && srcProfile && dstProfile) {

        if (!m_transforms.contains(KisProfilePair(srcProfile, dstProfile))) {
            tf = createTransform(dstColorSpace,
                         srcProfile,
                         dstProfile,
                         renderingIntent);
            if (tf) {
//                  kdDebug() << "Going to add transform to cache "
//                            << " srcprofile: " << srcProfile->productName()
//                            << " dstProfile " << dstProfile->productName() << "\n";

                 m_transforms[KisProfilePair(srcProfile, dstProfile)] = tf;
            }
        }
        else {
            tf = m_transforms[KisProfilePair(srcProfile, dstProfile)];
        }

        if ( tf ) {
            m_lastUsedTransform = tf;
            m_lastUsedSrcProfile = srcProfile;
            m_lastUsedDstProfile = dstProfile;
        }
    }

    if (tf) {

        cmsDoTransform(tf, const_cast<Q_UINT8 *>(src), dst, numPixels);

        if (dstColorSpace -> hasAlpha())
        {
            // Lcms does nothing to the destination alpha channel so we must convert that manually.
            while (numPixels > 0) {
                Q_UINT8 alpha = getAlpha(src);
                dstColorSpace -> setAlpha(dst, alpha, 1);

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
        dstColorSpace -> fromQColor(color, opacity, dst);

        src += srcPixelSize;
        dst += dstPixelSize;
        numPixels--;
    }

    return true;
}


KisColorAdjustment *KisAbstractColorSpace::createBrightnessContrastAdjustment(Q_UINT16 */*transferValues*/)
{
    return NULL;
}

void KisAbstractColorSpace::applyAdjustment(const Q_UINT8 */*src*/, Q_UINT8 */*dst*/, KisColorAdjustment */*adj*/, Q_INT32 /*nPixels*/)
{

}


// BC: should this default be HSV-based?
Q_INT8 KisAbstractColorSpace::difference(const Q_UINT8* src1, const Q_UINT8* src2)
{
    if ( m_defaultToXYZ != 0 && m_defaultFromXYZ != 0 ) {
        Q_INT32 psize = pixelSize();


        if ( m_conversionCache.size() < 2 * psize ) {
            m_conversionCache.resize( 2 * psize, QGArray::SpeedOptim );
        }

        cmsDoTransform( m_defaultToXYZ, const_cast<Q_UINT8*>( src1 ), m_conversionCache.data(), 1);
        cmsDoTransform( m_defaultToXYZ, const_cast<Q_UINT8*>( src2 ), m_conversionCache.data() + psize, 1);

        return KisColorSpaceRegistry::getXYZ16()->difference( m_conversionCache.data(), m_conversionCache.data() + psize );

    }
    else {
        QColor color1, color2;
        toQColor(src1, &color1);
        toQColor(src2, &color2);

        int h1, s1, v1, h2, s2, v2;
        rgb_to_hsv(color1.red(), color1.green(), color1.blue(), &h1, &s1, &v1);
        rgb_to_hsv(color2.red(), color2.green(), color2.blue(), &h2, &s2, &v2);

        return QMAX(QABS(v1 - v2), QMAX(QABS(s1 - s2), QABS(h1 - h2)));
    }
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

    const_cast<KisAbstractColorSpace *>(this) -> fromQColor(QColor(dstRed, dstGreen, dstBlue), newAlpha, dst);
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
        const_cast<KisAbstractColorSpace *>(this)->fromQColor(QColor(CLAMP((totalRed / factor) + offset, 0, QUANTUM_MAX),
                                        CLAMP((totalGreen / factor) + offset, 0, QUANTUM_MAX),
                                        CLAMP((totalBlue / factor) + offset, 0, QUANTUM_MAX)),
            dstOpacity,
            dst);
    }
    if (channelFlags & FLAG_ALPHA) {
        const_cast<KisAbstractColorSpace *>(this)->fromQColor(dstColor, CLAMP((totalAlpha/ factor) + offset, 0, QUANTUM_MAX), dst);
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

        const_cast<KisAbstractColorSpace *>(this)->fromQColor( c, dst  + (i * psize));
    }
}

Q_UINT8 KisAbstractColorSpace::intensity8(const Q_UINT8 * src) const
{
    QColor c;
        QUANTUM opacity;
        const_cast<KisAbstractColorSpace *>(this)->toQColor(src, &c, &opacity);
        return (Q_UINT8)((c.red() * 0.30 + c.green() * 0.59 + c.blue() * 0.11) + 0.5);

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
        Q_UINT32 len = pixelSize() * rows * cols;

        if ( !srcProfile ) srcProfile = srcSpace->getDefaultProfile();
        if ( !dstProfile ) dstProfile = getDefaultProfile();

        // If our conversion cache is too small, extend it.
        if (!m_conversionCache.resize( len, QGArray::SpeedOptim )) {
            kdDebug() << "Could not allocate enough memory for the conversion!\n";
            // XXX: We should do a slow, pixel by pixel bitblt here...
            return;
        }

        if (srcProfile && dstProfile) {
            for (Q_INT32 row = 0; row < rows; row++) {
                srcSpace -> convertPixelsTo(src + row * srcRowStride, srcProfile,
                                            m_conversionCache.data() + row * cols * pixelSize(), this, dstProfile,
                                            cols);
            }
        }
        else {
            for (Q_INT32 row = 0; row < rows; row++) {
                srcSpace -> convertPixelsTo(src + row * srcRowStride, 0,
                                            m_conversionCache.data() + row * cols * pixelSize(), this, 0,
                                            cols);
            }
        }


        // The old srcRowStride is no longer valid because we converted to the current cs
        srcRowStride = cols * pixelSize();

        bitBlt(dst,
               dststride,
               m_conversionCache.data(),
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

QImage KisAbstractColorSpace::convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                                              KisProfileSP srcProfile, KisProfileSP dstProfile,
                                              Q_INT32 renderingIntent, float /*exposure*/)

{
    QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
    img.setAlphaBuffer( true );

    KisAbstractColorSpace * dstCS = KisColorSpaceRegistry::instance() -> get("RGBA");

    if (! srcProfile && ! dstProfile) {
        srcProfile = getDefaultProfile();
        dstProfile = dstCS->getDefaultProfile();
    }

    convertPixelsTo(const_cast<Q_UINT8 *>(data), srcProfile,
                    img.bits(), dstCS, dstProfile,
                    width * height, renderingIntent);

    return img;
}


cmsHTRANSFORM KisAbstractColorSpace::createTransform(KisAbstractColorSpace * dstColorSpace,
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
                              dstColorSpace -> colorSpaceType(),
                              renderingIntent,
                              flags);

        return tf;
    }
    return 0;
}

