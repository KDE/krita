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

#include <koColor.h>

#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_colorspace_registry.h"
#include "kis_fill_painter.h"

// XXX: This needs to be a 8-bits one-channel color strategy that 
// can compose with any other color strategy.
KisSelection::KisSelection(KisLayerSP layer, const QString& name) 
 	: super(layer -> width(),
		layer -> height(),
		layer -> colorStrategy(),
		name)
{
	m_parentLayer = layer;
	m_maskColor = KoColor::white();
}

KisSelection::KisSelection(KisLayerSP layer, const QString& name, KoColor color) 
 	: super(layer -> width(),
		layer -> height(),
		layer -> colorStrategy(),
		name)
{
	m_parentLayer = layer;
	m_maskColor = color;
}


KisSelection::~KisSelection() 
{
}

QUANTUM KisSelection::selected(Q_INT32 x, Q_INT32 y)
{
	KoColor c;
	QUANTUM opacity;
	if (pixel(x, y, &c, &opacity)) {
		return opacity;
	}
	else {
		return MIN_SELECTED;
	}
}

void KisSelection::setSelected(Q_INT32 x, Q_INT32 y, QUANTUM s)
{
	setPixel(x, y, m_maskColor, s);
}

QImage KisSelection::maskImage() const 
{
	return QImage();
}

void KisSelection::clear() 
{
	KisFillPainter painter(this);
	painter.fillRect(0, 0, width(), height(), m_maskColor, MIN_SELECTED);
}

void KisSelection::clear(KoColor c) 
{
	KisFillPainter painter(this);
	painter.fillRect(0, 0, width(), height(), c, MIN_SELECTED);

}

void KisSelection::changeMaskColor(KoColor c)
{
	// For all pixels in the selection, change only the color.
	kdDebug() << "KisSelection::changeMaskColor not implemented.\n";
	m_maskColor = c;
}
