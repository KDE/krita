/*
 *  colorpicker.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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

#include <kaction.h>

#include <koColor.h>

#include "kis_tool_colorpicker.h"
#include "kis_doc.h"
#include "kis_view.h"
#include "kis_cursor.h"

ColorPicker::ColorPicker(KisDoc *doc) : KisTool(doc)
{
	m_cursor = KisCursor::pickerCursor();
}

ColorPicker::~ColorPicker() 
{
}

KoColor ColorPicker::pick(KisImageSP img, KisPaintDeviceSP device, int x, int y)
{
	if (!img -> colorMode() == cm_RGB && !img -> colorMode() == cm_RGBA)
		return KoColor::white();

	QRgb rgb = device -> pixel(x, y);
	return KoColor(qRed(rgb), qGreen(rgb),  qBlue(rgb), cs_RGB);
}

void ColorPicker::mousePress(QMouseEvent *e)
{
	if (e->button() == QMouseEvent::LeftButton || e->button() == QMouseEvent::RightButton) {
		KisView *view = getCurrentView();
		KisImageSP img = m_doc -> currentImg();
		KisPaintDeviceSP layer;

		if (!img) 
			return;

		layer = img -> getCurrentPaintDevice();
		
		if (!layer -> visible())
			return;

		QPoint pos = zoomed(e -> pos());

		if (!layer -> tileExtents().contains(pos))
			return;

		KoColor pickedColor = pick(img, layer, pos.x(), pos.y());

		if (e -> button() == QMouseEvent::LeftButton)
			view -> setSetFGColor(pickedColor);
		else 
			view -> setSetBGColor(pickedColor);
	}
}

void ColorPicker::setupAction(QObject *collection)
{
	KToggleAction *toggle = new KToggleAction(i18n("&Color picker"), "colorpicker", 0, this, SLOT(toolSelect()), collection, "tool_colorpicker");

	toggle -> setExclusiveGroup("tools");
}

