/*
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
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

#if !defined KIS_TOOL_SELECT_RECTANGULAR_H_
#define KIS_TOOL_SELECT_RECTANGULAR_H_

#include <qcursor.h>
#include <qpoint.h>
#include "kis_tool.h"
#include "kis_tool_non_paint.h"

class KisToolRectangularSelect : public KisToolNonPaint {
	Q_OBJECT
	typedef KisToolNonPaint super;

public:
	KisToolRectangularSelect(KisView *view, KisDoc *doc);
	virtual ~KisToolRectangularSelect();

	virtual void paint(QPainter& gc);
	virtual void paint(QPainter& gc, const QRect& rc);
	virtual void clear();
	virtual void clear(const QRect& rc);
	virtual void mousePress(QMouseEvent *e);
	virtual void mouseMove(QMouseEvent *e);
	virtual void mouseRelease(QMouseEvent *e);
	virtual void setCursor(const QCursor& cursor);
	virtual void cursor(QWidget *w) const;

public slots:
	virtual void activateSelf();


private:
	void paintOutline();
	void paintOutline(QPainter& gc, const QRect& rc);

#if 0
private:
	virtual void draw(const QPoint& start, const QPoint& end, QPaintEvent *e = 0);
	virtual QRegion::RegionType regionType();
	virtual void setSelection(const QRect& rc, KisLayer *lay);
	void dragSelectArea(QMouseEvent *event);
#endif

private:
	KisView *m_view;
	KisDoc *m_doc;
	QPoint m_startPos;
	QPoint m_endPos;
	QCursor m_cursor;
	bool m_selecting;
#if 0
	bool m_dragging;
	bool m_moving;
	bool m_cleared;
	bool m_firstTimeMoving;
	bool m_drawn;   
	QPoint m_dragStart;
	QPoint m_dragEnd;
	QRect m_imageRect;
	bool m_dragSelectArea;
	QPoint m_hotSpot;
	QPoint m_oldDragPoint;
	QRegion m_selectRegion;
	bool m_dragFirst;
	float m_dragdist;
#endif
};

#endif // KIS_TOOL_SELECT_RECTANGULAR_H_

