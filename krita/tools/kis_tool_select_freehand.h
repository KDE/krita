/*
 *  kis_tool_select_freehand.h - part of Krayon
 *
 *  Copyright (c) 2001 Toshitaka Fujioka <fujioka@kde.org>
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

#ifndef __selecttoolfreehand_h__
#define __selecttoolfreehand_h__

#include <qpoint.h>
#include <qpointarray.h>

#include "kis_tool.h"

class KisDoc;
class KisCanvas;
class KisView;

class FreehandSelectTool : public KisTool {
public:
	FreehandSelectTool(KisDoc *doc, KisCanvas *canvas);
	virtual ~FreehandSelectTool();

	virtual void setupAction(QObject *collection);

	virtual void clearOld();
	virtual bool willModify() const;

	virtual void paintEvent(QPaintEvent *e);
	virtual void mousePress(QMouseEvent *event);
	virtual void mouseMove(QMouseEvent *event);
	virtual void mouseRelease(QMouseEvent *event);

	void start( QPoint p );
	void finish( QPoint p ); 

protected:
	void drawLine(const QPoint& start, const QPoint& end); 

	QPoint      m_dragStart;
	QPoint      m_dragEnd;

	QPoint      mStart;
	QPoint      mFinish;

	bool        m_dragging;
	bool        m_drawn;   

	KisCanvas   *m_canvas;

private:
	QRect m_selectRect;
	QPointArray m_pointArray;
	uint m_index;

	bool moveSelectArea;
	bool dragSelectArea;
	QPoint m_hotSpot;
       	QPoint oldDragPoint;
	QRegion m_selectRegion;
	QRect m_imageRect;
	bool dragFirst;
	float m_dragdist;
};

#endif //__selecttoolfreehand_h__

