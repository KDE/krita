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
#include "kis_tool_zoom.h"
#include "kis_view.h"

KisZoomTool::KisZoomTool(KisView *view, KisDoc *doc) : super(view, doc)
{
	m_view = view;
	setCursor(KisCursor::zoomCursor());
}

void KisZoomTool::mousePress(QMouseEvent *e)
{
	if (e -> button() == Qt::LeftButton)
		m_view -> zoomIn();
	else if (e -> button() == Qt::RightButton)
		m_view -> zoomOut();
}

void KisZoomTool::setup()
{
	KToggleAction *toggle;

	toggle = new KToggleAction(i18n("&Zoom Tool"), "viewmag", 0, this, SLOT(activateSelf()), m_view -> actionCollection(), "tool_zoom");
	toggle -> setExclusiveGroup("tools");
}

