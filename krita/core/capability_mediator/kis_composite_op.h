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

#if !defined KIS_COMPOSITE_OP_H
#define KIS_COMPOSITE_OP_H

#include <qstring.h>

#include "kis_global.h"
#include "kis_abstract_capability.h"

/**
   KisCompositeOp is a KisAbstractCapability and the base class of all
   the various composite operations. A composite operation can
   composite a chunk of bytes together, or a single pixel.

   A Composite Op can be relevant to one or more colour
   strategies.

   XXX: this means that the more interesting ops, where we
   need access to the whole image are harder to do.

*/
class KisCompositeOp : public KisAbstractCapability {

	typedef KisAbstractCapability super;

public:
	KisCompositeOp();
	KisCompositeOp(QString & label,
		       QString & desctription);
	virtual ~KisCompositeOp();

	virtual void composite(Q_INT32 stride,
			       QUANTUM *dst, 
			       Q_INT32 dststride,
			       QUANTUM *src, 
			       Q_INT32 srcstride,
			       Q_INT32 rows, 
			       Q_INT32 cols, 
			       QUANTUM opacity = OPACITY_OPAQUE) = 0;

};

#endif // KIS_COMPOSITE_OP_H

