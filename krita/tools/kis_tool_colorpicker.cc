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

#include <qpoint.h>
#include <kaction.h>
#include <klocale.h>
#include <koColor.h>
#include "kis_cursor.h"
#include "kis_canvas_subject.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_tool_colorpicker.h"
#include "kis_tool_colorpicker.moc"

KisToolColorPicker::KisToolColorPicker()
{
	setCursor(KisCursor::pickerCursor());
	m_subject = 0;
}

KisToolColorPicker::~KisToolColorPicker() 
{
}

void KisToolColorPicker::mousePress(QMouseEvent *e)
{
	if (m_subject) {
		KisImageSP img;
		KisPaintDeviceSP dev;
		QPoint pos;
		KoColor c;
		QUANTUM opacity;

		if (e -> button() != QMouseEvent::LeftButton && e -> button() != QMouseEvent::RightButton)
			return;

		if (!(img = m_subject -> currentImg()))
			return;

		dev = img -> activeDevice();

		if (!dev || !dev -> visible())
			return;

		pos = e -> pos();

		if (!dev -> contains(pos))
			return;

		if (dev -> pixel(pos.x(), pos.y(), &c, &opacity)) {
			if (e -> button() == QMouseEvent::LeftButton)
				m_subject -> setFGColor(c);
			else 
				m_subject -> setBGColor(c);
		}
	}
}

void KisToolColorPicker::setup(KActionCollection *collection)
{
	KToggleAction *toggle;

	toggle = new KToggleAction(i18n("&Color Picker"), "colorpicker", 0, this, SLOT(activateSelf()), collection, "tool_colorpicker");
	toggle -> setExclusiveGroup("tools");
}

void KisToolColorPicker::update(KisCanvasSubject *subject)
{
	super::update(subject);
	m_subject = subject;
}

