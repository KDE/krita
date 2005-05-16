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
#include <qtimer.h>

#include <kapplication.h>
#include <kaction.h>
#include <klocale.h>
#include <knuminput.h>

#include <kis_cursor.h>
#include <kis_selection_manager.h>
#include <kis_canvas_subject.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_button_press_event.h>
#include <kis_canvas_subject.h>
#include <kis_selection_options.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_iterators_pixel.h>
#include <kis_color_utilities.h>
#include <kis_selected_transaction.h>
#include <kis_undo_adapter.h>

#include "kis_tool_selectpicker.h"

void selectByColor(KisPaintDeviceSP dev, KisSelectionSP selection, const QColor & c, int fuzziness, enumSelectionMode mode)
{
	// XXX: Multithread this!
	Q_INT32 x, y, w, h;

	QUANTUM opacity;
	dev -> exactBounds(x, y, w, h);

	KisStrategyColorSpaceSP cs = dev -> colorStrategy();
	KisProfileSP profile = dev -> profile();

	for (int y2 = y; y2 < h - y; ++y2) {
		KisHLineIterator hiter = dev -> createHLineIterator(x, y2, w, false);
		KisHLineIterator selIter = selection -> createHLineIterator(x, y2, w, true);
		while (!hiter.isDone()) {
			QColor c2;
			cs -> toQColor(hiter.rawData(), &c2, &opacity, profile);

			// Don't try to select transparent pixels. The Gimp has an option to match transparent pixels; we don't, for the moment.
			if (opacity > OPACITY_TRANSPARENT) {

				Q_UINT8 match = matchColors(c, c2, fuzziness);
				//kdDebug() << " Match: " << QString::number(match) << ", mode: " << mode << "\n";
				if (mode == SELECTION_ADD) {
					Q_UINT8 d = *(selIter.rawData());
					if (d + match > MAX_SELECTED) {
						*(selIter.rawData()) = MAX_SELECTED;
					}
					else {
						*(selIter.rawData()) = match + d;
					}

				}
				else if (mode == SELECTION_SUBTRACT) {
					Q_UINT8 selectedness = *(selIter.rawData());
					if (match < selectedness) {
						*(selIter.rawData()) = selectedness - match;
					}
					else {
						*(selIter.rawData()) = 0;
					}
				}
			}
			++hiter;
			++selIter;
		}
	}

}



KisToolSelectPicker::KisToolSelectPicker()
{
	setName("tool_selectpicker");
	setCursor(KisCursor::pickerCursor());
	m_subject = 0;
	m_optWidget = 0;
	m_fuzziness = 20;
	m_currentSelectAction = m_defaultSelectAction = SELECTION_ADD;
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), SLOT(slotTimer()) );
}

KisToolSelectPicker::~KisToolSelectPicker()
{
}

void KisToolSelectPicker::activate()
{
	KisToolNonPaint::activate();
	m_timer->start(50);
	setPickerCursor(m_currentSelectAction);
}

void KisToolSelectPicker::clear()
{
	m_timer->stop();
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

		KisSelectedTransaction *t = new KisSelectedTransaction(i18n("Selection Picker"),dev);

		dev -> pixel(pos.x(), pos.y(), &c, &opacity);
		kdDebug() << "Going to select colors similar to: " << c.red() << ", " << c.green() << ", "<< c.blue() << "\n";
		if (opacity > OPACITY_TRANSPARENT)
			selectByColor(dev, dev -> selection(), c, m_fuzziness, m_currentSelectAction);
		else
			m_subject -> selectionManager() -> selectAll();

		if(img -> undoAdapter())
			img -> undoAdapter() -> addCommand(t);
		m_subject -> canvasController() -> updateCanvas();

	}
}

void KisToolSelectPicker::slotTimer()
{
#if KDE_IS_VERSION(3,4,0)
	int state = kapp->keyboardMouseState() & (Qt::ShiftButton|Qt::ControlButton|Qt::AltButton);
#else
	int state = kapp->keyboardModifiers() & (KApplication::ShiftModifier
			|KApplication::ControlModifier|KApplication::Modifier1);
#endif
	enumSelectionMode action;

	if (state == Qt::ShiftButton)
		action = SELECTION_ADD;
	else if (state == Qt::ControlButton)
		action = SELECTION_SUBTRACT;
	else
		action = m_defaultSelectAction;

	if (action != m_currentSelectAction) {
		m_currentSelectAction = action;
		setPickerCursor(action);
	}
}

void KisToolSelectPicker::setPickerCursor(enumSelectionMode action)
{
	switch (action) {
		case SELECTION_ADD:
			m_subject -> setCanvasCursor(KisCursor::pickerPlusCursor());
			break;
		case SELECTION_SUBTRACT:
			m_subject -> setCanvasCursor(KisCursor::pickerMinusCursor());
	}
}

void KisToolSelectPicker::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Selection Picker"), "tool_picker_selection", Qt::Key_E, this, SLOT(activate()), collection, name());
		Q_CHECK_PTR(m_action);
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
	m_defaultSelectAction = (enumSelectionMode)action;
}

QWidget* KisToolSelectPicker::createOptionWidget(QWidget* parent)
{
	m_optWidget = new QWidget(parent);
	Q_CHECK_PTR(m_optWidget);

	m_optWidget -> setCaption(i18n("Selection Picker"));

	QVBoxLayout * l = new QVBoxLayout(m_optWidget);
	Q_CHECK_PTR(l);

	KisSelectionOptions * options = new KisSelectionOptions(m_optWidget, m_subject);
	Q_CHECK_PTR(options);

	l -> addWidget( options);
	connect (options, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));

	QHBoxLayout * hbox = new QHBoxLayout(l);
	Q_CHECK_PTR(hbox);

	QLabel * lbl = new QLabel(i18n("Fuzziness: "), m_optWidget);
	Q_CHECK_PTR(lbl);

	hbox -> addWidget(lbl);

	KIntNumInput * input = new KIntNumInput(m_optWidget, "fuzziness");
	Q_CHECK_PTR(input);

	input -> setRange(0, 200, 10, true);
	input -> setValue(20);
	hbox -> addWidget(input);
	connect(input, SIGNAL(valueChanged(int)), this, SLOT(slotSetFuzziness(int)));

	return m_optWidget;
}

QWidget* KisToolSelectPicker::optionWidget()
{
	return m_optWidget;
}

#include "kis_tool_selectpicker.moc"
