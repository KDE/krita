/*
 *  kis_tool_duplicate.h - part of Krita
 *
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef __KIS_TOOL_DUPLICATE_H__
#define __KIS_TOOL_DUPLICATE_H__

#include "kis_tool.h"
#include "kis_tool_freehand.h"

class KisEvent;
class KisButtonPressEvent;

class KisToolDuplicate : public KisToolFreeHand {

	typedef KisToolFreeHand super;
	Q_OBJECT

public:
	KisToolDuplicate();
	virtual ~KisToolDuplicate();
  
	virtual void setup(KActionCollection *collection);
	virtual void buttonPress(KisButtonPressEvent *e);
	
	virtual void paintAt(const KisPoint &pos,
			     const double pressure,
			     const double xTilt,
			     const double yTilt);
	

protected slots:
	virtual void activate();

protected:
	virtual void initPaint(KisEvent *e);

	// Tool starting duplicate
	KisPoint m_offset; // This member give the offset from the click position to the point where we take the duplication
	bool m_isOffsetNotUptodate; // Tells if the offset is update
	KisPoint m_position; // Give the position of the last alt-click
};

#endif //__KIS_TOOL_DUPLICATE_H__

