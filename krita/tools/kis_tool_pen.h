/*
 *  kis_tool_pen.h - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_PEN_H_
#define KIS_TOOL_PEN_H_

#include "kis_tool_freehand.h"

/**
 * Hard-edged pen-like tool. Funny: the icon is a pencil, and the effect
 * is neither that of an ink pen or a pencil, but more like a biro. Perhaps
 * rename, later, when we have a real pencil, a real fountain pen, a real
 * ink pen.
 */
class KisToolPen : public KisToolFreeHand {
	Q_OBJECT
	typedef KisToolFreeHand super;

public:
	KisToolPen();
	virtual ~KisToolPen();

        virtual void setup(KActionCollection *collection);

private:
	virtual void paintAt(const KisPoint & pos,
			     const double pressure,
			     const double xtilt,
			     const double ytilt);
	virtual void paintLine(const KisPoint & pos1,
			       const double pressure1,
			       const double xtilt1,
			       const double ytilt1,
			       const KisPoint & pos2,
			       const double pressure2,
			       const double xtilt2,
			       const double ytilt2);
};
#endif // KIS_TOOL_PEN_H_

