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

KisPaintOp * KisDuplicateOpFactory::createOp(KisPainter * painter)
{ 
	KisPaintOp * op = new KisDuplicateOp(painter); 
	return op; 
}


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
	if (m_source) device = m_source;
	if (!device) return;

	KisBrush * brush = m_painter->brush();
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

	QPoint srcPoint = QPoint((Q_INT32)(pt.x() - m_painter -> duplicateOffset().x()),
				 (Q_INT32)(pt.y() - m_painter -> duplicateOffset().y()));

		
	Q_INT32 sw = dab -> extent().width();
	Q_INT32 sh = dab -> extent().height();

	if (srcPoint.x() < 0 )
		srcPoint.setX(0);

	if( srcPoint.y() < 0)
		srcPoint.setY(0);

	KisPaintDeviceSP srcdev = new KisPaintDevice(dab.data() -> colorStrategy(), "duplicate srcdev");
	int srcY = 0;

	while ( srcY < sh )
	{
		KisHLineIteratorPixel srcLit = srcdev -> createHLineIterator(0, srcY, sw - 1, true);
		KisHLineIteratorPixel dabLit = dab.data() -> createHLineIterator(0, srcY, sw - 1, false);
		KisHLineIteratorPixel devLit = device -> createHLineIterator(srcPoint.x(), srcPoint.y() + srcY, srcPoint.x() + sw - 1, false);
		
		while( !srcLit.isDone() )
		{
			KisPixel srcP= srcLit.pixel();
			KisPixel dabP = dabLit.pixel();
			KisPixel devP = devLit.pixel();
			for( Q_INT32 i = 0; i < device -> colorStrategy() -> nColorChannels(); i++) {
				QUANTUM devQ = (QUANTUM) devP[ i ];
				QUANTUM dabQ = (QUANTUM) dabP[ i ];
				srcP[ i ] = (QUANTUM) (((QUANTUM_MAX - dabQ) * (devQ) ) / QUANTUM_MAX);
			}
			for( Q_INT32 i = device -> colorStrategy() -> nColorChannels(); i < device -> colorStrategy() -> nChannels(); i++) {
				QUANTUM devQ = (QUANTUM) devP[ i ];
				QUANTUM dabQ = (QUANTUM) dabP[ i ];
				srcP[ i ] = (QUANTUM) ((dabQ * devQ) / QUANTUM_MAX);
			}
			
			// XXX: Fix this when alpha is set in KisPixel
			//device -> colorStrategy() -> computeDuplicatePixel( &srcUit, &dabUit, &devUit);
			/*
			for (int i = 0; i < depth; ++i) {
				dstPR[i] = ( (QUANTUM_MAX - dabPR[i]) * (srcPR[i) ) / QUANTUM_MAX;
			}
			dstPR.alpha() = ( dabPR.alpha() * srcPR.alpha() ) / QUANTUM_MAX;
			*/
			srcLit++; dabLit++; devLit++;
		}
		srcY++;
	}

	QRect dabRect = dab -> extent();

	Q_ASSERT(dabRect.x() == 0);
	Q_ASSERT(dabRect.y() == 0);
	
	KisImage * image = device -> image();
	
	if (image != 0) {
		QRect imageRect = image -> bounds();
		if (x > imageRect.width()
			|| y > imageRect.height()
			|| x + dabRect.width() < 0
			|| y < + dabRect.height() < 0) return;
	}
	
	if (dabRect.isNull() || dabRect.isEmpty() || !dabRect.isValid()) return;
	
	m_painter -> bltSelection( x,  y,  m_painter -> compositeOp(), dab.data(), m_painter -> opacity(), 0, 0, dabRect.width(), dabRect.height());
	m_painter -> addDirtyRect(QRect(x, y, dabRect.width(), dabRect.height()));

}
