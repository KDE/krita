/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
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
#include <stdlib.h>
#include <qpoint.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <koColor.h>
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_view.h"
#include "kis_tool_memento.h"
#include "kis_tool_move.h"

KisToolMove::KisToolMove(KisView *view, KisDoc *doc) : super(view, doc), KisStrategyMove(view, doc)
{
	m_view = view;
	m_doc = doc;
	setCursor(KisCursor::moveCursor());
}

KisToolMove::~KisToolMove()
{
}

void KisToolMove::mousePress(QMouseEvent *e)
{
	QPoint pos = e -> pos();
	KisImageSP img = m_view -> currentImg();
	KisPaintDeviceSP dev;

	if (!img || !(dev = img -> activeDevice()))
		return;

	if (e -> button() == QMouseEvent::LeftButton && dev -> contains(pos))
		startDrag(pos);
}

void KisToolMove::mouseMove(QMouseEvent *e)
{
	drag(e -> pos());
}

void KisToolMove::mouseRelease(QMouseEvent *e)
{
	if (e -> button() == QMouseEvent::LeftButton)
		endDrag(e -> pos());
}

void KisToolMove::keyPress(QKeyEvent *e)
{
	Q_INT32 dx = 0;
	Q_INT32 dy = 0;
	KisImageSP img;
	KisPaintDeviceSP dev;

	if (!(img = m_view -> currentImg()))
		return;

	if (!(dev = img -> activeDevice()))
		return;

	switch (e -> key()) {
	case Qt::Key_Home:
		dx = -dev -> x();
		dy = -dev -> y();
		break;
	case Qt::Key_Left:
		dx = -1;
		break;
	case Qt::Key_Right:
		dx = 1;
		break;
	case Qt::Key_Up:
		dy = -1;
		break;
	case Qt::Key_Down:
		dy = 1;
		break;
	default:
		return;
	}

}

void KisToolMove::setup()
{
	KToggleAction *toggle;

	toggle = new KToggleAction(i18n("&Move"), "move", 0, this, SLOT(activateSelf()), m_view -> actionCollection(), "tool_move");
	toggle -> setExclusiveGroup("tools");
}

