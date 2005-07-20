/*
 *  kis_tool_brush.cc - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <qlabel.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_brush.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_cmb_composite.h"
#include "kis_cursor.h"
#include "kis_painter.h"
#include "kis_tool_brush.h"
#include "kis_canvas_subject.h"

KisToolBrush::KisToolBrush()
        : super(i18n("Brush"))
{
	setName("tool_brush");
	setCursor(KisCursor::brushCursor());
	m_rate = 100; // Conveniently hardcoded for now
	m_timer = new QTimer(this);
	Q_CHECK_PTR(m_timer);

	connect(m_timer, SIGNAL(timeout()), this, SLOT(timeoutPaint()));
}

KisToolBrush::~KisToolBrush()
{
	delete m_timer;
	m_timer = 0;
}

void KisToolBrush::timeoutPaint()
{
	if (currentImage() && painter()) {
		painter() -> paintAt(m_prevPos, m_prevPressure, m_prevXTilt, m_prevYTilt);
		currentImage() -> notify(painter() -> dirtyRect());
	}
}


void KisToolBrush::update(KisCanvasSubject *subject)
{
	super::update(subject);
	setCursor(KisCursor::brushCursor());
}

void KisToolBrush::initPaint(KisEvent *e) 
{
	super::initPaint(e);

	KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_subject->currentPaintop(), m_painter);

	painter()->setPaintOp(op); // And now the painter owns the op and will destroy it.
	
	if (op->incremental()) {
		m_timer -> start( m_rate );
	}
}


void KisToolBrush::endPaint()
{
	m_timer -> stop();
	super::endPaint();
}


void KisToolBrush::setup(KActionCollection *collection)
{
	
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("&Brush"),
					    "paintbrush", 0, this,
					    SLOT(activate()), collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}


#include "kis_tool_brush.moc"

