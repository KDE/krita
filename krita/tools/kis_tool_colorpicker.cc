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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <qpoint.h>
#include <kaction.h>
#include <klocale.h>
#include <koColor.h>
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_view.h"
#include "kis_tool_colorpicker.h"

KisToolColorPicker::KisToolColorPicker(KisView *view, KisDoc *doc) : super(view, doc)
{
	KToggleAction *toggle;

	m_view = view;
	m_cursor = KisCursor::pickerCursor();
	toggle = new KToggleAction(i18n("&Color Picker"), "colorpicker", 0, this, SLOT(activateSelf()), view -> actionCollection(), "tool_colorpicker");
	toggle -> setExclusiveGroup("tools");
}

KisToolColorPicker::~KisToolColorPicker() 
{
}

void KisToolColorPicker::mousePress(QMouseEvent *e)
{
	KisImageSP img;
	KisPaintDeviceSP dev;
	QPoint pos;
	KoColor c;

	if (e -> button() != QMouseEvent::LeftButton && e -> button() != QMouseEvent::RightButton)
		return;

	if (!(img = m_view -> currentImg()))
		return;

	dev = img -> activeDevice();

	if (!dev || !dev -> visible())
		return;

	pos = e -> pos();
	
	if (!dev -> contains(pos))
		return;

	c = dev -> pixel(pos.x(), pos.y());

	if (e -> button() == QMouseEvent::LeftButton)
		m_view -> setFGColor(c);
	else 
		m_view -> setBGColor(c);
}

void KisToolColorPicker::setCursor(const QCursor& cursor)
{
	m_cursor = cursor;
}

void KisToolColorPicker::cursor(QWidget *w) const
{
	if (w)
		w -> setCursor(m_cursor);
}

