/*
 *  polylinetool.cc - part of Krayon
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
#include <qstring.h>

#include <kaction.h>
#include <klocale.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"

#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_tool_polyline.h"
#include "kis_view.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"

KisToolPolyLine::KisToolPolyLine() 
	: super()
{
	setName("tool_polyline");
	m_subject = 0;
}

KisToolPolyLine::~KisToolPolyLine()
{
}

void KisToolPolyLine::buttonPress(KisButtonPressEvent *event)
{
// 	KisView *view = getCurrentView();
// 	KisPainter *p;

// 	// start the polyline, and/or complete the 
// 	// polyline segment
// 	if (event -> button() == LeftButton) {
// 		if (m_dragging) {
// 			// erase old line on canvas
// 			draw(m_dragStart, m_dragEnd);
// 			m_dragEnd = event -> pos();
// 			p = view -> kisPainter();
// 			p -> drawLine(zoomed(m_dragStart.x()), zoomed(m_dragStart.y()), zoomed(m_dragEnd.x()),   zoomed(m_dragEnd.y()));
// 		}
        
// 		m_dragging = true;
// 		m_dragStart = event -> pos();
// 		m_dragEnd = event -> pos();
// 	}
// 	else {   
// 		m_dragging = false;
// 		m_dragEnd = event -> pos();
// 		p = view -> kisPainter();
// 		p -> drawLine(zoomed(m_dragStart.x()), zoomed(m_dragStart.y()), zoomed(m_dragEnd.x()), zoomed(m_dragEnd.y()));
// 	}    
}

void KisToolPolyLine::buttonRelease(KisButtonReleaseEvent * /*event*/)
{
}

void KisToolPolyLine::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Polyline"),
					    "polyline", 
					    Qt::Key_P, 
					    this, 
					    SLOT(activate()), 
					    collection, 
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

QString KisToolPolyLine::settingsName() const
{
	return "polylineTool";
}

#include "kis_tool_polyline.moc"
