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
#include <qpainter.h>
#include <qpen.h>
#include <qcolor.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <koColor.h>

#include "kis_vec.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_doc.h"
#include "kis_view.h"
#include "kis_tool_brush.h"
#include "kis_tool_paint.h"
#include "kis_layer.h"
#include "kis_alpha_mask.h"
#include "kis_cursor.h"

KisToolBrush::KisToolBrush()
        : super(),
          m_mode( HOVER ),
	  m_hotSpotX ( 0 ),
	  m_hotSpotY ( 0 ),
	  m_brushWidth ( 0 ),
	  m_brushHeight ( 0 ),
	  m_spacing ( 1 ),
#if defined SLOWLINE
	  m_dragDist ( 0 ),
#endif
# if defined PERICOLINE
	  m_x1 ( 0 ),
	  m_y1 ( 0 ),
#endif
	  m_usePattern ( false ),
	  m_useGradient ( false )
{
	setCursor(KisCursor::crossCursor());

        m_painter = 0;
	m_dab = 0;
	m_currentImage = 0;

#if defined TRACERLINE
	m_points = 0;
#endif
}

KisToolBrush::~KisToolBrush()
{
#if defined TRACERLINE
	if (m_points) delete m_points;
#endif
}

void KisToolBrush::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_currentImage = subject -> currentImg();

	super::update(m_subject);
}
#if defined TRACERLINE
void KisToolBrush::paint(QPainter& gc)
{
 	if (m_mode == PAINT)
 		paintLine(gc, QRect());
}

void KisToolBrush::paint(QPainter& gc, const QRect& rc)
{
 	if (m_mode == PAINT)
 		paintLine(gc, rc);
}
#endif

void KisToolBrush::mousePress(QMouseEvent *e)
{
        if (!m_subject) return;

        if (!m_subject->currentBrush()) return;

        if (e->button() == QMouseEvent::LeftButton) {
		kdDebug() << "mouse press button:" << e->button() << "\n";
		m_mode = PAINT;
		initPaint();

#if defined SLOWLINE
		// Remember the startposition of the stroke
		m_dragStart = e -> pos();
		m_dragDist = 0;
#endif

#if defined TRACERLINE
		if (m_points) delete m_points;

		m_points = new QPointArray(1);
		
		m_points -> setPoint(0, translateImageXYtoViewPort( e->pos()));
#endif		

#if defined PERICOLINE
		m_x1 = e -> pos().x();
		m_y1 = e -> pos().y();
#endif
                paint(e->pos(), 128, 0, 0);
		m_currentImage -> notify(e -> pos().x(),
 				       e -> pos().y(),
 				       m_dab -> width(),
 			               m_dab -> height());
         }
}


void KisToolBrush::mouseRelease(QMouseEvent* e)
{
	if (e->button() == QMouseEvent::LeftButton && m_mode == PAINT) {
		endPaint();
# if defined PERICOLINE
		m_x1 = 0;
		m_y1 = 0;
# endif		
        }
}


#if defined TRACERLINE
void KisToolBrush::mouseMove(QMouseEvent *e) {
	if (m_mode == PAINT) {
		Q_INT32 s = m_points -> size();
		m_points -> resize(s + 1);
		m_points -> setPoint( s, translateImageXYtoViewPort( e->pos()));
		paintLine( s - 1 );
		return;
	}
}
#endif

#if defined SLOWLINE
void KisToolBrush::mouseMove(QMouseEvent *e)
{
	if (m_mode == PAINT) {
  		QPoint pos = e -> pos();
		KisVector end(pos.x(), pos.y());
		KisVector start(m_dragStart.x(), m_dragStart.y());
		KisVector dragVec = end - start;
		float savedDist = m_dragDist;
		float newDist = dragVec.length();
		float dist = savedDist + newDist;

		if (static_cast<int>(dist) < m_spacing) {
			m_dragDist += newDist;
			m_dragStart = pos;
			return;
		}

		m_dragDist = 0;
#if 0
		dragVec.normalize(); // XX: enabling this gives a link error, so copied the relevant code below.
#endif
		double length, ilength;
		double x, y, z;
		x = dragVec.x();
		y = dragVec.y();
		z = dragVec.z();
		length = x * x + y * y + z * z;
		length = sqrt (length);

		if (length)
		{
			ilength = 1/length;
			x *= ilength;
			y *= ilength;
			z *= ilength;
		}

		dragVec.setX(x);
		dragVec.setY(y);
		dragVec.setZ(z);

		KisVector step = start;

		while (dist >= m_spacing) {
			if (savedDist > 0) {
				step += dragVec * (m_spacing - savedDist);
				savedDist -= m_spacing;
			}
			else {
				step += dragVec * m_spacing;
			}
			QPoint p(qRound(step.x()), qRound(step.y()));
			paint(p, 128, 0, 0);
			m_currentImage -> notify(p.x(),
					       p.y(),
					       m_dab -> width(),
					       m_dab -> height());

			kdDebug() << "paint: (" << p.x() << "," << p.y() << ")\n";
			dist -= m_spacing;
		}

		if (dist > 0)
			m_dragDist = dist;

		m_dragStart = pos;
         }
}
#endif

#if defined SPOTTYLINE
void KisToolBrush::mouseMove(QMouseEvent *e)
{
	// XXX: Funny, this: the mouse button of a mouse-move event is always 0; this problably means
	// I should be checking the status of every button here.
	// XXX: Even if I accept all events, playing around with the stylus gives two or three spurious
	// mouse-move events if I lift the stylus from the pad.
	if (m_mode == PAINT) {
			paint(e->pos(), 128, 0, 0);
			m_currentImage -> notify(e -> pos().x(),
					       e -> pos().y(),
					       m_dab -> width(),
					       m_dab -> height());

	}
}
#endif

#if defined PERICOLINE
void KisToolBrush::mouseMove(QMouseEvent *e)
{
	if (m_mode == PAINT) {
		Q_INT32 x1, y1, x2, y2;

		x1 = m_x1;
		y1 = m_y1;

		x2 = e->pos().x();
		y2 = e->pos().y();


		QRect r = QRect(x1, y1, x2 - x1, y2 - y1);
		kdDebug() << "Painting on: (" << x1 << "," << y1 << ") - (" << x2 - x1 << "," << y2 - y1 << ")\n";
		r.normalize();

		m_x1 = x2;
		m_y1 = y2;

		if (x1 == x2 && y1 == y2) {
			kdDebug() << "Same! (" << x2 << "," << y2 << ") - (" << x1 << "," << y1 << ")\n";
		}
// 		else if ((abs(x1 - x2) < m_spacing) && (abs(y1 - y2) < m_spacing)) {
// 			kdDebug() << "Too close! (" << x2 << "," << y2 << ") - (" << x1 << "," << y1 << ")\n";
// 		}
// 		else if ((abs(x1 - x2) == m_spacing) && (abs(y1 - y2) == m_spacing)) {
// 			kdDebug() << "Spot on! (" << x2 << "," << y2 << ") - (" << x1 << "," << y1 << ")\n";
// 			paint(e->pos(), 128, 0, 0);
// 		}
		else {
			// Draw a line
			int diffX;
			int diffY;

			int runX;
			int runY;

			Q_INT32 tmp; // For swapping

			diffX = x1 - x2;
			diffY = y1 - y2;

			if (abs (diffX) > abs(diffY) ) {

				if ( diffX < 0 ) {
					diffX = - diffX;
					diffY = - diffY;

					tmp = x1;
					x1 = x2;
					x2 = tmp;
					
					tmp = y1;
					y1 = y2;
					y2 = tmp;
				}
				// draw first point
				paint(QPoint(x2, y2), 128, 0, 0);

				// draw middle points
				for ( runX = 1; runX < diffX; runX ++ ) {
					runY = diffY * runX / diffX;
					paint(QPoint(runX + x2, runY + y2), 128, 0, 0);
				}
				// draw last point
				paint(QPoint(x1, y1), 128, 0, 0);
			}			
			else {
				if ( diffY < 0 ) {
					// swap coordinates
					diffX = - diffX;
					diffY = - diffY;

					tmp = x1;
					x1 = x2;
					x2 = tmp;

					tmp = y1;
					y1 = y2;
					y2 = tmp;
						

				}
				// draw first point
				paint(QPoint(x2, y2), 128, 0, 0);

				// draw middle points
				for( runY = 1; runY < diffY; runY++ )
				{
					runX = diffX * runY / diffY;
					paint(QPoint(runX + x2, runY + y2), 128, 0, 0);

				}
				// draw last point
				paint(QPoint(x1, y1), 128, 0, 0);
			
			}
			m_currentImage -> notify();
		}
	}
}

#endif

void KisToolBrush::tabletEvent(QTabletEvent *e)
{
         if (e->device() == QTabletEvent::Stylus) {
		 if (!m_subject) {
			 e->accept();
			 return;
		 }

		 if (!m_subject->currentBrush()) {
			 e->accept();
			 return;
		 }

		 Q_INT32 pressure = e -> pressure();

		 if (pressure < 5 && m_mode == PAINT_STYLUS) {
			 endPaint();
		 }
		 else if (pressure >= 5 && m_mode == HOVER) {
			 m_mode = PAINT_STYLUS;
			 initPaint();
			 paint(e->pos(), e->pressure(), e->xTilt(), e->yTilt());
		 }
		 else if (pressure >= 5 && m_mode == PAINT_STYLUS) {
			 kdDebug() << "Tablet: painting " << e->pos().x() << ", " << e -> pos().y() << "\n";
			 paint(e->pos(), e->pressure(), e->xTilt(), e->yTilt());
		 }
         }
	 e->accept();
}


void KisToolBrush::initPaint()
{
	// Create painter
	KisPaintDeviceSP device;
	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
		if (m_painter)
			delete m_painter;
		m_painter = new KisPainter( device );
		m_painter->beginTransaction("brush");
	}

	// Retrieve and cache brush data. XXX: this is not ideal, since it is
	// done for every stroke, even if the brush and colour have not changed.
	// So, more work is done than is necessary.
	KisBrush *brush = m_subject -> currentBrush();
	KisAlphaMask *mask = brush -> mask();
	m_brushWidth = mask -> width();
	m_brushHeight = mask -> height();

	m_hotSpot = brush -> hotSpot();
	m_hotSpotX = m_hotSpot.x();
	m_hotSpotY = m_hotSpot.y();

	m_spacing = brush -> spacing();
	if (m_spacing <= 0) {
		m_spacing = brush -> width();
	}
	
	// Set the cursor -- ideally. this should be a pixmap created from the brush,
	// now that X11 can handle colored cursors.

#if 0
	// Setting cursors has no effect until the tool is selected again; this
	// should be fixed.
	setCursor(KisCursor::brushCursor());
#endif


	// Create dab
	m_dab = new KisLayer(mask -> width(),
			     mask -> height(),
			     m_currentImage -> imgType(),
			     "dab");
        m_dab -> opacity(OPACITY_TRANSPARENT);
	for (int y = 0; y < mask -> height(); y++) {
		for (int x = 0; x < mask -> width(); x++) {
                        m_dab -> setPixel(x, y, m_subject -> fgColor(), mask -> alphaAt(x, y));
                }
        }
}

void KisToolBrush::endPaint() 
{
	m_mode = HOVER;
	KisPaintDeviceSP device;
	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
		KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
		if (adapter && m_painter) {
			// If painting in mouse release, make sure painter
			// is destructed or end()ed
			adapter -> addCommand(m_painter->endTransaction());
		}
		delete m_painter;
		m_painter = 0;
		m_dab = 0; // XXX: No need to delete m_dab because shared pointer?
	}
}

void KisToolBrush::paint(const QPoint & pos,
                         const Q_INT32 /*pressure*/,
                         const Q_INT32 /*xTilt*/,
                         const Q_INT32 /*yTilt*/)
{
#if 0
        kdDebug() << "paint: " << pos.x() << ", " << pos.y() << endl;
#endif
	Q_INT32 x = pos.x() - m_hotSpotX;
	Q_INT32 y = pos.y() - m_hotSpotY;

        if (!m_currentImage) return;

        // Blit the temporary KisPaintDevice onto the current layer
        KisPaintDeviceSP device = m_currentImage -> activeDevice();
        if (device) {
                m_painter->bitBlt( x,  y,  COMPOSITE_NORMAL, m_dab.data() );
        }
}


void KisToolBrush::setup(KActionCollection *collection)
{
        KToggleAction *toggle;
        toggle = new KToggleAction(i18n("&Brush"),
				"handdrawn", 0, this,
                                   SLOT(activate()), collection,
                                   "tool_brush");
        toggle -> setExclusiveGroup("tools");
}

#if defined TRACERLINE
void KisToolBrush::paintLine(Q_INT32 s)
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		QWidget *canvas = controller -> canvas();
		QPainter gc(canvas);
		QRect rc;

		paintLine(gc, rc, s);
	}
}

void KisToolBrush::paintLine(QPainter& gc, const QRect&, Q_INT32 s)
{
	if (m_subject) {

		RasterOp op = gc.rasterOp();
		QPen old = gc.pen();
		QPen pen( Qt::SolidLine );

		gc.setRasterOp(Qt::NotROP);
		gc.setPen(pen);

		gc.drawPolyline(*m_points, s);

		gc.setRasterOp(op);
		gc.setPen(old);
	}
}

QPoint KisToolBrush::translateImageXYtoViewPort(const QPoint& p) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		Q_ASSERT(controller);
		QPoint p2;
		p2 = controller -> viewToWindow(p);

		p2.setX(p2.x() - controller -> horzValue());
		p2.setY(p2.y() - controller -> vertValue());


		p2 *= m_subject -> zoomFactor();

		return p2;
}

#endif
