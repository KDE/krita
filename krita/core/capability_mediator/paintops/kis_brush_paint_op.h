/*
 *  Copyright (c) 2004, Boudewijn Rempt <boud@valdyas.org>
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

#if !defined KIS_BRUSH_PAINT_OP_H
#define KIS_BRUSH_PAINT_OP_H

#include <qstring.h>

#include "kis_global.h"

#include "kis_paint_op.h"

class KisPainter;

/**
   KisBrushPaintOp is a paint op that paints a 'soft', i.e. an 
   anti-aliased dot.
*/
class KisBrushPaintOp : public KisPaintOp {
	typedef KisPaintOp super;
public:

	KisBrushPaintOp();
	KisBrushPaintOp(QString & label,
		   QString & desctription);
	virtual ~KisBrushPaintOp();

	virtual void paint(const KisPainter &gc,
			   const double x,
			   const double y,
			   const Q_INT32 pressure,
			   const Q_INT32 /*xTilt*/,
			   const Q_INT32 /*yTilt*/);
};

#endif // KIS_BRUSH_PAINT_OP_H

