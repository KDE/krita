/*
 *  selecttool.cpp - part of Krayon
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *                2001 John Califf <jcaliff@compuzone.net>
 *                2002 Patrick Julien <freak@ideasandassociates.com>
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

#include <qpainter.h>
#include <qregion.h>

#include <kaction.h>
#include <kdebug.h>

#include "kis_canvas.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_tool_select_rectangular.h"
#include "kis_view.h"
#include "kis_vec.h"

RectangularSelectTool::RectangularSelectTool(KisDoc *doc, KisCanvas *canvas) : KisTool(doc)
{
	m_dragging = false;
	m_moving = false;
	m_cleared = true;
	m_firstTimeMoving = false;
	m_cursor = KisCursor::selectCursor();
	m_canvas = canvas;
}

RectangularSelectTool::~RectangularSelectTool()
{
}

void RectangularSelectTool::clearOld()
{
	QRect rc;

	drawRect(m_dragStart, m_dragEnd); 
	m_cleared = true;
	rc.setRect(0, 0, m_doc -> current() -> width(), m_doc -> current() -> height());
	m_view -> updateCanvas(rc);
	m_selectRegion = QRegion();
	m_dragEnd = m_dragStart = QPoint(0, 0);
}

void RectangularSelectTool::mousePress(QMouseEvent *event)
{
	if (event -> button() != LeftButton)
		return;

	if (m_moving) {
		m_dragSelectArea = true;
		m_firstTimeMoving = true;
		m_dragStart = event -> pos();
		m_dragdist = 0;
		m_hotSpot = event -> pos();
		
		int x = zoomed(m_hotSpot.x());
		int y = zoomed(m_hotSpot.y());
		
		m_hotSpot = QPoint(x - m_imageRect.topLeft().x(), y - m_imageRect.topLeft().y());
		m_oldDragPoint = event -> pos();
		setClipImage();
	}
	else {
		clearOld();
		m_cleared = false;
		drawRect(m_dragStart, m_dragEnd); 
		m_dragging = true;
		m_dragStart = event -> pos();
		m_dragEnd = event -> pos();
		m_dragSelectArea = false;
	}
}

void RectangularSelectTool::dragSelectArea(QMouseEvent *event)
{
	int spacing = 10;
	float zF = m_view -> zoomFactor();
	QPoint pos = event -> pos();
	int mouseX = pos.x();
	int mouseY = pos.y();

	KisVector end(mouseX, mouseY);
	KisVector start(m_dragStart.x(), m_dragStart.y());

	KisVector dragVec = end - start;
	float saved_dist = m_dragdist;
	float new_dist = dragVec.length();
	float dist = saved_dist + new_dist;

	Q_ASSERT(event);

	if (m_firstTimeMoving) {
		m_doc -> getSelection() -> erase();
		clearOld();
		m_view -> slotUpdateImage();
		m_firstTimeMoving = false;
	}

	if (static_cast<int>(dist) < spacing ) {
		m_dragdist += new_dist;
		m_dragStart = pos;
		return;
	}
		
	m_dragdist = 0;
	dragVec.normalize();
	KisVector step = start;

	while (dist >= spacing) {
		if (saved_dist > 0) {
			step += dragVec * (spacing - saved_dist);
			saved_dist -= spacing;
		}
		else
			step += dragVec * spacing;

		QPoint p(qRound(step.x()), qRound(step.y()));
		QRect ur(zoomed(m_oldDragPoint.x()) - m_hotSpot.x() - m_view->xScrollOffset(),
				zoomed( m_oldDragPoint.y() ) - m_hotSpot.y() - m_view->yScrollOffset(),
				(int)(m_clipPixmap.width() * (zF > 1.0 ? zF : 1.0)),
				(int)(m_clipPixmap.height() * (zF > 1.0 ? zF : 1.0)));

		m_view -> updateCanvas(ur);
		dragSelectImage(p, m_hotSpot);
		m_oldDragPoint = p;
		dist -= spacing * 2;
	}

	if (dist > 0) 
		m_dragdist = dist;

	m_dragStart = pos;
}

void RectangularSelectTool::mouseMove( QMouseEvent* event )
{
	if (m_dragSelectArea) {
		dragSelectArea(event);
		return;
	}
		
	if (m_dragging) {
		drawRect( m_dragStart, m_dragEnd );
		m_dragEnd = event->pos();
		drawRect( m_dragStart, m_dragEnd );
	}
	else {
		if ((m_moving = !m_selectRegion.isNull() && m_selectRegion.contains(event -> pos())))
			setMoveCursor();
		else 
			setSelectCursor();
	}
}

void RectangularSelectTool::mouseRelease( QMouseEvent* event )
{
	if (event -> button() == LeftButton && m_dragging && !m_moving) {
		m_dragging = false;
		m_drawn = true;

		QPoint zStart = zoomed(m_dragStart);
		QPoint zEnd   = zoomed(m_dragEnd);
		QRect selectRect(zStart.x(), zStart.y(), zEnd.x(), zEnd.y());
		QPoint old_end = m_dragEnd;

		if (zStart.x() <= zEnd.x()) {
			selectRect.setLeft(zStart.x());
			selectRect.setRight(zEnd.x());
		}    
		else {
			selectRect.setLeft(zEnd.x());                   
			selectRect.setRight(zStart.x());
		}

		if (zStart.y() <= zEnd.y()) {
			selectRect.setTop(zStart.y());
			selectRect.setBottom(zEnd.y());            
		}    
		else {
			selectRect.setTop(zEnd.y());
			selectRect.setBottom(zStart.y());            
		}

		m_imageRect = selectRect;

		if (selectRect.left() != selectRect.right() && selectRect.top() != selectRect.bottom())
			m_selectRegion = QRegion(selectRect, QRegion::Rectangle);
		else
			m_selectRegion = QRegion();

		KisImage *img = m_doc -> current();

		if(!img) 
			return;

		KisLayer *lay = img -> getCurrentLayer();

		if(!lay) 
			return;
        
		// if there are several partially overlapping or interior
		// layers we must be sure to draw only on the current one
		if (selectRect.intersects(lay -> layerExtents())) {
			selectRect = selectRect.intersect(lay -> layerExtents());

			// the selection class handles getting the selection
			// content from the given rectangular area
			m_doc -> getSelection() -> setRectangularSelection(selectRect, lay);

			kdDebug(0) << "selectRect" 
				<< " left: "   << selectRect.left() 
				<< " top: "    << selectRect.top()
				<< " right: "  << selectRect.right() 
				<< " bottom: " << selectRect.bottom() << endl;
		}    

		if (m_dragEnd.x() > m_doc -> current() -> width())
			m_dragEnd.setX(m_doc -> current() -> width());

		if (m_dragEnd.y() > m_doc -> current() -> height())
			m_dragEnd.setY(m_doc -> current() -> height());

		if (old_end != m_dragEnd) {
			drawRect(m_dragStart, old_end);
			drawRect(m_dragStart, m_dragEnd);
		}
	}
	else {
		// Initialize
		m_dragSelectArea = false;
		m_selectRegion = QRegion();
		setSelectCursor();

		QPoint pos = event -> pos();

		KisImage *img = m_doc -> current();

		if (!img)
			return;

		if (!img -> getCurrentLayer() -> visible())
			return;

		if (pasteClipImage(zoomed(pos) - m_hotSpot)) {
			img -> markDirty(QRect(zoomed(pos) - m_hotSpot, m_clipPixmap.size()));
			m_doc -> setModified(true);
		}
	}

	m_moving = false;
}

void RectangularSelectTool::drawRect(const QPoint& start, const QPoint& end, QPaintEvent *e)
{
	QPainter gc(m_canvas);
	float zF = m_view -> zoomFactor();

	gc.setRasterOp(Qt::NotROP);
	gc.setPen(QPen(Qt::DotLine));

	if (e)
		gc.setClipRect(e -> rect());

	gc.drawRect(start.x() + m_view -> xPaintOffset() - (int)(zF * m_view -> xScrollOffset()),
				start.y() + m_view -> yPaintOffset() - (int)(zF * m_view -> yScrollOffset()), 
				end.x() - start.x(), 
				end.y() - start.y());
}

void RectangularSelectTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Rectangular select"), "rectangular", 0, this,  
			SLOT(toolSelect()), collection, "tool_select_rectangular");

	toggle -> setExclusiveGroup("tools");
}

bool RectangularSelectTool::willModify() const
{
	return false;
}

void RectangularSelectTool::paintEvent(QPaintEvent *event)
{
	if (!m_cleared)
		drawRect(m_dragStart, m_dragEnd, event);
}

