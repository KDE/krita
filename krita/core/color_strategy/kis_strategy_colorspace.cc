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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
					    Q_UINT8 * dst, KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile,
					    Q_UINT32 numPixels,
					    Q_INT32 renderingIntent)
{
  	kdDebug(DBG_AREA_CMS) << "convertPixels: src profile: " << srcProfile << ", dst profile: " << dstProfile << "\n";
	cmsHTRANSFORM tf = 0;

	if (srcProfile && dstProfile) {
		if (!m_transforms.contains(KisProfilePair(srcProfile, dstProfile))) {
 			kdDebug(DBG_AREA_CMS) << "Create new transform: src profile: " 
					      << srcProfile -> productName() 
					      << ", dst profile: " 
					      << dstProfile -> productName() << "\n";
			tf = createTransform(dstColorStrategy,
					     srcProfile,
					     dstProfile,
					     renderingIntent);
			if (tf == 0) 
				kdDebug(DBG_AREA_CMS) << "Creating transform failed! src profile: " << srcProfile << ", dst profile: " << dstProfile << "\n";
			m_transforms[KisProfilePair(srcProfile, dstProfile)] = tf;
		}
		else {
			tf = m_transforms[KisProfilePair(srcProfile, dstProfile)];
			if (tf == 0) 
				kdDebug(DBG_AREA_CMS) << "Retrieving cached transform failed! src profile: " << srcProfile << ", dst profile: " << dstProfile << "\n";
		}

		if (tf) {
			cmsDoTransform(tf, const_cast<Q_UINT8 *>(src), dst, numPixels);
			return true;
		}
		
		kdDebug(DBG_AREA_CMS) << "No transform from "
			  << srcProfile -> productName() << " to " << dstProfile -> productName()
			  << ", going to convert through RGB!\n";
	}

	Q_INT32 srcPixelSize = pixelSize();
	Q_INT32 dstPixelSize = dstColorStrategy -> pixelSize();

	while (numPixels > 0) {
		kdDebug(DBG_AREA_CMS) << "Falling back on conversion by qcolor\n";
		QColor color;
		QUANTUM opacity;

		toQColor(src, &color, &opacity);
		kdDebug(DBG_AREA_CMS) << "QColor created: " << color.red() << ", " << color.green() << ", " << color.blue() << "\n";
		dstColorStrategy -> nativeColor(color, opacity, dst);

		src += srcPixelSize;
		dst += dstPixelSize;
		numPixels--;
	}

	return true;
}

namespace {
	int simpleAdjust(int channel, int brightness, double contrast) {
	
		int nd = channel + brightness;
		nd = (int)(((nd - QUANTUM_MAX / 2 ) * contrast) + QUANTUM_MAX / 2);
		return QMAX( 0, QMIN( QUANTUM_MAX, nd ) );
		
	}
}
void KisStrategyColorSpace::adjustBrightnessContrast(const Q_UINT8 *src, Q_UINT8 *dst, Q_INT8 brightness, Q_INT8 contrast, Q_INT32 nPixels)
{
	if (brightness < -100) brightness = -100;
	if (brightness > 100) brightness = 100;
	if (contrast < -100) contrast = 100;
	if (contrast > 100) contrast = 100;

	int psize = pixelSize();


	QColor c1, c2;
	
	for (int i = 0; i < nPixels; ++i) {

		toQColor(src + (i * psize), &c1);

		double dblContrast = (100.0 + contrast) / 100;
		dblContrast *= dblContrast;

		// change the brightness
		c2.setRgb( simpleAdjust(c1.red(), brightness, dblContrast),
			   simpleAdjust(c1.green(), brightness, dblContrast),
			   simpleAdjust(c1.blue(), brightness, dblContrast));
		   
		nativeColor(c2, dst + (i * psize));
	}
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

void KisStrategyColorSpace::darken(const Q_UINT8 * src, Q_UINT8 * dst, Q_INT8 shade, bool compensate, double compensation, Q_INT32 nPixels) const
{
	kdDebug() << " Darken: " << shade << ", compensate: " << compensate << ", compensation: " << compensation << "\n";

	QColor c;
	Q_INT32 psize = pixelSize();
	
	for (int i = 0; i < nPixels; ++i) {
		
		const_cast<KisStrategyColorSpace *>(this) -> toQColor(src + (i * psize), &c);
		Q_INT32 r, g, b;
		
		if (compensate) {
			r = (Q_INT32) ((c.red() * shade) / (compensation * 255));
			g = (Q_INT32) ((c.green() * shade) / (compensation * 255));
			b = (Q_INT32) ((c.blue() * shade) / (compensation * 255));
		}
		else {
			r = (Q_INT32) (c.red() * shade / 255);
			g = (Q_INT32) (c.green() * shade / 255);
			b = (Q_INT32) (c.blue() * shade / 255);
		}
		kdDebug() << "Color: " << r << ", " << g << ", " << b << "\n";
		c.setRgb(r, g, b);
	
		const_cast<KisStrategyColorSpace *>(this)->nativeColor( c, dst  + (i * psize));
	}
}

void KisStrategyColorSpace::bitBlt(Q_UINT8 *dst,
				   Q_INT32 dststride,
				   KisStrategyColorSpaceSP srcSpace,
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
  	kdDebug(DBG_AREA_CMS) << id().name() << "::bitBlt. source color space: " << srcSpace -> id().name() << "\n";


	if (m_id!= srcSpace -> id()) {
		int len = pixelSize() * rows * cols;
		Q_UINT8 * convertedSrcPixels = new Q_UINT8[len];
		Q_CHECK_PTR(convertedSrcPixels);

		memset(convertedSrcPixels, 0, len * sizeof(Q_UINT8));

		if (srcProfile && dstProfile) {
 			kdDebug(DBG_AREA_CMS) << "src profile: " << srcProfile -> productName() << ", dst profile: " << dstProfile -> productName() << "\n";
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

cmsHTRANSFORM KisStrategyColorSpace::createTransform(KisStrategyColorSpaceSP dstColorStrategy,
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
                kdDebug(DBG_AREA_CMS) << "Going to create transform\n";
		cmsHTRANSFORM tf = cmsCreateTransform(srcProfile -> profile(),
						      colorSpaceType(),
						      dstProfile -> profile(),
						      dstColorStrategy -> colorSpaceType(),
						      renderingIntent,
						      flags);

		if (!tf) {
			kdDebug(DBG_AREA_CMS) << "No transform created for: src profile " << srcProfile 
				  << ", src colorspace: " << colorSpaceType() 
				  << ", dst profile " << dstProfile 
				  << ", dst colorspace: " << dstColorStrategy -> colorSpaceType() << "\n";
		}
		
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
