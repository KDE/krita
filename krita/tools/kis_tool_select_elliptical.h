/*
 *  kis_tool_select_elliptical.h - part of Krayon^WKrita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef __KIS_TOOL_SELECT_ELLIPTICAL_H__
#define __KIS_TOOL_SELECT_ELLIPTICAL_H__

#include <qpoint.h>
#include "kis_tool.h"
#include "kis_tool_non_paint.h"
#include "kis_tool_select_rectangular.h"


class KisToolSelectElliptical : public KisToolSelectRectangular {

	typedef KisToolSelectRectangular super;
	Q_OBJECT

public:
	KisToolSelectElliptical();
	virtual ~KisToolSelectElliptical();
	
	virtual void setup(KActionCollection *collection);

protected:
	virtual void draw(const QPoint& start, const QPoint& end, QPaintEvent *e = 0);
	virtual QRegion::RegionType regionType();
	virtual void setSelection(const QRect& rc, KisLayer *lay);

private:
	KisCanvasSubject * m_subject;
};

#endif //__KIS_TOOL_SELECT_ELLIPTICAL_H__

