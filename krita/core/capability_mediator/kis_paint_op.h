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

#if !defined KIS_PAINT_OP_H
#define KIS_PAINT_OP_H

#include <qstring.h>

#include "kis_global.h"

#include "kis_abstract_capability.h"

class KisPainter;

/**
   KisPaintOp is a KisAbstractCapability and the base class of all
   the various paint operations. A paint operation implements a certain
   painting strategy, like soft painting, hard painting, airbrushing, 
   convolving.

   When the paintop is done creating what it needs to, it can bitBlt the
   result onto the actual layer.

   The paintop can use KisPainter to retrieve the currently set brushes,
   colours etc., until I devise something to configure a certain paint action.

*/
class KisPaintOp : public KisAbstractCapability {

	typedef KisAbstractCapability super;
public:

	KisPaintOp();
	KisPaintOp(QString & label,
		   QString & description);
	virtual ~KisPaintOp();

	virtual void paint(const KisPainter &gc,
			   const double x,
			   const double y,
			   const Q_INT32 pressure,
			   const Q_INT32 /*xTilt*/,
			   const Q_INT32 /*yTilt*/) = 0;
	
};

#endif // KIS_PAINT_OP_H

