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

KisStrategyColorSpace::KisStrategyColorSpace(const KisID& id, Q_UINT32 cmType, icColorSpaceSignature colorSpaceSignature)
	: m_id(id),
	  m_cmType(cmType),
	  m_colorSpaceSignature(colorSpaceSignature)
{
	// Load all profiles that are suitable for this colorspace signature
	resetProfiles();
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

bool KisStrategyColorSpace::convertPixelsTo(QUANTUM * src, KisProfileSP srcProfile,
					    QUANTUM * dst, KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile,
					    Q_UINT32 length,
					    Q_INT32 renderingIntent)
{
//  	kdDebug() << "convertPixels for " << length << " pixels from " << name() << " to " << dstColorStrategy -> name() << "\n";
// 	kdDebug() << " src profile: " << srcProfile << ", dst profile: " << dstProfile << "\n";
	cmsHTRANSFORM tf = 0;

	if (!m_transforms.contains(KisProfilePair(srcProfile, dstProfile))) {
// 		kdDebug() << "Create new transform\n";
		tf = createTransform(dstColorStrategy,
				     srcProfile,
				     dstProfile,
				     renderingIntent);

		m_transforms[KisProfilePair(srcProfile, dstProfile)] = tf;
	}
	else {
// 		kdDebug() << "Use cached transform\n";
		tf = m_transforms[KisProfilePair(srcProfile, dstProfile)];
	}

	if (tf) {
		cmsDoTransform(tf, src, dst, length);
		return true;
	}

	kdDebug() << "No transform from "
		  << srcProfile -> productName() << " to " << dstProfile -> productName()
		  << ", so cannot convert pixels!\n";
	return false;

}

void KisStrategyColorSpace::bitBlt(Q_INT32 stride,
				   QUANTUM *dst,
				   Q_INT32 dststride,
				   KisStrategyColorSpaceSP srcSpace,
				   QUANTUM *src,
				   Q_INT32 srcstride,
				   QUANTUM opacity,
				   Q_INT32 rows,
				   Q_INT32 cols,
				   CompositeOp op,
				   KisProfileSP srcProfile,
				   KisProfileSP dstProfile)
{
	if (rows <= 0 || cols <= 0)
		return;
// 	kdDebug() << name() << "::bitBlt. source color space: " << srcSpace -> name() << "\n";


 	if (m_id!= srcSpace -> id()) {
// 		kdDebug() << "compositing heterogenous color spaces: src = " << srcSpace -> name() << ", dst = " << m_name << "\n";
		int len = pixelSize() * rows * cols;
 		QUANTUM * convertedSrcPixels = new QUANTUM[len];
		memset(convertedSrcPixels, 0, len * sizeof(QUANTUM));

		// XXX: Set profiles
   		srcSpace -> convertPixelsTo(src, 0,
					    convertedSrcPixels, this, 0,
					    rows * cols);
 		srcstride = (srcstride / srcSpace -> pixelSize()) * pixelSize();

		bitBlt(stride,
		       dst,
		       dststride,
		       convertedSrcPixels,
		       srcstride,
		       opacity,
		       rows,
		       cols,
		       op);

 		delete[] convertedSrcPixels;
 	}
	else {
		bitBlt(stride,
		       dst,
		       dststride,
		       src,
		       srcstride,
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
	if (!m_profileFilenames.empty()) {
		KisProfile * profile = 0;
		for ( QStringList::Iterator it = m_profileFilenames.begin(); it != m_profileFilenames.end(); ++it ) {
			profile = new KisProfile(*it, colorSpaceType());
			profile -> loadAsync();
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

	if (dstProfile != 0
	    && dstColorStrategy != 0
	    && dstColorStrategy != 0
	    && dstProfile -> colorSpaceSignature() == dstColorStrategy -> colorSpaceSignature()
	    && srcProfile != 0
	    && srcProfile -> colorSpaceSignature() == colorSpaceSignature()) {
		cmsHTRANSFORM tf = cmsCreateTransform(srcProfile -> profile(),
						      colorSpaceType(),
						      dstProfile -> profile(),
						      dstColorStrategy -> colorSpaceType(),
						      renderingIntent,
						      flags);
		return tf;
	}
	else {
		kdDebug() << "Haven't got the profiles, we're going to crash, probably.\n";
		return 0;
	}

}
