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

#include "kis_duplicateop.h"

KisDuplicateOp::KisDuplicateOp(KisPainter * painter)
	: super(painter) 
{
}

KisDuplicateOp::~KisDuplicateOp() 
{
}

void KisDuplicateOp::paintAt(const KisPoint &pos,
			     const double pressure,
			     const double /*xTilt*/,
			     const double /*yTilt*/)
{
	if (!m_painter) return;
	
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

	if (brush -> brushType() == IMAGE || 
	    brush -> brushType() == PIPE_IMAGE) {
		dab = brush -> image(device -> colorStrategy(), pressure, xFraction, yFraction);
	}
	else {
		KisAlphaMaskSP mask = brush -> mask(pressure, xFraction, yFraction);
		dab = computeDab(mask);
	}
	
	m_painter -> setPressure(pressure);

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
	QPoint srcPoint = QPoint((Q_INT32)(pt.x() - m_painter -> duplicateOffset().x()),
				 (Q_INT32)(pt.y() - m_painter -> duplicateOffset().y()));

	if( srcPoint.x() >= device -> width() || srcPoint.y() >= device -> height() )
		return;
		
	Q_INT32 sw = dab->width();
	Q_INT32 sh = dab->height();

	if( srcPoint.x() + sw > device->width() )
		sw = device->width() - srcPoint.x();

	if( srcPoint.y() + sh > device->height() )
		sh = device->height() - srcPoint.y();

	if(sw < 0 || sh < 0)
		return;

	if (srcPoint.x() < 0 )
		srcPoint.setX(0);

	if( srcPoint.y() < 0)
		srcPoint.setY(0);

	KisPaintDevice* srcdev = new KisPaintDevice(sw, sh, dab.data() -> colorStrategy(), "");
	
	KisIteratorLinePixel srcLit = srcdev -> iteratorPixelSelectionBegin( 0, sx, sw - 1, sy);
	KisIteratorLinePixel dabLit = dab.data() -> iteratorPixelSelectionBegin( 0, sx, sw - 1, sy);
	KisIteratorLinePixel srcLitend = srcdev -> iteratorPixelSelectionEnd( 0, sx, sw - 1, sh - 1);
	KisIteratorLinePixel devLit = device -> iteratorPixelSelectionBegin( m_painter -> transaction(), srcPoint.x(), srcPoint.x() + sw - 1, srcPoint.y());
	while ( srcLit <= srcLitend )
	{
		KisIteratorPixel srcUit = *srcLit;
		KisIteratorPixel dabUit = *dabLit;
		KisIteratorPixel srcUitend = srcLit.end();
		KisIteratorPixel devUit = * devLit;
		while( srcUit <= srcUitend )
		{
			device -> colorStrategy() -> computeDuplicatePixel( &srcUit, &dabUit, &devUit);
			++srcUit; ++dabUit; ++devUit;
		}
		++srcLit; ++dabLit; ++devLit;
	}
	m_painter -> bitBlt( x,  y,  m_painter -> compositeOp(), srcdev, m_painter -> opacity(), sx, sy, srcdev -> width(),srcdev -> width());
	delete srcdev;
	m_painter -> addDirtyRect(QRect(x, y, dab -> width(), dab -> height()));

}
