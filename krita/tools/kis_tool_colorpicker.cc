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
#include <qlayout.h>
#include <qcheckbox.h>

#include <kaction.h>
#include <klocale.h>
#include <koColor.h>

#include "kis_cursor.h"
#include "kis_canvas_subject.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_rgb_widget.h"
#include "kis_tool_colorpicker.h"
#include "kis_tool_colorpicker.moc"

KisToolColorPicker::KisToolColorPicker()
{
	setName("tool_colorpicker");
	setCursor(KisCursor::pickerCursor());
	m_subject = 0;
	m_optWidget = 0;
	m_colorSelector = 0;
	m_updateColor = 0;
	m_update = true;
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
			if(m_colorSelector)
				if (e -> button() == QMouseEvent::LeftButton)
					m_colorSelector -> slotSetFGColor(c);
				else
					m_colorSelector -> slotSetBGColor(c);
			if(m_update)
				if (e -> button() == QMouseEvent::LeftButton)
					m_subject -> setFGColor(c);
				else 
					m_subject -> setBGColor(c);
		}
	}
}

void KisToolColorPicker::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("&Color Picker"), "colorpicker", 0, this, SLOT(activate()), collection, name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

void KisToolColorPicker::update(KisCanvasSubject *subject)
{
	super::update(subject);
	m_subject = subject;
}

QWidget* KisToolColorPicker::createoptionWidget(QWidget* parent)
{
	m_optWidget = new QWidget(parent);
	m_optWidget -> setCaption(i18n("Color Picker"));
	
	m_frame = new QVBoxLayout(m_optWidget);
	m_colorSelector = new KisRGBWidget(m_optWidget);
	m_updateColor = new QCheckBox(i18n("Update current color"),m_optWidget);
	m_updateColor -> setChecked(m_updateColor);
	m_frame -> addWidget(m_colorSelector);
	m_frame -> addWidget(m_updateColor);

	connect(m_updateColor,SIGNAL(toggled(bool)), SLOT(slotSetUpdateColor(bool)));
	connect(m_colorSelector,SIGNAL(fgColorChanged(const KoColor&)), SLOT(slotFGColorSelected(const KoColor&)));
	connect(m_colorSelector,SIGNAL(bgColorChanged(const KoColor&)), SLOT(slotBGColorSelected(const KoColor&)));

	return m_optWidget;
}

QWidget* KisToolColorPicker::optionWidget()
{
	return m_optWidget;
}

void KisToolColorPicker::slotSetUpdateColor(bool state)
{
	m_update = state;
}

void KisToolColorPicker::slotFGColorSelected(const KoColor& c)
{
	if(m_update)
		m_subject -> setFGColor(c);
}

void KisToolColorPicker::slotBGColorSelected(const KoColor& c)
{
	if(m_update)
		m_subject -> setBGColor(c);
}
