/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_alpha_mask.h"
#include "kis_point.h"
#include "kis_strategy_colorspace.h"
#include "kis_global.h"
#include "kistilemgr.h"

KisPaintOp::KisPaintOp(KisPainter * painter) 
{
	m_painter = painter;
}

KisPaintOp::~KisPaintOp()
{
}

KisLayerSP KisPaintOp::computeDab(KisAlphaMaskSP mask) 
{
	// XXX: According to the SeaShore source, the Gimp uses a temporary layer
	// the size of the layer that is being painted on. Thas layer is cleared
	// between painting actions. Our temporary layer is only just big enough to
	// contain the brush mask, and for every paintAt, the dab is composited with
	// the target layer.
	KisLayerSP dab = new KisLayer(mask -> width(),
				      mask -> height(),
				      m_painter -> device() -> colorStrategy(),
				      "dab");

	KisStrategyColorSpaceSP colorStrategy = dab -> colorStrategy();
	Q_INT32 maskWidth = mask -> width();
	Q_INT32 maskHeight = mask -> height();
	Q_INT32 dstDepth = dab -> depth();
	QUANTUM *quantums = new QUANTUM[maskWidth * maskHeight * dstDepth];
	QUANTUM *dst = quantums;

	for (int y = 0; y < maskHeight; y++) {
		for (int x = 0; x < maskWidth; x++) {
			colorStrategy -> nativeColor(m_painter -> paintColor(), 
						     mask -> alphaAt(x, y),
						     dst);
			dst += dstDepth;
		}
	}

	KisTileMgrSP dabTiles = dab -> data();
	dabTiles -> writePixelData(0, 0, maskWidth - 1, maskHeight - 1, quantums, maskWidth * dstDepth);
	delete [] quantums;

	return dab;

}

void KisPaintOp::splitCoordinate(double coordinate, Q_INT32 *whole, double *fraction)
{
	Q_INT32 i = static_cast<Q_INT32>(coordinate);

	if (coordinate < 0) {
		// We always want the fractional part to be positive.
		// E.g. -1.25 becomes -2 and +0.75
		i--;
	}

	double f = coordinate - i;

	*whole = i;
	*fraction = f;
}
