/*
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include <qlabel.h>
#include <qcombobox.h>
#include <qcolor.h>

#include <kaction.h>
#include <klocale.h>
#include <knuminput.h>

#include "kis_cursor.h"
#include "kis_canvas_subject.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_tool_selectpicker.h"
#include "kis_tool_selectpicker.moc"
#include "kis_button_press_event.h"
#include "kis_canvas_subject.h"
#include "kis_selection_options.h"

KisToolSelectPicker::KisToolSelectPicker()
{
	setName("tool_selectpicker");
	setCursor(KisCursor::pickerCursor());
	m_subject = 0;
	m_optWidget = 0;
	m_updateColor = 0;
	m_update = true;
}

KisToolSelectPicker::~KisToolSelectPicker() 
{
}

void KisToolSelectPicker::buttonPress(KisButtonPressEvent *e)
{
	kdDebug() << "button press: " << m_subject << "\n";
	
	if (m_subject) {
		KisImageSP img;
		KisPaintDeviceSP dev;
		QPoint pos;
		QColor c;
		QUANTUM opacity;

		if (e -> button() != QMouseEvent::LeftButton && e -> button() != QMouseEvent::RightButton)
			return;

		if (!(img = m_subject -> currentImg()))
			return;

		dev = img -> activeDevice();

		if (!dev || !dev -> visible())
			return;

		pos = QPoint(e -> pos().floorX(), e -> pos().floorY());

		if (dev -> pixel(pos.x(), pos.y(), &c, &opacity))
			if(m_update)
				if (e -> button() == QMouseEvent::LeftButton)
					m_subject -> setFGColor(c);
				else 
					m_subject -> setBGColor(c);
	}
}

void KisToolSelectPicker::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("&Selection Picker"), "selectpicker", Qt::Key_E, this, SLOT(activate()), collection, name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

void KisToolSelectPicker::update(KisCanvasSubject *subject)
{
	super::update(subject);
	m_subject = subject;
}

void KisToolSelectPicker::slotSetFuzziness(int fuzziness)
{
	m_fuzziness = fuzziness;
}

void KisToolSelectPicker::slotSetAction(int action)
{
	m_selectAction = action;
}

QWidget* KisToolSelectPicker::createOptionWidget(QWidget* parent)
{
	m_optWidget = new QWidget(parent);
	m_optWidget -> setCaption(i18n("Selection Picker"));

	QVBoxLayout * l = new QVBoxLayout(m_optWidget);
	
	KisSelectionOptions * options = new KisSelectionOptions(parent, m_subject);
	l -> addWidget( options);
	connect (l, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));
	
	QHBoxLayout * hbox = new QHBoxLayout(l);
	
	QLabel * lbl = new QLabel(i18n("Fuzziness"), m_optWidget);
	hbox -> addWidget(lbl);
	
	KIntNumInput * input = new KIntNumInput(m_optWidget, "fuzziness");
	hbox -> addWidget(input);
	connect(input, SIGNAL(valueChanged(int)), this, SLOT(slotSetFuzziness(int)));

	m_optWidget -> setCaption(i18n("Selection Picker"));

	return m_optWidget;
}

QWidget* KisToolSelectPicker::optionWidget()
{
	return m_optWidget;
}

void KisToolSelectPicker::slotSetUpdateColor(bool state)
{
	m_update = state;
}
