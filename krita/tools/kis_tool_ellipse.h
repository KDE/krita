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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __ellipsetool_h__
#define __ellipsetool_h__

#include <qpoint.h>

#include "kis_tool.h"
#include "kis_tool_rectangle.h"

class KisCanvas;
class KisDoc;
class KisPainter;
class KisView;

class EllipseTool : public RectangleTool {
	typedef RectangleTool super;

public:
	EllipseTool(KisDoc *doc, KisCanvas *canvas);
	virtual ~EllipseTool();

	virtual QString settingsName() const;

	virtual void setupAction(QObject *collection);

protected:
	virtual void draw(const QPoint& start, const QPoint& stop);
	virtual void draw(KisPainter *gc, const QRect& rc);

protected:
	int m_lineThickness;
	QPoint  m_dragStart;
	QPoint  m_dragEnd;
	bool    m_dragging;
};

#endif //__linetool_h__

