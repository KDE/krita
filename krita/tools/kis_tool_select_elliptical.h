/*
 *  kis_tool_select_elliptical.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
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

#ifndef __selecttoolelliptical_h__
#define __selecttoolelliptical_h__

#include <qpoint.h>
#include "kis_tool.h"

class KisDoc;
class KisCanvas;
class KisView;

class EllipticalSelectTool : public KisTool {
public:
	EllipticalSelectTool(KisDoc *doc, KisCanvas *canvas);
	virtual ~EllipticalSelectTool();

	virtual void setupAction(QObject *collection);

	virtual void clearOld();
	virtual bool willModify() const;

	virtual void paintEvent(QPaintEvent *e);
	virtual void mousePress(QMouseEvent *event);
	virtual void mouseMove(QMouseEvent *event);
	virtual void mouseRelease(QMouseEvent *event);

protected:
	void drawEllipse(const QPoint&, const QPoint&); 

protected:
    QPoint     m_dragStart;
    QPoint     m_dragEnd;
    bool       m_dragging;
    bool       m_drawn;   
    bool       m_init;

    KisCanvas *m_canvas;

    QRect      m_selectRect;

private:

    bool        moveSelectArea;
    bool        dragSelectArea;
    QPoint      m_hotSpot, oldDragPoint;
    QRegion     m_selectRegion;
    QRect       m_imageRect;
    bool        dragFirst;
    float       m_dragdist;
};

#endif //__selecttoolelliptical_h__
