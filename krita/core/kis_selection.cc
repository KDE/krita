/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */

#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>
#include <qcolor.h>

#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_colorspace_registry.h"
#include "kis_fill_painter.h"
#include "kis_colorspace_alpha.h"
#include "kis_iterators_pixel.h"


KisSelection::KisSelection(KisPaintDeviceSP layer, const QString& name)
 	: super(
 		new KisColorSpaceAlpha(), // Note that the alpha color
					  // model has _state_, so we
					  // create a new one, instead
		name)
{
// 	kdDebug() << "Selection created with color space type: " << colorStrategy() -> name() << "\n";
	m_parentLayer = layer;
	m_maskColor = Qt::white;
	m_alpha = KisColorSpaceAlphaSP(dynamic_cast<KisColorSpaceAlpha*> (colorStrategy().data()));
 	m_alpha -> setMaskColor(m_maskColor);
}

KisSelection::KisSelection(KisPaintDeviceSP layer, const QString& name, QColor color)
	: super(layer -> colorStrategy(), name)
{
	m_parentLayer = layer;
	m_maskColor = color;
}


KisSelection::~KisSelection()
{
}

// QUANTUM KisSelection::selected(Q_INT32 x, Q_INT32 y)
// {
// 	QColor c;
// 	QUANTUM opacity;
// 	if (pixel(x, y, &c, &opacity)) {
// 		return opacity;
// 	}
// 	else {
// 		return MIN_SELECTED;
// 	}
// }
// 
// void KisSelection::setSelected(Q_INT32 x, Q_INT32 y, QUANTUM s)
// {
// 	setPixel(x, y, m_maskColor, s);
// }

QImage KisSelection::maskImage()
{
	Q_INT32 x, y, w, h, y2, x2;
	m_parentLayer -> exactBounds(x, y, w, h);
	QImage img = QImage(w, h, 32);
	uint black = qRgb(0, 0, 0);
	uint white = qRgb(255, 255, 255);
	
	for (y2 = y; y2 < h - y; ++y2) {
		KisHLineIteratorPixel it = createHLineIterator(x, y, w, false);
		x2 = 0;
		while (!it.isDone()) {
			// We have always just one 8-bit channel, so this is safe.
			if (it.pixel().alpha() > SELECTION_THRESHOLD) {
				
				img.setPixel(x2, y2, black);
			}
			else {
				img.setPixel(x2, y2, white);
			}
			++x2;
			++it;
		}
	}
	return img;
}

void KisSelection::select(QRect r)
{
	KisFillPainter painter(this);
	painter.fillRect(r, m_maskColor, MAX_SELECTED);
	Q_INT32 x, y, w, h;
	extent(x, y, w, h);
	kdDebug () << "Selection rect: x:" << x << ", y: " << y << ", w: " << w << ", h: " << h << "\n";
}

void KisSelection::clear(QRect r)
{
	KisFillPainter painter(this);
	painter.fillRect(r, m_maskColor, MIN_SELECTED);
}

void KisSelection::invert(QRect rect)
{
	KisRectIterator it = createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
	while ( ! it.isDone() )
	{
		// CBR this is wrong only first byte is inverted
		// BSAR: But we have always only one byte in this color model :-).
		*(it.rawData()) = QUANTUM_MAX - *(it.rawData());
		++it;
	}
}

void KisSelection::setMaskColor(QColor c)
{
	m_alpha -> setMaskColor(c);
	m_maskColor = c;
}

QRect KisSelection::selectedRect()
{
	return extent();
}

