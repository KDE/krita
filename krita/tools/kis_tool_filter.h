/*
 *  kis_tool_filter.h - part of Krita
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

#ifndef __KIS_TOOL_FILTER_H__
#define __KIS_TOOL_FILTER_H__

#include "kis_tool.h"
#include "kis_tool_freehand.h"

class KisEvent;
class KisButtonPressEvent;
class KisView;

class KisToolFilter : public KisToolFreeHand {

	typedef KisToolFreeHand super;

public:
	KisToolFilter(KisView* view);
	virtual ~KisToolFilter();
  
	virtual void setup(KActionCollection *collection);
	
	virtual void paintAt(const KisPoint &pos,
			     const double pressure,
			     const double xTilt,
			     const double yTilt);
	
	virtual void paintLine(const KisPoint & pos1,
			       const double pressure1,
			       const double xTilt1,
			       const double yTilt1,
			       const KisPoint & pos2,
			       const double pressure2,
			       const double xTilt2,
			       const double yTilt2);
private:
	KisView* m_view;
};

#endif //__KIS_TOOL_FILTER_H__

