/*
 *  kis_tool_airbrush.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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
#include <qevent.h>
#include <qtimer.h>

#include <kaction.h>
#include <kcommand.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_brush.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_tool_airbrush.h"
#include "kis_view.h"

namespace {
	Q_INT32 RATE = 100;
}

KisToolAirBrush::KisToolAirBrush()
	: super(),
	  m_mode( HOVER ),
	  m_dragDist( 0 ),
	  m_pressure( PRESSURE_DEFAULT ),
	  m_xTilt( 0 ),
	  m_yTilt( 0 )

{
	setCursor(KisCursor::airbrushCursor());

	m_painter = 0;
	m_currentImage = 0;

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(timeoutPaint()));
}

KisToolAirBrush::~KisToolAirBrush()
{
}

void KisToolAirBrush::timeoutPaint()
{
	if (m_painter) {
		m_painter -> airBrushAt(m_currentPos, m_pressure, m_xTilt, m_yTilt);
		m_currentImage -> notify( m_painter -> dirtyRect() );
	}
}

void KisToolAirBrush::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_currentImage = subject -> currentImg();

	super::update(m_subject);
}

void KisToolAirBrush::mousePress(QMouseEvent *e)
{
	if (!m_subject) return;
	if (!m_currentImage) return;
	if (!m_currentImage -> activeDevice()) return;

	if (e->button() == QMouseEvent::LeftButton) {
		m_mode = PAINT;
		initPaint(e -> pos());
		m_painter -> airBrushAt(e -> pos(), PRESSURE_DEFAULT, 0, 0);
		m_currentImage -> notify( m_painter -> dirtyRect() );
		m_currentPos = e -> pos();
	}
}


void KisToolAirBrush::mouseRelease(QMouseEvent *e)
{
	if (e->button() == QMouseEvent::LeftButton && m_mode == PAINT ) {
		endPaint();
	}
}

void KisToolAirBrush::mouseMove(QMouseEvent *e)
{
	if (m_mode == PAINT) {
		paintLine(m_dragStart, e -> pos(), PRESSURE_DEFAULT, 0, 0);
		m_currentPos = e -> pos();
	}
}

void KisToolAirBrush::tabletEvent(QTabletEvent *e)
{
         if (e->device() == QTabletEvent::Stylus) {
		 if (!m_currentImage -> activeDevice()) {
			 e -> accept();
			 return;
		 }

		 if (!m_subject) {
			 e -> accept();
			 return;
		 }

		 double pressure = e -> pressure() / 255.0;

		 if (pressure < PRESSURE_THRESHOLD && m_mode == PAINT_STYLUS) {
			 endPaint();
		 } else if (pressure >= PRESSURE_THRESHOLD && m_mode == HOVER) {
			 m_mode = PAINT_STYLUS;
			 initPaint(e -> pos());
			 m_painter -> airBrushAt(e -> pos(), pressure, e->xTilt(), e->yTilt());
			 m_currentImage -> notify( m_painter -> dirtyRect() );

		 } else if (pressure >= PRESSURE_THRESHOLD && m_mode == PAINT_STYLUS) {
			 paintLine(m_dragStart, e -> pos(), pressure, e -> xTilt(), e -> yTilt());
		 }
		 m_currentPos = e -> pos();
		 m_pressure = pressure;
		 m_xTilt = e -> xTilt();
		 m_yTilt = e -> yTilt();

         }
	 e -> accept();
}

void KisToolAirBrush::initPaint(const QPoint & pos)
{
	if (!m_currentImage) return;
	if (!m_currentImage -> activeDevice()) return;

	m_dragStart = pos;
	m_dragDist = 0;

	// Create painter
	KisPaintDeviceSP device;
	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
            delete m_painter;
		m_painter = new KisPainter( device );
		m_painter -> beginTransaction(i18n("airbrush"));
	}

	m_painter -> setBrush(m_subject -> currentBrush());
	m_painter -> setPaintColor(m_subject -> fgColor());

	// Set the cursor -- ideally. this should be a mask created from the brush,
	// now that X11 can handle custom cursors.
#if 0
	// Setting cursors has no effect until the tool is selected again; this
	// should be fixed.
	setCursor(KisCursor::brushCursor());
#endif

	m_timer -> start( RATE );
}

void KisToolAirBrush::endPaint()
{
	m_mode = HOVER;
	m_timer -> stop();

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

	}
}

void KisToolAirBrush::paintLine(const QPoint & pos1,
				const QPoint & pos2,
				const double pressure,
				const double xtilt,
				const double ytilt)
{
	if (!m_currentImage -> activeDevice()) return;

	m_dragDist = m_painter -> paintLine(PAINTOP_AIRBRUSH, pos1, pos2, pressure, xtilt, ytilt, m_dragDist);
	m_currentImage -> notify( m_painter -> dirtyRect() );
	m_dragStart = pos2;
}


void KisToolAirBrush::setup(KActionCollection *collection)
{
	KRadioAction * radio = new KRadioAction(i18n("&Airbrush Tool"),
						"airbrush", 0, this,
						SLOT(activate()), collection,
						"tool_airbrush");
	radio -> setExclusiveGroup("tools");
}



#include "kis_tool_airbrush.moc"

