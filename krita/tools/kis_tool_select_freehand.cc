/*
 *  kis_tool_select_freehand.cc - part of Krayon^WKrita
 *
 *  Copyright (c) 2001 Toshitaka Fujioka <fujioka@kde.org>
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
#include <qpen.h>
#include <qregion.h>

#include <kaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcommand.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_tool_select_freehand.h"
#include "kis_view.h"
#include "kis_vec.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"

KisToolSelectFreehand::KisToolSelectFreehand() : super()
{
	setName("tool_select_freehand");
	m_subject = 0;
	setCursor(KisCursor::selectCursor());

	m_dragging = false;
	m_dragStart = QPoint(-1,-1);
	m_dragEnd =   QPoint(-1,-1);

	mStart  = QPoint(-1, -1);
	mFinish = QPoint(-1, -1);     

	m_index = 0;
	m_dragging = false;
	moveSelectArea = false;
}

KisToolSelectFreehand::~KisToolSelectFreehand()
{
}

void KisToolSelectFreehand::start( QPoint p )
{
// 	mStart = p;
}

void KisToolSelectFreehand::finish( QPoint p )
{
// 	mFinish = p;
// 	drawLine(mStart, mFinish);
// 	m_pointArray.putPoints(m_index, 1, mFinish.x(), mFinish.y());
}

void KisToolSelectFreehand::clearOld()
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

void KisToolSelectFreehand::buttonPress(KisButtonPressEvent *event)
{
// 	// start the freehand line.
// 	if (event -> button() == LeftButton && !moveSelectArea) {
// 		m_dragging = true;
// 		clearOld();
// 		start( event->pos() );

// 		m_dragStart = event->pos();
// 		m_dragEnd = event->pos();
// 		dragSelectArea = false;
// 	}
// 	else if (event -> button() == LeftButton && moveSelectArea) {
// 		dragSelectArea = true;
// 		dragFirst = true;
// 		m_dragStart = event->pos();
// 		m_dragdist = 0;

// 		m_hotSpot = event->pos();
// 		int x = zoomed( m_hotSpot.x() );
// 		int y = zoomed( m_hotSpot.y() );

// 		m_hotSpot = QPoint( x - m_imageRect.topLeft().x(), y - m_imageRect.topLeft().y() );

// 		oldDragPoint = event->pos();
// 		setClipImage();
// 	}
// 	else if (event->button() == RightButton) {
// 		// TODO
// 		return;
// 	}
}


void KisToolSelectFreehand::move(KisMoveEvent *event)
{
// 	if (event -> button() == RightButton) 
// 		return;

// 	KisView *view = getCurrentView();

// 	if (m_dragging && !dragSelectArea) {
// 		m_dragEnd = event->pos();
// 		m_pointArray.putPoints( m_index, 1, m_dragStart.x(),m_dragStart.y() );
// 		++m_index;
// 		drawLine(m_dragStart, m_dragEnd);
// 		m_dragStart = m_dragEnd;
// 	}
// 	else if (!m_dragging && !dragSelectArea) {
// 		if (!m_selectRegion.isNull() && m_selectRegion.contains(event -> pos())) {
// 			setMoveCursor();
// 			moveSelectArea = true;
// 		}
// 		else {
// 			setSelectCursor();
// 			moveSelectArea = false;
// 		}
// 	}
// 	else if (dragSelectArea) {
// 		if (dragFirst) {
// 			// remove select image
// 			m_doc->getSelection()->erase();
// 			clearOld();
// 			view->slotUpdateImage();
// 			dragFirst = false;
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

// 			QRect ur( zoomed( oldDragPoint.x() ) - m_hotSpot.x() - view->xScrollOffset(),
// 					zoomed( oldDragPoint.y() ) - m_hotSpot.y() - view->yScrollOffset(),
// 					(int)( m_clipPixmap.width() * ( zF > 1.0 ? zF : 1.0 ) ),
// 					(int)( m_clipPixmap.height() * ( zF > 1.0 ? zF : 1.0 ) ) );

// 			view->updateCanvas( ur );

// 			dragSelectImage( p, m_hotSpot );

// 			oldDragPoint = p;
// 			dist -= spacing;
// 		}

// 		if ( dist > 0 ) 
// 			m_dragdist = dist;
// 		m_dragStart = pos;
// 	}
}


void KisToolSelectFreehand::buttonRelease(KisButtonReleaseEvent *event)
{
// 	if ( event->button() == RightButton ) {
// 		// TODO
// 		return;
// 	}

// 	if ( !moveSelectArea ) {
// 		// stop drawing freehand.
// 		m_dragging = false;

// 		m_imageRect = getDrawRect( m_pointArray );
// 		QPointArray points = zoomPointArray( m_pointArray );

// 		// need to connect start and end positions to close the freehand line.
// 		finish( event->pos() );

// 		// we need a bounding rectangle and a point array of 
// 		// points in the freehand line        

// 		m_doc->getSelection()->setPolygonalSelection( m_imageRect, points, m_doc->currentImg()->getCurrentLayer() );

// 		kdDebug(0) << "selectRect" 
// 			<< " left: "   << m_imageRect.left() 
// 			<< " top: "    << m_imageRect.top()
// 			<< " right: "  << m_imageRect.right() 
// 			<< " bottom: " << m_imageRect.bottom()
// 			<< endl;

// 		if ( m_pointArray.size() > 1 )
// 			m_selectRegion = QRegion( m_pointArray, true );
// 		else
// 			m_selectRegion = QRegion();

// 		// Initialize
// //		m_index = 0;
// //		m_pointArray.resize( 0 );
// 	}
// 	else {
// 		// Initialize
// 		dragSelectArea = false;
// 		m_selectRegion = QRegion();
// 		setSelectCursor();
// 		moveSelectArea = false;

// 		QPoint pos = event->pos();

// 		KisImage *img = m_doc->currentImg();
// 		if ( !img )
// 			return;
// 		if( !img->getCurrentLayer()->visible() )
// 			return;
// 		if( pasteClipImage( zoomed( pos ) - m_hotSpot ) )
// 			img->markDirty( QRect( zoomed( pos ) - m_hotSpot, m_clipPixmap.size() ) );
// 	}
}


void KisToolSelectFreehand::drawLine( const QPoint& start, const QPoint& end )
{
// 	KisView *view = getCurrentView();
// 	QPainter p;

// 	p.begin( m_canvas );
// 	p.setRasterOp( Qt::NotROP );
// 	p.setPen( QPen( Qt::DotLine ) );
// 	float zF = view->zoomFactor();

// 	p.drawLine(start.x() + view->xPaintOffset() - (int)(zF * view->xScrollOffset()),
// 			start.y() + view->yPaintOffset() - (int)(zF * view->yScrollOffset()),
// 			end.x() + view->xPaintOffset() - (int)(zF * view->xScrollOffset()),
// 			end.y() + view->yPaintOffset() - (int)(zF * view->yScrollOffset()));

// 	p.end();
}

void KisToolSelectFreehand::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Freehand Select"), 
					    "freehand", 
					    Qt::Key_K, 
					    this,  
					    SLOT(activate()),
					    collection, 
					    name());

		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

bool KisToolSelectFreehand::willModify() const
{
// 	return false;
}

void KisToolSelectFreehand::paintEvent(QPaintEvent *e)
{
// 	if (m_pointArray.size() > 1) {
// 		KisView *view = getCurrentView();
// 		QPainter gc(m_canvas);
// 		QPen pen(Qt::DotLine);
// 		float zF = view -> zoomFactor();

// 		Q_ASSERT(view);
// 		gc.setRasterOp(Qt::NotROP);
// 		gc.setPen(pen);
// 		gc.scale(zF, zF);
// 		gc.translate(view -> xPaintOffset() - view -> xScrollOffset(), view -> yPaintOffset() - view -> yScrollOffset());
// 		gc.setClipRect(e -> rect());
// 		gc.drawPolyline(m_pointArray);
// 		gc.drawLine(m_pointArray[m_pointArray.size() - 1], m_pointArray[0]);
// 	}
}

#include "kis_tool_select_freehand.h"

#include "kis_tool_select_freehand.moc"
