/*
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include <kaction.h>
#include <klocale.h>
#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_tool_zoom.h"
#include "kis_tool_zoom.moc"

KisToolZoom::KisToolZoom()
{
	m_subject = 0;
	setCursor(KisCursor::zoomCursor());
}

KisToolZoom::~KisToolZoom()
{
}

void KisToolZoom::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	super::update(m_subject);
}

void KisToolZoom::mousePress(QMouseEvent *e)
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
	
		if (e -> button() == Qt::LeftButton)
			controller -> zoomIn(e -> pos().x(), e -> pos().y());
		else if (e -> button() == Qt::RightButton)
			controller -> zoomOut(e -> pos().x(), e -> pos().y());
	}
}

void KisToolZoom::setup(KActionCollection *collection)
{
	KToggleAction *toggle;

	toggle = new KToggleAction(i18n("&Zoom Tool"), "viewmag", 0, this, SLOT(activate()), collection, "tool_zoom");
	toggle -> setExclusiveGroup("tools");
}

