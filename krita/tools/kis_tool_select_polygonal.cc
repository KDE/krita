/*
 *  kis_tool_select_polygonal.h - part of Krayon^WKrita
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

#include <qpainter.h>
#include <qregion.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"

#include "kis_tool_select_polygonal.h"
#include "kis_vec.h"

KisToolSelectPolygonal::KisToolSelectPolygonal() 
	: super()
{
	m_subject = 0;

	m_dragging = false;
	m_dragStart = QPoint(-1,-1);
	m_dragEnd =   QPoint(-1,-1);

	m_start  = QPoint(-1, -1);
	m_finish = QPoint(-1, -1);     

	setCursor(KisCursor::selectCursor());

	m_index = 0;
	m_dragging = false;
	moveSelectArea = false;
}

KisToolSelectPolygonal::~KisToolSelectPolygonal()
{
}


void KisToolSelectPolygonal::start(QPoint p)
{
	m_start = p;
}


void KisToolSelectPolygonal::finish(QPoint p)
{
	m_finish = p;
	drawLine(m_start, m_finish);
}

void KisToolSelectPolygonal::clearOld()
{
// 	KisView *view = getCurrentView();

// 	m_dragStart = QPoint(-1,-1);
// 	m_dragEnd = QPoint(-1,-1);
// 	m_index = 0;
// 	m_pointArray.resize(0);
	
// 	// clear everything in 
// 	QRect updateRect(0, 0, m_doc->currentImg()->width(), m_doc->currentImg()->height());
// 	view->updateCanvas(updateRect);
// 	m_selectRegion = QRegion();
}

void KisToolSelectPolygonal::mousePress( QMouseEvent* event )
{
// 	// start the polyline, and/or complete the segment
// 	if (event -> button() == LeftButton && !moveSelectArea) {
// 		if (m_dragging) {
// 			// erase old line on canvas
// 			drawLine(m_dragStart, m_dragEnd);
		
// 			// get currentImg position
// 			m_dragEnd = event -> pos();
			
// 			// draw new and final line for this segment
// 			drawLine(m_dragStart, m_dragEnd);
// 		} 
// 		else {
// 			clearOld();
// 			m_start = event -> pos();
// 			m_pointArray.resize(m_index = 0);
// 			m_pointArray.putPoints(m_index++, 1, m_start.x(), m_start.y());
// 		}
        
// 		// here we need to add the point to the point array
// 		// so it can be passed to the selection class to determine
// 		// selection area and bounds.
// 		m_dragging = true;
// 		m_dragStart = event -> pos();
// 		m_dragEnd = event -> pos();
// 		dragSelectArea = false;
// 		m_pointArray.putPoints(m_index++, 1, m_dragStart.x(), m_dragStart.y());
// 	} 
// 	else if (event -> button() == Qt::RightButton || event -> button() == Qt::MidButton && !moveSelectArea) {   
// 		m_dragging = false;
// 		finish(event -> pos());

// 		m_pointArray.putPoints(m_index++, 1, m_finish.x(), m_finish.y());
// 		m_imageRect = getDrawRect(m_pointArray);
// 		QPointArray points = zoomPointArray(m_pointArray);

// 		// need to connect start and end positions to close the
// 		// polyline 
        
// 		// we need a bounding rectangle and a point array of 
// 		// points in the polyline
// 		// m_doc->getSelection()->setBounds(m_selectRect);        

// 		m_doc -> getSelection() -> setPolygonalSelection(m_imageRect, points, m_doc -> currentImg() -> getCurrentLayer());
// 		m_pointArray.putPoints(m_index++, 1, m_pointArray[0].x(), m_pointArray[0].y());

// 		kdDebug() << "selectRect" << " left: "   << m_imageRect.left() << " top: "    << m_imageRect.top();
// 	       	kdDebug()  << " right: "  << m_imageRect.right() << " bottom: " << m_imageRect.bottom() << endl;

// 		if (m_pointArray.size() > 1)
// 			m_selectRegion = QRegion(m_pointArray, true);
// 		else
// 			m_selectRegion = QRegion();

// 		// Initialize
// //		m_index = 0;
// //		m_pointArray.resize( 0 );
// 	}
// 	else if (event -> button() == Qt::LeftButton && moveSelectArea) {
// 		dragSelectArea = true;
// 		m_dragFirst = true;
// 		m_dragStart = event->pos();
// 		m_dragdist = 0;

// 		m_hotSpot = event->pos();
		
// 		int x = zoomed(m_hotSpot.x());
// 		int y = zoomed(m_hotSpot.y());

// 		m_hotSpot = QPoint(x - m_imageRect.topLeft().x(), y - m_imageRect.topLeft().y());

// 		m_oldDragPoint = event->pos();
// 		setClipImage();
// 	}
}

void KisToolSelectPolygonal::mouseMove( QMouseEvent* event )
{
// 	KisView *view = getCurrentView();

// 	if (m_dragging) {
// 		drawLine(m_dragStart, m_dragEnd);
// 		m_dragEnd = event->pos();
// 		drawLine(m_dragStart, m_dragEnd);
// 	}
// 	else if (!m_dragging && !dragSelectArea) {
// 		if (!m_selectRegion.isNull() && m_selectRegion.contains(event->pos())) {
// 			setMoveCursor();
// 			moveSelectArea = true;
// 		}
// 		else {
// 			setSelectCursor();
// 			moveSelectArea = false;
// 		}
// 	}
// 	else if (dragSelectArea) {
// 		if (m_dragFirst) {
// 			// remove select image
// 			m_doc->getSelection()->erase();

// 			// refresh canvas
// 			clearOld();
// 			view->slotUpdateImage();
// 			m_dragFirst = false;
// 		}

// 		int spacing = 10;
// 		float zF = view->zoomFactor();
// 		QPoint pos = event->pos();
// 		int mouseX = pos.x();
// 		int mouseY = pos.y();

// 		KisVector end( mouseX, mouseY );
// 		KisVector start( m_dragStart.x(), m_dragStart.y() );

// 		KisVector dragVec = end - start;
// 		float saved_dist = m_dragdist;
// 		float new_dist = dragVec.length();
// 		float dist = saved_dist + new_dist;

// 		if ( (int)dist < spacing ) {
// 			m_dragdist += new_dist;
// 			m_dragStart = pos;
// 			return;
// 		}
// 		else
// 			m_dragdist = 0;

// 		dragVec.normalize();
// 		KisVector step = start;

// 		while ( dist >= spacing ) {
// 			if ( saved_dist > 0 ) {
// 				step += dragVec * ( spacing - saved_dist );
// 				saved_dist -= spacing;
// 			}
// 			else
// 				step += dragVec * spacing;

// 			QPoint p( qRound( step.x() ), qRound( step.y() ) );

// 			QRect ur( zoomed( m_oldDragPoint.x() ) - m_hotSpot.x() - view->xScrollOffset(),
// 					zoomed( m_oldDragPoint.y() ) - m_hotSpot.y() - view->yScrollOffset(),
// 					(int)( m_clipPixmap.width() * ( zF > 1.0 ? zF : 1.0 ) ),
// 					(int)( m_clipPixmap.height() * ( zF > 1.0 ? zF : 1.0 ) ) );

// 			view->updateCanvas( ur );

// 			dragSelectImage( p, m_hotSpot );

// 			m_oldDragPoint = p;
// 			dist -= spacing;
// 		}

// 		if ( dist > 0 ) 
// 			m_dragdist = dist;
// 		m_dragStart = pos;
// 	}
}

void KisToolSelectPolygonal::mouseRelease(QMouseEvent *event)
{
// 	if (moveSelectArea) {
// 		// Initialize
// 		dragSelectArea = false;
// 		m_selectRegion = QRegion();
// 		setSelectCursor();
// 		moveSelectArea = false;

// 		QPoint pos = event -> pos();

// 		KisImage *img = m_doc -> currentImg();

// 		if (!img)
// 			return;

// 		if(!img -> getCurrentLayer() -> visible())
// 			return;

// 		if (pasteClipImage(zoomed(pos) - m_hotSpot)) {
// 			img -> markDirty(QRect(zoomed(pos) - m_hotSpot, m_clipPixmap.size()));
// 			m_doc -> setModified(true);
// 		}
// 	}
}

void KisToolSelectPolygonal::drawLine( const QPoint& start, const QPoint& end )
{
// 	KisView *view = getCurrentView();
// 	QPainter p;

// 	p.begin( m_canvas );
// 	p.setRasterOp( Qt::NotROP );
// 	p.setPen( QPen( Qt::DotLine ) );
// 	float zF = view->zoomFactor();

// 	p.drawLine( QPoint( start.x() + view->xPaintOffset() 
// 				- (int)(zF * view->xScrollOffset()),
// 				start.y() + view->yPaintOffset() 
// 				- (int)(zF * view->yScrollOffset())), 
// 			QPoint( end.x() + view->xPaintOffset() 
// 				- (int)(zF * view->xScrollOffset()),
// 				end.y() + view->yPaintOffset() 
// 				- (int)(zF * view->yScrollOffset())) );

// 	p.end();
}

void KisToolSelectPolygonal::setup(KActionCollection *collection)
{
	KRadioAction *radio = new KRadioAction(i18n("&Polygonal Select"),
					       "polygonal" , 
					       0, 
					       this, 
					       SLOT(activate()),
					       collection, 
					       "tool_select_polygonal");
	radio -> setExclusiveGroup("tools");
}

bool KisToolSelectPolygonal::willModify() const
{
// 	return false;
}

void KisToolSelectPolygonal::paintEvent(QPaintEvent *e)
{
// 	KisView *view = getCurrentView();
// 	QPainter gc(m_canvas);
// 	QPen pen(Qt::DotLine);
// 	float zF = view -> zoomFactor();

// 	Q_ASSERT(view);
// 	gc.setRasterOp(Qt::NotROP);
// 	gc.setPen(pen);
// 	gc.scale(zF, zF);
// 	gc.translate(view -> xPaintOffset() - view -> xScrollOffset(), view -> yPaintOffset() - view -> yScrollOffset());
// 	gc.setClipRect(e -> rect());
// 	gc.drawPolyline(m_pointArray);
}

#include "kis_tool_select_polygonal.h"

#include "kis_tool_select_polygonal.moc"
