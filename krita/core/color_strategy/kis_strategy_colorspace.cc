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
#include "kis_strategy_colorspace.h"
#include "kis_pixel_representation.h"
#include "kis_global.h"

KisStrategyColorSpace::KisStrategyColorSpace(const QString& name) : m_name(name)
{

}

KisStrategyColorSpace::~KisStrategyColorSpace()
{
}

void KisStrategyColorSpace::convertTo(KisPixelRepresentation& src, KisPixelRepresentation& dst,  KisStrategyColorSpaceSP cs)
{
	 KisPixelRepresentationRGB intermediate;

	 this->convertToRGBA(src, intermediate);
	 cs->convertFromRGBA(intermediate, dst);
}



void KisStrategyColorSpace::convertPixels(QUANTUM * src, KisStrategyColorSpaceSP srcSpace, QUANTUM * dst, Q_UINT32 srcLen)
{
	Q_INT32 srcPixelSize = srcSpace -> depth() * sizeof(QUANTUM);
	Q_INT32 dstPixelSize = depth() * sizeof(QUANTUM);
	KoColor c;
	QUANTUM opacity;

	for (Q_UINT32 s = 0, d = 0; s <= srcLen; s += srcPixelSize, d += dstPixelSize) {
		srcSpace -> toKoColor(&src[s], &c, &opacity);
		nativeColor(c, opacity, &dst[d]);
	}
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

	QUANTUM * dstPixels = 0;
 	if (!(m_name == srcSpace -> name())) {
 		QUANTUM * dstPixels = new QUANTUM[depth() * rows * cols  * sizeof(QUANTUM)];

  		convertPixels(src, srcSpace, dstPixels, (rows * cols * srcSpace -> depth() * sizeof(QUANTUM)));
 		dst = dstPixels;
 	}

	bitBlt(stride,
	       dst, 
	       dststride,
	       src, 
	       srcstride,
	       opacity,
	       rows, 
	       cols, 
	       op);


 	if (!(m_name == srcSpace -> name())) {
 		delete[] dstPixels;
 	}

}

