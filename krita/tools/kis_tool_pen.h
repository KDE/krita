/*
 *  kis_tool_pen.h - part of KImageShop^WKrayon^WKrita
 *
 *  Copyright (c) 1999 Matthias Elter
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

#ifndef __KIS_TOOL_PEN_H__
#define __KIS_TOOL_PEN_H__

#include "kis_tool.h"
#include "kis_tool_paint.h"

class KisBrush;
class KisDoc;
class KisImageCmd;
class KisFrameBuffer;

class KisToolPen : public KisToolPaint {

	typedef KisToolPaint super;
	Q_OBJECT

public:
	KisToolPen();
	virtual ~KisToolPen();
  
	virtual void setup(KActionCollection *collection);

	virtual void mousePress(QMouseEvent*); 
	virtual void mouseMove(QMouseEvent*);
	virtual void mouseRelease(QMouseEvent*);

	bool paint(const QPoint& pos);

protected:
	KisCanvasSubject *m_subject;

	QPoint m_dragStart;
	bool m_dragging;
	float m_dragdist;

	// tool options
	int m_lineThickness;
	int m_penColorThreshold;
};

#endif //__KIS_TOOL_PEN_H__

