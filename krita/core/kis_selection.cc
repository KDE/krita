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

#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_global.h"

KisSelection::KisSelection(KisLayerSP layer, const QString& name) 
// 	: super(layer -> width(),
// 		layer -> height(),
// 		name)
{
	m_layer = layer;
}

KisSelection::~KisSelection() 
{
}

QUANTUM KisSelection::selected(Q_INT32 x, Q_INT32 y) const
{
// 	if (m_mask.valid(x, y)) {
// 		return (QUANTUM)m_mask.pixelIndex(x, y);
// 	}
// 	else {
		return 0;
// 	}
}

QUANTUM KisSelection::setSelected(Q_INT32 x, Q_INT32 y, QUANTUM s)
{
// 	if (m_mask.valid(x, y)) {
// 		int previous = m_mask.pixelIndex(x, y);
// 		m_mask.setPixel(x, y, s);
// 		return (QUANTUM)previous;
// 	}
// 	else {
		return 0;
// 	}
	
}

QImage KisSelection::maskImage() const 
{
 	return QImage();
}

void KisSelection::reset()
{
// 	m_mask.fill(NOT_SELECTED);
}

// #include "kis_selection.moc"
