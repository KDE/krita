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

#include "kis_strategy_colorspace.h"
#include "kis_pixel.h"
#include "kis_global.h"
#include "kis_factory.h"
#include "kis_profile.h"
#include "kis_config.h"
#include "kis_id.h"
#include "kis_integer_maths.h"
#include "kis_color_conversions.h"

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

KisStrategyColorSpace::KisStrategyColorSpace(const KisID& id, Q_UINT32 cmType, icColorSpaceSignature colorSpaceSignature)
	: m_id(id),
	  m_cmType(cmType),
	  m_colorSpaceSignature(colorSpaceSignature)
{
	// Load all profiles that are suitable for this colorspace signature
	resetProfiles();
	m_alphaPos = -1;
	m_alphaSize = -1;
}

KisStrategyColorSpace::~KisStrategyColorSpace()
{
	TransformMap::iterator it;
	for ( it = m_transforms.begin(); it != m_transforms.end(); ++it ) {
		cmsDeleteTransform(it.data());
        }
	m_transforms.clear();
}

bool KisStrategyColorSpace::convertTo(KisPixel& src, KisPixel& dst, Q_INT32 renderingIntent)
{
	return convertPixelsTo(src.channels(), src.profile(),
			       dst.channels(), dst.colorStrategy(), dst.profile(),
			       renderingIntent);
}

bool KisStrategyColorSpace::convertPixelsTo(const Q_UINT8 * src, KisProfileSP srcProfile,
					    Q_UINT8 * dst, KisStrategyColorSpace * dstColorStrategy, KisProfileSP dstProfile,
					    Q_UINT32 numPixels,
					    Q_INT32 renderingIntent)
{
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

KisColorAdjustment *KisStrategyColorSpace::createBrightnessContrastAdjustment(Q_UINT16 *transferValues)
{
	return NULL;
}

void KisStrategyColorSpace::applyAdjustment(const Q_UINT8 *src, Q_UINT8 *dst, KisColorAdjustment *adj, Q_INT32 nPixels)
{
}


// BC: should this default be HSV-based?
Q_INT8 KisStrategyColorSpace::difference(const Q_UINT8* src1, const Q_UINT8* src2)
{
	QColor color1, color2;
	toQColor(src1, &color1);
	toQColor(src2, &color2);

	int h1, s1, v1, h2, s2, v2;
	rgb_to_hsv(color1.red(), color1.green(), color1.blue(), &h1, &s1, &v1);
	rgb_to_hsv(color2.red(), color2.green(), color2.blue(), &h2, &s2, &v2);

	return QMAX(QABS(v1 - v2), QMAX(QABS(s1 - s2), QABS(h1 - h2)));
}

void KisStrategyColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
	Q_UINT32 totalRed = 0, totalGreen = 0, totalBlue = 0, newAlpha = 0;

	QColor c;
	Q_UINT8 opacity;
	
	while (nColors--)
	{
		// Ugly hack to get around the current constness mess of the colour strategy...
		const_cast<KisStrategyColorSpace *>(this) -> toQColor(*colors, &c, &opacity);
		
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

	const_cast<KisStrategyColorSpace *>(this) -> nativeColor(QColor(dstRed, dstGreen, dstBlue), newAlpha, dst);
}

void KisStrategyColorSpace::convolveColors(Q_UINT8** colors, Q_INT32 * kernelValues, enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nColors) const
{
}

void KisStrategyColorSpace::darken(const Q_UINT8 * src, Q_UINT8 * dst, Q_INT32 shade, bool compensate, double compensation, Q_INT32 nPixels) const
{
	QColor c;
	Q_INT32 psize = pixelSize();
	
	for (int i = 0; i < nPixels; ++i) {
		
		const_cast<KisStrategyColorSpace *>(this) -> toQColor(src + (i * psize), &c);
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
	
		const_cast<KisStrategyColorSpace *>(this)->nativeColor( c, dst  + (i * psize));
	}
}

Q_UINT8 KisStrategyColorSpace::intensity8(const Q_UINT8 * src) const
{
	QColor c;
        QUANTUM opacity;
        const_cast<KisStrategyColorSpace *>(this)->toQColor(src, &c, &opacity);
        return (Q_UINT8)(c.red() * 0.30 + c.green() * 0.59 + c.blue() * 0.11) + 0.5;

}


void KisStrategyColorSpace::bitBlt(Q_UINT8 *dst,
				   Q_INT32 dststride,
				   KisStrategyColorSpace * srcSpace,
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
		Q_UINT8 * convertedSrcPixels = new Q_UINT8[len];
		Q_CHECK_PTR(convertedSrcPixels);

		memset(convertedSrcPixels, 0, len * sizeof(Q_UINT8));

		if (srcProfile && dstProfile) {
			for (Q_INT32 row = 0; row < rows; row++) {
				srcSpace -> convertPixelsTo(src + row * srcRowStride, srcProfile,
							    convertedSrcPixels + row * cols * pixelSize(), this, dstProfile,
							    cols);
			}
		}
		else {
			for (Q_INT32 row = 0; row < rows; row++) {
				srcSpace -> convertPixelsTo(src + row * srcRowStride, 0,
							    convertedSrcPixels + row * cols * pixelSize(), this, 0,
							    cols);
			}
		}
 		srcRowStride = cols * pixelSize();

		bitBlt(dst,
		       dststride,
		       convertedSrcPixels,
		       srcRowStride,
		       srcAlphaMask,
		       maskRowStride,
		       opacity,
		       rows,
		       cols,
		       op);

 		delete[] convertedSrcPixels;
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

void KisStrategyColorSpace::resetProfiles()
{
	// XXX: Should find a way to make sure not all profiles are read for all color strategies
	m_profiles.clear();
	m_profileFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_profiles", "*.icm");
	m_profileFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_profiles", "*.ICM");
	m_profileFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_profiles", "*.ICC");
	m_profileFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_profiles", "*.icc");

	if (!m_profileFilenames.empty()) {
		KisProfile * profile = 0;
		for ( QStringList::Iterator it = m_profileFilenames.begin(); it != m_profileFilenames.end(); ++it ) {

			profile = new KisProfile(*it, colorSpaceType());
			Q_CHECK_PTR(profile);

			profile -> load();
			if (profile -> valid() && profile -> colorSpaceSignature() == m_colorSpaceSignature) {

				m_profiles.push_back(profile);
			}
		}
	}

	// XXX: Make configurable; make flexible
	QDir d("/usr/share/color/icc/", "*.icc");
	m_profileFilenames += d.entryList();
	if (!m_profileFilenames.empty()) {
		KisProfile * profile = 0;
		for ( QStringList::Iterator it = m_profileFilenames.begin(); it != m_profileFilenames.end(); ++it ) {


			profile = new KisProfile(d.filePath(*it), colorSpaceType());
			Q_CHECK_PTR(profile);

			profile -> load();
			if (profile -> valid() && profile -> colorSpaceSignature() == m_colorSpaceSignature) {

				m_profiles.push_back(profile);
			}
		}
	}
}

KisProfileSP KisStrategyColorSpace::getProfileByName(const QString & name)
{
	vKisProfileSP::iterator it;

	for ( it = m_profiles.begin(); it != m_profiles.end(); ++it ) {
		if ((*it) -> productName() == name) {
			return *it;
		}
	}
	return 0;

}

cmsHTRANSFORM KisStrategyColorSpace::createTransform(KisStrategyColorSpace * dstColorStrategy,
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

void KisStrategyColorSpace::setAlpha(Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels)
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
