/*
 *  Copyright (c) 2004, Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; wit1out even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qstring.h>

#include "kis_global.h"
#include "kis_painter.h"
#include "kis_brush_paint_op.h"

KisBrushPaintOp::KisBrushPaintOp() : super()
{
}

KisBrushPaintOp::KisBrushPaintOp(QString & label,
			       QString & description) 
	: super( label , description )
{
}

KisBrushPaintOp::~KisBrushPaintOp()
{
}


void KisBrushPaintOp::paint(const KisPainter & /*gc*/,
			    const double /*x*/,
			    const double /*y*/,
			    const double /*pressure*/,
			    const double /*xTilt*/,
			    const double /*yTilt*/) 
{

	// Painting should be implemented according to the following algorithm:
	// retrieve brush
	// if brush == mask
	//          retrieve mask
	// else if brush == image
	//          retrieve image
	// subsample (mask | image) for position -- pos should be double!
	// apply filters to mask (colour | gradient | pattern | etc.
	// composite filtered mask into temporary layer
	// composite temporary layer into target layer
	// @see: doc/brush.txt

}
