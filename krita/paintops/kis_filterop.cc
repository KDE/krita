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

#include <qrect.h>

#include <kdebug.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_iterators_pixel.h"
#include "kis_paintop.h"

#include "kis_filterop.h"


KisPaintOp * KisFilterOpFactory::createOp(KisPainter * painter)
{ 
	KisPaintOp * op = new KisFilterOp(painter); 
	return op; 
}


KisFilterOp::KisFilterOp(KisPainter * painter)
	: super(painter) 
{
}

KisFilterOp::~KisFilterOp() 
{
}

void KisFilterOp::paintAt(const KisPoint &pos,
			  const double pressure,
			  const double /*xTilt*/,
			  const double /*yTilt*/)
{
	if (!m_painter) return;
	
	KisFilterSP filter = m_painter -> filter();
	if (!filter) return;

	KisPaintDeviceSP device = m_painter -> device();
	if (!device) return;

	KisBrush * brush = m_painter -> brush();
	if (!brush) return;

	KisPoint hotSpot = brush -> hotSpot(pressure);
	KisPoint pt = pos - hotSpot;

	// Split the coordinates into integer plus fractional parts. The integer
	// is where the dab will be positioned and the fractional part determines
	// the sub-pixel positioning.
	Q_INT32 x;
	double xFraction;
	Q_INT32 y;
	double yFraction;

	splitCoordinate(pt.x(), &x, &xFraction);
	splitCoordinate(pt.y(), &y, &yFraction);

	KisPaintDeviceSP dab = 0;
	if (brush -> brushType() == IMAGE || brush -> brushType() == PIPE_IMAGE) {
		return;
	}
	else {
		KisAlphaMaskSP mask = brush -> mask(pressure, xFraction, yFraction);
		dab = computeDab(mask);
	}
	
	m_painter -> setPressure(pressure);
	
	Q_INT32 sw = dab->width();
	Q_INT32 sh = dab->height();

	if( x + sw > device->width() )
		sw = device->width() - x;

	if( y + sh > device->height() )
		sh = device->height() - y;

	if(sw < 0 || sh < 0)
		return;

	// Draw correctly near the left and top edges
	Q_INT32 sx = 0;
	Q_INT32 sy = 0;
	if (x < 0) {
		sx = -x;
		x = 0;
	}
	if (y < 0) {
		sy = -y;
		y = 0;
	}

	KisPaintDevice* srcdev = new KisPaintDevice(sw, sh, dab.data() -> colorStrategy(), "");

	KisIteratorLinePixel srcLit = srcdev -> iteratorPixelSelectionBegin( 0, sx, sw - 1, sy);

	KisIteratorLinePixel dabLit = dab.data()->iteratorPixelSelectionBegin( 0, sx, sw - 1, sy);

	KisIteratorLinePixel srcLitend = srcdev->iteratorPixelSelectionEnd( 0, sx, sw - 1, sh - 1);

	// XXX: Do we need the transaction here? The bitBlt later on
	// makes it already part of the transaction.
	KisIteratorLinePixel devLit = device->iteratorPixelSelectionBegin( m_painter -> transaction(), x, x + sw - 1, y);

	Q_INT32 stop = device -> depth() - 1;

	while ( srcLit <= srcLitend )
	{
		KisIteratorPixel srcUit = *srcLit;
		KisIteratorPixel srcUitend = srcLit.end();
		KisIteratorPixel dabUit = *dabLit;
		KisIteratorPixel devUit = * devLit;
		while ( srcUit <= srcUitend )
		{
			KisPixelRepresentation srcP = srcUit;
			KisPixelRepresentation dabP = dabUit;
			KisPixelRepresentation devP = devUit;
			for( Q_INT32 i = 0; i < stop; i++)
			{
				srcUit[ i ] = ( devUit[ i ] * (QUANTUM_MAX - dabUit[ i ]) ) / QUANTUM_MAX;
// 				kdDebug() << " srcUit[ " << i << " ] = " << srcUit[i] << " devUit[ " << i << " ] = " << devUit[i] << " dabUit[ " << i << " ] = " << dabUit[ i ] << endl;
			}
			srcUit[ stop ] = ( dabUit[ stop ] );//* devUit[ stop ] ) / QUANTUM_MAX;
// 			kdDebug() << " srcUit[ " << stop << " ] = " << srcUit[stop] << " devUit[ " << stop << " ] = " << devUit[stop] << " dabUit[ " << stop << " ] = " << dabUit[ stop ] << endl;
			++srcUit; ++dabUit; ++devUit;
		}
	++srcLit; ++dabLit; ++devLit;
	}
// 	kdDebug() << "applying filter to the square" << endl;
	filter -> process( srcdev, m_filterConfiguration, QRect(0, 0, srcdev -> width(), srcdev -> height()), 0 );
	
// 	KisIteratorLinePixel srcLit2 = srcdev->iteratorPixelSelectionBegin( 0, sx, sw - 1, sy);
// 	KisIteratorLinePixel srcLitend2 = srcdev->iteratorPixelSelectionEnd( 0, sx, sw - 1, sh - 1);
// 	while ( srcLit <= srcLitend )
// 	{
// 		KisIteratorPixel srcUit = *srcLit2;
// 		KisIteratorPixel srcUitend = srcLit2.end();
// 		while ( srcUit <= srcUitend )
// 		{
// 			KisPixelRepresentation srcP = srcUit;
// 			for( Q_INT32 i = 0; i < device->depth(); i++)
// 			{
// 				kdDebug() << " srcUit[ " << i << " ] = " << srcUit[i] << endl;
// 			}
// 			++srcUit;
// 		}
// 		++srcLit2;
// 	}
	
	
	m_painter -> bitBlt( x,  y,  m_painter -> compositeOp(), srcdev, m_painter -> opacity(), sx, sy, srcdev -> width(),srcdev -> width());
	delete srcdev;
	m_painter -> addDirtyRect(QRect(x, y, dab -> width(), dab -> height()));
}
