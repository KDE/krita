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

KisStrategyColorSpace::KisStrategyColorSpace(const QString& name, Q_UINT32 cmType, icColorSpaceSignature colorSpaceSignature) 
	: m_name(name),
	  m_cmType(cmType),
	  m_colorSpaceSignature(colorSpaceSignature),
	  m_displayTransform(0),
	  m_profile(0)
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
	m_profile = 0;
	if (m_displayTransform != 0) cmsDeleteTransform(m_displayTransform);
}

void KisStrategyColorSpace::convertTo(KisPixel& /*src*/, KisPixel& /*dst*/,  KisStrategyColorSpaceSP /*cs*/)
{
	// XXX: Call convertPixels
}


void KisStrategyColorSpace::convertPixels(QUANTUM * src, KisStrategyColorSpaceSP srcSpace, QUANTUM * dst, Q_UINT32 numPixels)
{
	kdDebug() << "convertPixels for " << numPixels << " pixels.\n";

 	if (!m_transforms.contains(srcSpace -> colorSpaceType())) {
 		cmsHTRANSFORM tf = cmsCreateTransform(srcSpace -> profile(), 
						      srcSpace -> colorSpaceType(), 
						      profile(),
						      m_cmType, 
						      INTENT_PERCEPTUAL,
						      0);
		m_transforms[srcSpace -> colorSpaceType()] = tf;
 	}

	cmsHTRANSFORM tf = m_transforms[srcSpace -> colorSpaceType()];

	cmsDoTransform(tf, src, dst, numPixels);
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
				   CompositeOp op)
{
	if (rows <= 0 || cols <= 0)
		return;
	
	// kdDebug() << name() << "::bitBlt. stride: " << stride
// 		  << ", dststride: " << dststride
// 		  << ", srcSpace: " << srcSpace -> name()
// 		  << ", opacity: " << (Q_UINT8) opacity
// 		  << ", rows: " << rows
// 		  << ", cols: " << cols
// 		  << ", op: " << op << "\n";


 	if (!(m_name == srcSpace -> name())) {
		int len = depth() * rows * cols;
 		QUANTUM * convertedSrcPixels = new QUANTUM[len];
		memset(convertedSrcPixels, 255, len * sizeof(QUANTUM));

  		convertPixels(src, srcSpace, convertedSrcPixels, (rows * cols * srcSpace -> depth()));
 		srcstride = (srcstride / srcSpace -> depth()) * depth();

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
	// XXX: Should find a way to make sure not all profiles are loaded for all color strategies
	m_profiles.clear();
	m_profileFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_profiles", "*.icm");
	m_profileFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_profiles", "*.ICM");
	if (!m_profileFilenames.empty()) {
		KisProfile * profile = 0;
		for ( QStringList::Iterator it = m_profileFilenames.begin(); it != m_profileFilenames.end(); ++it ) {
			profile = new KisProfile(*it);
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
