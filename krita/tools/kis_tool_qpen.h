/*
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
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

#if !defined KIS_TOOL_QPEN_H_
#define KIS_TOOL_QPEN_H_

#include <qpoint.h>
#include <qpointarray.h>
#include <qpixmap.h>

#include "kis_tool.h"
#include "kis_tool_paint.h"

/**
   KisToolQPen is a very simple pen tool that
   paints a line using QPainter.

*/
class KisToolQPen : public KisToolPaint {
	typedef KisToolPaint super;

public:
	KisToolQPen(KisView *view, KisDoc *doc);
	virtual ~KisToolQPen();

public:
        virtual void setup();
	virtual void mousePress(QMouseEvent *e);
	virtual void tabletEvent(QTabletEvent *e);
	virtual void mouseMove(QMouseEvent *e);
	virtual void mouseRelease(QMouseEvent *e);

private:
	KisView *m_view;
	KisDoc *m_doc;

        bool m_mousePressed; // Are we drawing?
        Q_INT32 m_oldPressure;
        // If the mouse moves too fast, keep the delta's in here
        QPointArray m_polyline; 
};
#endif // KIS_TOOL_QPEN_H_

