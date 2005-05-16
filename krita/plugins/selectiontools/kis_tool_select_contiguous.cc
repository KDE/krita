/*
 *  kis_tool_select_contiguous - part of Krayon^WKrita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <qpainter.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qapplication.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>
#include <kcolorbutton.h>

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
#include <kis_selection_options.h>
#include <kis_canvas_observer.h>
#include <kis_fill_painter.h>
#include <kis_undo_adapter.h>
#include <kis_selected_transaction.h>

#include "kis_tool_select_contiguous.h"

KisToolSelectContiguous::KisToolSelectContiguous() : super()
{
	setName("tool_select_contiguous");
	m_subject = 0;
	m_optWidget = 0;
	m_options = 0;
	m_fuzziness = 20;
	m_selectAction = SELECTION_ADD;

	//XXX : make wizard cursor from tool icon.
	setCursor(KisCursor::arrowCursor());
}

KisToolSelectContiguous::~KisToolSelectContiguous()
{
}

void KisToolSelectContiguous::buttonPress(KisButtonPressEvent * e)
{
	if (m_subject) {
		KisImageSP img;
		KisPaintDeviceSP dev;
		QPoint pos;

		if (e -> button() != QMouseEvent::LeftButton && e -> button() != QMouseEvent::RightButton)
			return;

		if (!(img = m_subject -> currentImg()))
			return;

		dev = img -> activeDevice();

		if (!dev || !dev -> visible())
			return;


		pos = QPoint(e -> pos().floorX(), e -> pos().floorY());
		QCursor oldCursor = m_subject -> setCanvasCursor(KisCursor::waitCursor());

		KisFillPainter fillpainter(dev);
		fillpainter.setFillThreshold(m_fuzziness);
		KisSelectionSP selection = fillpainter.createFloodSelection(pos.x(), pos.y());

		QColor c = m_options -> maskColor();
		if (c.isValid())
			selection -> setMaskColor(c);
		
		KisSelectedTransaction *t = new KisSelectedTransaction(i18n("Select Contiguous Areas"), dev.data());
		
		if (!dev -> hasSelection()) {
			dev->selection()->clear();
		}
		
		switch (m_selectAction) {
			case SELECTION_ADD:
				dev -> addSelection(selection);
				break;
			case SELECTION_SUBTRACT:
				dev -> subtractSelection(selection);
				break;
			default: kdDebug() << "KisToolSelectContiguous: invalid select action: " << m_selectAction << endl;
		}

		KisUndoAdapter *adapter = img -> undoAdapter();
		if (adapter)
			adapter -> addCommand(t);
			
		m_subject -> setCanvasCursor(oldCursor);
		m_subject -> canvasController() -> updateCanvas();
	}

}

void KisToolSelectContiguous::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Contiguous Select"),
					    "tool_wizard_selection" ,
					    0,
					    this,
					    SLOT(activate()),
					    collection,
					    name());
		Q_CHECK_PTR(m_action);
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

void KisToolSelectContiguous::update(KisCanvasSubject *subject)
{
	super::update(subject);
	m_subject = subject;
}

void KisToolSelectContiguous::slotSetFuzziness(int fuzziness)
{
	m_fuzziness = fuzziness;
}


void KisToolSelectContiguous::slotSetAction(int action)
{
	if (action >= SELECTION_ADD && action <= SELECTION_SUBTRACT)
		m_selectAction =(enumSelectionMode)action;
// XXX: Fix cursors when then are done.
// 	switch(m_selectAction) {
// 		case SELECTION_ADD:
// 			m_subject -> setCanvasCursor(KisCursor::pickerPlusCursor());
// 			break;
// 		case SELECTION_SUBTRACT:
// 			m_subject -> setCanvasCursor(KisCursor::pickerMinusCursor());
// 	};
}


QWidget* KisToolSelectContiguous::createOptionWidget(QWidget* parent)
{
	m_optWidget = new QWidget(parent);
	Q_CHECK_PTR(m_optWidget);
	m_optWidget -> setCaption(i18n("Select Contiguous Areas"));

	QVBoxLayout * l = new QVBoxLayout(m_optWidget);
	Q_CHECK_PTR(l);

	m_options = new KisSelectionOptions(m_optWidget, m_subject);
	Q_CHECK_PTR(m_options);

	l -> addWidget( m_options);
	connect (m_options, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));

	QHBoxLayout * hbox = new QHBoxLayout(l);
	Q_CHECK_PTR(hbox);

	QLabel * lbl = new QLabel(i18n("Fuzziness: "), m_optWidget);
	hbox -> addWidget(lbl);

	KIntNumInput * input = new KIntNumInput(m_optWidget, "fuzziness");
	Q_CHECK_PTR(input);

	input -> setRange(0, 200, 10, true);
	input -> setValue(20);
	hbox -> addWidget(input);
	connect(input, SIGNAL(valueChanged(int)), this, SLOT(slotSetFuzziness(int)));

	return m_optWidget;
}

QWidget* KisToolSelectContiguous::optionWidget()
{
        return m_optWidget;
}

#include "kis_tool_select_contiguous.moc"
