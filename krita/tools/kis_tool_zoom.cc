/*
 *  zoomtool.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qpoint.h>

#include <kaction.h>

#include "kis_canvas.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_tool_zoom.h"
#include "kis_view.h"

ZoomTool::ZoomTool(KisDoc *doc) : KisTool(doc)
{
	setCursor();
}

ZoomTool::~ZoomTool()
{
}

void ZoomTool::mousePress(QMouseEvent *e)
{
	KisView *view = getCurrentView();

	if (e -> button() != LeftButton && e -> button() != RightButton)
		return;

	if (e -> button() == LeftButton)
		view -> zoom_in();
	else
		view -> zoom_out();
}

void ZoomTool::setCursor()
{
	KisView *view = getCurrentView();

	view -> kisCanvas() -> setCursor(KisCursor::zoomCursor());
	m_cursor = KisCursor::zoomCursor();
}

void ZoomTool::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Zoom tool"), "viewmag", 0, this, SLOT(toolSelect()), collection, "tool_zoom");

	toggle -> setExclusiveGroup("tools");
}
