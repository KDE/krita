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

#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_global.h"

KisSelection::KisSelection(KisPaintDeviceSP layer, const QString& name) 
{
	m_layer = layer;
	m_mask = new MaskVector(layer -> width() * layer -> height(), OPACITY_TRANSPARENT);
}

KisSelection::~KisSelection() 
{
	delete m_mask;
}
