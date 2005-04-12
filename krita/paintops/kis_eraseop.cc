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
#include "kis_paintop.h"

#include "kis_eraseop.h"

KisPaintOp * KisEraseOpFactory::createOp(KisPainter * painter)
{ 
	KisPaintOp * op = new KisEraseOp(painter); 
	Q_CHECK_PTR(op);
	return op; 
}


KisEraseOp::KisEraseOp(KisPainter * painter)
	: super(painter)
{
}

KisEraseOp::~KisEraseOp() 
{
}

void KisEraseOp::paintAt(const KisPoint &pos,
			 const double pressure,
			 const double /*xTilt*/,
			 const double /*yTilt*/)
{
// Erasing is traditionally in paint applications one of two things:
// either it is painting in the 'background' color, or it is replacing
// all pixels with transparent (black?) pixels.
//
// That's what this paint op does for now; however, anyone who has
// ever worked with paper and soft pencils knows that a sharp piece of
// eraser rubber is a pretty useful too for making sharp to fuzzy lines
// in the graphite layer, or equally useful: for smudging skin tones.
//
// A smudge tool for Krita is in the making, but when working with
// a tablet, the eraser tip should be at least as functional as a rubber eraser.
// That means that only after repeated or forceful application should all the
// 'paint' or 'graphite' be removed from the surface -- a kind of pressure
// sensitive, incremental smudge.
//
// And there should be an option to not have the eraser work on certain
// kinds of material. Layers are just a hack for this; putting your ink work
// in one layer and your pencil in another is not the same as really working
// with the combination.

	if (!m_painter) return;
	
	KisPaintDeviceSP device = m_painter -> device();
	if (!device) return;

	KisBrush *brush = m_painter -> brush();
	KisPoint hotSpot = brush -> hotSpot(pressure);
	KisPoint pt = pos - hotSpot;

	Q_INT32 destX;
	double xFraction;
	Q_INT32 destY;
	double yFraction;

	splitCoordinate(pt.x(), &destX, &xFraction);
	splitCoordinate(pt.y(), &destY, &yFraction);

	KisAlphaMaskSP mask = brush -> mask(pressure, xFraction, yFraction);

	KisLayerSP dab = new KisLayer(device -> colorStrategy(), "eraser_dab");
	Q_CHECK_PTR(dab);

	Q_INT32 maskWidth = mask -> width();
	Q_INT32 maskHeight = mask -> height();


	if (device -> alpha()) {
		dab -> setOpacity(OPACITY_OPAQUE);
		for (int y = 0; y < maskHeight; y++) {
			for (int x = 0; x < maskWidth; x++) {
				// the color doesn't matter, since we only composite the alpha
				dab -> setPixel(x, y, m_painter -> paintColor(), QUANTUM_MAX - mask->alphaAt(x, y));
			}
		}
		QRect dabRect = dab -> extent();

		Q_ASSERT(dabRect.x() == 0);
		Q_ASSERT(dabRect.y() == 0);
	
		KisImage * image = device -> image();
	
		if (image != 0) {
			QRect imageRect = image -> bounds();
				if (destX > imageRect.width()
				|| destY > imageRect.height()
				|| destX + dabRect.width() < 0
				|| destY < + dabRect.height() < 0) return;
		}
	
		if (dabRect.isNull() || dabRect.isEmpty() || !dabRect.isValid()) return;

		m_painter -> bltSelection(destX, destY, COMPOSITE_ERASE, dab.data(), OPACITY_OPAQUE, 0, 0, maskWidth, maskHeight);

 	} else {
		dab -> setOpacity(OPACITY_TRANSPARENT);
		for (int y = 0; y < maskHeight; y++) {
			for (int x = 0; x < maskWidth; x++) {
				dab -> setPixel(x, y, m_painter -> backgroundColor(), mask->alphaAt(x, y));
			}
		}
				QRect dabRect = dab -> extent();

		Q_ASSERT(dabRect.x() == 0);
		Q_ASSERT(dabRect.y() == 0);
	
		KisImage * image = device -> image();
	
		if (image != 0) {
			QRect imageRect = image -> bounds();
				if (destX > imageRect.width()
				|| destY > imageRect.height()
				|| destX + dabRect.width() < 0
				|| destY < + dabRect.height() < 0) return;
		}
	
		if (dabRect.isNull() || dabRect.isEmpty() || !dabRect.isValid()) return;

		m_painter -> bltSelection(destX, destY, COMPOSITE_OVER, dab.data(), OPACITY_OPAQUE, 0, 0, maskWidth, maskHeight);
 	}

	m_painter -> addDirtyRect(QRect(destX, destY, maskWidth, maskHeight));
}

