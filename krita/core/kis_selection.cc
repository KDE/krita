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

#include "kdebug.h"

#include <koColor.h>

#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_colorspace_registry.h"
#include "kis_fill_painter.h"
#include "kis_colorspace_alpha.h"
#include "kis_iterators_quantum.h"
#include "kis_iterators_pixel.h"


KisSelection::KisSelection(KisLayerSP layer, const QString& name) 
 	: super(layer -> width(),
		layer -> height(),
#if USE_ALPHA_MAP
 		new KisColorSpaceAlpha(), // Note that the alpha color
					  // model has _state_, so we
					  // create a new one, instead
#else					  // of sharing.
		layer -> colorStrategy(),
#endif
		name)
{
	m_parentLayer = layer;
	m_maskColor = KoColor::white();
#if USE_ALPHA_MAP
	m_alpha = KisColorSpaceAlphaSP(dynamic_cast<KisColorSpaceAlpha*> (colorStrategy().data()));
 	m_alpha -> setMaskColor(m_maskColor);
#endif
	kdDebug() << "Selection created with compositeOp " << compositeOp() << "\n";
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

void KisSelection::select(QRect r)
{
	KisFillPainter painter(this);
	painter.fillRect(r, m_maskColor, MAX_SELECTED);
	m_selectedRect |= r;
}

void KisSelection::clear(QRect r) 
{
	KisFillPainter painter(this);
	painter.fillRect(r, m_maskColor, MIN_SELECTED);
	if (r.contains(m_selectedRect, true)) {
		    m_selectedRect = QRect();
	}
}

void KisSelection::invert(QRect r)
{
	// XXX: switch to proper iterators
	KisTileCommand* ktc = new KisTileCommand("Invert", (KisPaintDeviceSP) this ); // Create a command

	KisIteratorLineQuantum lineIt = iteratorQuantumSelectionBegin(ktc, r.x(), r.x() + r.width() - 1, r.y() );
	KisIteratorLineQuantum lastLine = iteratorQuantumSelectionEnd(ktc, r.x(), r.x() + r.width() - 1, r.y() + r.height() - 1);
	while( lineIt <= lastLine )
	{
		KisIteratorQuantum quantumIt = *lineIt;
		KisIteratorQuantum lastQuantum = lineIt.end();
		while( quantumIt <= lastQuantum )
		{
			quantumIt = QUANTUM_MAX - quantumIt;
			++quantumIt;
		}
		++lineIt;
	}
	m_selectedRect |= r;
}

void KisSelection::setMaskColor(KoColor c)
{
#if USE_ALPHA_MAP
	m_alpha -> setMaskColor(c);
#endif
	m_maskColor = c;
}

QRect KisSelection::selectedRect() 
{ 
	if (!m_selectedRect.isNull()) {
		return m_selectedRect;
	}
	else {
		return QRect(0, 0, 0, 0);
	}
}

