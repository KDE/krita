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
	  m_dragDist ( 0 ),
	  m_usePattern ( false ),
	  m_useGradient ( false )
{
#if 0 // until we got a decent cursor
	setCursor(KisCursor::crossCursor());
#endif
        m_painter = 0;
	m_dab = 0;
	m_currentImage = 0;
}

KisToolBrush::~KisToolBrush()
{
}

void KisToolBrush::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_currentImage = subject -> currentImg();

	super::update(m_subject);
}

void KisToolBrush::mousePress(QMouseEvent *e)
{
        if (!m_subject) return;

        if (!m_subject->currentBrush()) return;

        if (e->button() == QMouseEvent::LeftButton) {
		m_mode = PAINT;
		initPaint(e -> pos());

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
        }
}


void KisToolBrush::mouseMove(QMouseEvent *e)
{
	if (m_mode == PAINT) {
		paintLine(m_dragStart, e -> pos(), 128, 0, 0);
	}
}

void KisToolBrush::tabletEvent(QTabletEvent *e)
{
         if (e->device() == QTabletEvent::Stylus) {
		 if (!m_subject) {
			 e -> accept();
			 return;
		 }

		 if (!m_subject -> currentBrush()) {
			 e->accept();
			 return;
		 }

		 Q_INT32 pressure = e -> pressure();

		 if (pressure < 5 && m_mode == PAINT_STYLUS) {
			 endPaint();
		 }
		 else if (pressure >= 5 && m_mode == HOVER) {
			 m_mode = PAINT_STYLUS;
			 initPaint(e -> pos());
			 paint(e -> pos(), e->pressure(), e->xTilt(), e->yTilt());
			 m_currentImage -> notify(e -> pos().x(),
						  e -> pos().y(),
						  m_dab -> width(),
						  m_dab -> height());

		 }
		 else if (pressure >= 5 && m_mode == PAINT_STYLUS) {
			 paintLine(m_dragStart, e -> pos(), pressure, e -> xTilt(), e -> yTilt());
		 }
         }
	 e -> accept();
}


void KisToolBrush::initPaint(const QPoint & pos)
{
	m_dragStart = pos;
	m_dragDist = 0;

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
		m_spacing = m_brushWidth;
	}
	
	if (m_spacing > m_brushWidth || m_spacing > m_brushHeight) {
		m_spacing = m_brushWidth;
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

void KisToolBrush::paintLine(const QPoint & pos1,
			     const QPoint & pos2,
			     const Q_INT32 pressure,
			     const Q_INT32 xtilt,
			     const Q_INT32 ytilt)
{
	Q_INT32 x1, y1, x2, y2;

	x1 = pos1.x();
	y1 = pos1.y();

	x2 = pos2.x();
	y2 = pos2.y();

	QRect r;
	if (x1 < x2 ) {
		if (y1 < y2) {
			r = QRect(x1, y1, x2 - x1 + m_brushWidth, y2 - y1 + m_brushHeight);
		}
		else {
			r = QRect(x1, y2, x2 - x1 + m_brushWidth, y1 - y2 + m_brushHeight);
		}
	}
	else {
		if (y1 < y2) {
			r = QRect(x2, y1, x1 - x2 + m_brushWidth, y2 - y1 + m_brushHeight);
		}
		else {
			r = QRect(x2, y2, x1 - x2 + m_brushWidth, y1 - y2 + m_brushHeight);
		}
	}

	KisVector end(x2, y2);
	KisVector start(x1, y1);

	KisVector dragVec = end - start;
	float savedDist = m_dragDist;
	float newDist = dragVec.length();
	float dist = savedDist + newDist;

	if (static_cast<int>(dist) < m_spacing) {
		m_dragDist += newDist;
		m_dragStart = pos2;
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
		paint(p, pressure, xtilt, ytilt);
		dist -= m_spacing;
	}
	m_currentImage -> notify(r);

		
	if (dist > 0)
		m_dragDist = dist;

	m_dragStart = pos2;
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
