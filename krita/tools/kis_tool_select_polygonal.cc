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
#include <kcommand.h>
#include <klocale.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_floatingselection.h"
#include "kis_tool_select_polygonal.h"
#include "kis_vec.h"
#include "kis_undo_adapter.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"

namespace {
	class PolygonSelectCmd : public KNamedCommand {
		typedef KNamedCommand super;

	public:
		PolygonSelectCmd(KisFloatingSelectionSP selection);
		virtual ~PolygonSelectCmd();

	public:
		virtual void execute();
		virtual void unexecute();

	private:
		KisFloatingSelectionSP m_selection;
		KisImageSP m_owner;
	};

	PolygonSelectCmd::PolygonSelectCmd(KisFloatingSelectionSP selection) : super(i18n("Elliptical Selection"))
	{
		m_selection = selection;
		m_owner = selection -> image();
	}

	PolygonSelectCmd::~PolygonSelectCmd()
	{
	}

	void PolygonSelectCmd::execute()
	{
		m_selection -> clearParentOnMove(true);
		m_owner -> setSelection(m_selection);
		m_owner -> notify(m_selection -> bounds());
	}

	void PolygonSelectCmd::unexecute()
	{
		m_owner -> unsetSelection(false);
	}
}

KisToolSelectPolygonal::KisToolSelectPolygonal() 
	: super()
{
	setName("tool_select_polygonal");
	setCursor(KisCursor::selectCursor());

	m_subject = 0;
	m_dragging = false;
	m_selecting = false;
	m_dragStart = QPoint(-1,-1);
	m_dragEnd =   QPoint(-1,-1);

	m_start  = QPoint(-1, -1);
	m_finish = QPoint(-1, -1);     

	m_index = 0;
}

KisToolSelectPolygonal::~KisToolSelectPolygonal()
{
}


void KisToolSelectPolygonal::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	super::update(m_subject);
}


void KisToolSelectPolygonal::paint(QPainter& gc)
{
	if (m_selecting)
		paintOutline(gc, QRect());
}

void KisToolSelectPolygonal::paint(QPainter& gc, const QRect& rc)
{
	if (m_selecting)
		paintOutline(gc, rc);
}

void KisToolSelectPolygonal::buttonPress(KisButtonPressEvent *event)
{
	if (m_subject && m_subject -> currentImg()) {
		// start the polyline, and/or complete the segment
		if (event -> button() == LeftButton) {
			if (m_dragging) {
				// erase old line on canvas
				drawLine(m_dragStart, m_dragEnd);

				// get currentImg position
				m_dragEnd = event -> pos().floorQPoint();

				// draw new and final line for this segment
				drawLine(m_dragStart, m_dragEnd);
			} 
			else {
				clearSelection();
				m_start = event -> pos().floorQPoint();
				m_pointArray.resize(m_index = 0);
				m_pointArray.putPoints(m_index++, 1, m_start.x(), m_start.y());
			}

			// here we need to add the point to the point array
			// so it can be passed to the selection class to determine
			// selection area and bounds.
			m_dragging = true;
			m_dragStart = event -> pos().floorQPoint();
			m_dragEnd = event -> pos().floorQPoint();
			m_pointArray.putPoints(m_index++, 1, m_dragStart.x(), m_dragStart.y());
		} 
		else if (event -> button() == Qt::RightButton || event -> button() == Qt::MidButton) {   
			m_dragging = false;
			finish(event -> pos().floorQPoint());

			m_pointArray.putPoints(m_index++, 1, m_finish.x(), m_finish.y());
	// 		m_imageRect = getDrawRect(m_pointArray);
	// 		QPointArray points = zoomPointArray(m_pointArray);

			// need to connect start and end positions to close the
			// polyline 

			// we need a bounding rectangle and a point array of 
			// points in the polyline
			// m_doc->getSelection()->setBounds(m_selectRect);        

	// 		m_doc -> getSelection() -> setPolygonalSelection(m_imageRect, points, m_doc -> currentImg() -> getCurrentLayer());
			m_pointArray.putPoints(m_index++, 1, m_pointArray[0].x(), m_pointArray[0].y());

			kdDebug() << "selectRect" << " left: "   << m_imageRect.left() << " top: "    << m_imageRect.top();
			kdDebug()  << " right: "  << m_imageRect.right() << " bottom: " << m_imageRect.bottom() << endl;

			if (m_pointArray.size() > 1)
				m_selectRegion = QRegion(m_pointArray, true);
			else
				m_selectRegion = QRegion();

			// Initialize
	//		m_index = 0;
	//		m_pointArray.resize( 0 );
		}
	}
}

void KisToolSelectPolygonal::move(KisMoveEvent *event)
{
// 	KisView *view = getCurrentView();

	if (m_dragging) {
		drawLine(m_dragStart, m_dragEnd);
		m_dragEnd = event->pos().floorQPoint();
		drawLine(m_dragStart, m_dragEnd);
	}
}

void KisToolSelectPolygonal::buttonRelease(KisButtonReleaseEvent *event)
{
}

void KisToolSelectPolygonal::drawLine( const QPoint& start, const QPoint& end )
{
// 	if (m_subject) {
// 		KisCanvasControllerInterface *controller = m_subject -> canvasController();
// 		QWidget *canvas = controller -> canvas();
// 		QPainter p(canvas);
	
// 		p.begin( m_canvas );
// 		p.setRasterOp( Qt::NotROP );
// 		p.setPen( QPen( Qt::DotLine ) );
// 		float zF = view->zoomFactor();

// 		p.drawLine( QPoint( start.x() + view->xPaintOffset() 
// 				- (int)(zF * view->xScrollOffset()),
// 				start.y() + view->yPaintOffset() 
// 				- (int)(zF * view->yScrollOffset())), 
// 			QPoint( end.x() + view->xPaintOffset() 
// 				- (int)(zF * view->xScrollOffset()),
// 				end.y() + view->yPaintOffset() 
// 				- (int)(zF * view->yScrollOffset())) );

// 		p.end();
// 	}
}


void KisToolSelectPolygonal::paintOutline()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		QWidget *canvas = controller -> canvas();
		QPainter gc(canvas);
		QRect rc;

		paintOutline(gc, rc);
	}
}


void KisToolSelectPolygonal::paintOutline(QPainter &gc, const QRect&)
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		RasterOp op = gc.rasterOp();
		QPen old = gc.pen();
		QPen pen(Qt::DotLine);
		QPoint start;
		QPoint end;

		Q_ASSERT(controller);
// 		start = controller -> windowToView(m_startPos);
// 		end = controller -> windowToView(m_endPos);

		gc.setRasterOp(Qt::NotROP);
		gc.setPen(pen);

		gc.drawPolyline(m_pointArray);
		gc.setPen(old);
	}
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

void KisToolSelectPolygonal::clearSelection()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		KisImageSP img = m_subject -> currentImg();

		Q_ASSERT(controller);

		if (img && img -> selection().data() != 0) {
			img -> unsetSelection();
                        controller -> canvas() -> update();
		}

// 		m_startPos = QPoint(0, 0);
// 		m_endPos = QPoint(0, 0);
		m_selecting = false;
		m_dragStart = QPoint(-1,-1);
		m_dragEnd = QPoint(-1,-1);
		m_index = 0;
		m_pointArray.resize(0);
	}
}


void KisToolSelectPolygonal::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("&Polygonal Select"),
					    "polygonal" , 
					    0, 
					    this, 
					    SLOT(activate()),
					    collection, 
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}


#include "kis_tool_select_polygonal.moc"
