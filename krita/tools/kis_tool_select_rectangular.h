/*
 *  selecttool.h - part of KImageShop
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
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

#ifndef __selecttoolrectangular_h__
#define __selecttoolrectangular_h__

#include <qpoint.h>
#include "kis_tool.h"

class KisDoc;
class KisCanvas;
class KisView;

class RectangularSelectTool : public KisTool {
public:
	RectangularSelectTool(KisDoc *doc, KisCanvas *canvas);
	virtual ~RectangularSelectTool();

	virtual void setupAction(QObject *collection);

	virtual void clearOld();
	virtual bool willModify() const;

	virtual void paintEvent(QPaintEvent *event);
	virtual void mousePress(QMouseEvent *event);
	virtual void mouseMove(QMouseEvent *event);
	virtual void mouseRelease(QMouseEvent *event);

protected:
	void drawRect(const QPoint& start, const QPoint& end); 

#if 0
protected:
	QPoint m_dragStart;
	QPoint m_dragEnd;
	bool m_dragging;
	bool m_drawn;   
	bool m_init;
	QRect m_selectRect;

private:
	bool m_moveSelectArea;
	bool m_dragSelectArea;
	QPoint m_hotSpot;
	QPoint m_oldDragPoint;
	QRegion m_selectRegion;
	QRect m_imageRect;
	bool m_dragFirst;
	float m_dragdist;
#endif
};

#endif //__selecttoolrectangular_h__
