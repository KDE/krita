/*
 *  kis_tool_ellipse.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef __KIS_TOOL_ELLIPSE_H__
#define __KIS_TOOL_ELLIPSE_H__

#include <qpoint.h>

#include "kis_tool.h"
#include "kis_tool_rectangle.h"

class KisCanvas;
class KisDoc;
class KisPainter;
class KisView;

class KisToolEllipse : public KisToolRectangle {

	typedef KisToolRectangle super;
	Q_OBJECT

public:
	KisToolEllipse();
	virtual ~KisToolEllipse();

	virtual QString settingsName() const;

	virtual void setup(KActionCollection *collection);

protected:
	virtual void draw(const QPoint& start, const QPoint& stop);
	virtual void draw(KisPainter *gc, const QRect& rc);

protected:
	KisCanvasSubject * m_subject;

	int m_lineThickness;
	QPoint  m_dragStart;
	QPoint  m_dragEnd;
	bool    m_dragging;
};

#endif //__KIS_TOOL_ELLIPSE_H__

