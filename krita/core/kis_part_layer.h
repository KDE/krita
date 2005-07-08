/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_PART_LAYER_
#define _KIS_PART_LAYER_

#include "kis_layer.h"

/**
 * A PartLayer is a layer that contains a KOffice Part like a KWord document
 * or a KSpread spreadsheet. Or whatever. A Karbon drawing.
 *
 * The part is rendered into an RBGA8 paint device so we can composite it with
 * the other layers.
 */

class KisPartLayer : public KisLayer {


public:

	KisPartLayer();
	virtual ~KisPartLayer();
	

};

#endif // _KIS_PART_LAYER_