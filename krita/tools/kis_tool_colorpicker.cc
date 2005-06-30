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
#include <qcolor.h>
#include <kmessagebox.h>

#include "kis_cursor.h"
#include "kis_canvas_subject.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_tool_colorpicker.h"
#include "kis_tool_colorpicker.moc"
#include "kis_button_press_event.h"
#include "kis_canvas_subject.h"
#include "kis_color.h"
#include "wdgcolorpicker.h"

KisToolColorPicker::KisToolColorPicker()
{
	setName("tool_colorpicker");
	setCursor(KisCursor::pickerCursor());
	m_optionsWidget = 0;
	m_subject = 0;
	m_updateColor = true;
	m_sampleMerged = true;
}

KisToolColorPicker::~KisToolColorPicker() 
{
}

void KisToolColorPicker::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	super::update(m_subject);
}

void KisToolColorPicker::buttonPress(KisButtonPressEvent *e)
{
	if (m_subject) {
		if (e -> button() != QMouseEvent::LeftButton && e -> button() != QMouseEvent::RightButton)
			return;

		KisImageSP img;

		if (!m_subject || !(img = m_subject -> currentImg()))
			return;

		KisPaintDeviceSP dev = img -> activeDevice();

		if (!m_sampleMerged) {
			if (!dev ) {
				return;
			}
			if (!dev -> visible()) {
				KMessageBox::information(0, i18n("Cannot pick the color as the active layer is hidden."));
				return;
			}
		}

		QPoint pos = QPoint(e -> pos().floorX(), e -> pos().floorY());

		if (!img -> bounds().contains(pos)) {
			return;
		}

		KisColor c;

		if (m_sampleMerged) {
			c = img -> mergedPixel(pos.x(), pos.y());
		} else {
			c = dev -> pixelAt(pos.x(), pos.y());
		}

		if (m_updateColor) {
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
		m_action = new KRadioAction(i18n("Tool &Color Picker"), "colorpicker", Qt::Key_E, this, SLOT(activate()), collection, name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

QWidget* KisToolColorPicker::createOptionWidget(QWidget* parent)
{
	m_optionsWidget = new ColorPickerOptionsWidget(parent);
	
	m_optionsWidget -> cbUpdateCurrentColour -> setChecked(m_updateColor);
	m_optionsWidget -> cbSampleMerged -> setChecked(m_sampleMerged);

	connect(m_optionsWidget -> cbUpdateCurrentColour, SIGNAL(toggled(bool)), SLOT(slotSetUpdateColor(bool)));
	connect(m_optionsWidget -> cbSampleMerged, SIGNAL(toggled(bool)), SLOT(slotSetSampleMerged(bool)));

	return m_optionsWidget;
}

QWidget* KisToolColorPicker::optionWidget()
{
	return m_optionsWidget;
}

void KisToolColorPicker::slotSetUpdateColor(bool state)
{
	m_updateColor = state;
}

void KisToolColorPicker::slotSetSampleMerged(bool state)
{
	m_sampleMerged = state;
}

