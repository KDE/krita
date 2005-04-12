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

#include "kis_tool_select_contiguous.h"

KisToolSelectContiguous::KisToolSelectContiguous() : super()
{
	setName("tool_select_contiguous");
	m_subject = 0;
	m_optWidget = 0;
	m_options = 0;
	m_fuzziness = 20;

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
		fillSelection(dev, m_selectAction, pos.x(), pos.y());
		m_subject -> setCanvasCursor(oldCursor);
		m_subject -> canvasController() -> updateCanvas();

	}

}


void KisToolSelectContiguous::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Contiguous Select"),
					    "select_wizard" ,
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
	m_selectAction =(enumSelectionMode)action;
// XXX: Fix cursors when then are done.
// 	switch(m_selectAction) {
// 		case SELECTION_REPLACE:
// 			m_subject -> setCanvasCursor(KisCursor::pickerCursor());
// 			break;
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

// flood filling

void KisToolSelectContiguous::fillSelection(KisPaintDeviceSP device, enumSelectionMode mode, int startX, int startY)
{

	m_device = device;

	if (device -> hasSelection()) {
		//if (device -> selection() -> selected(startX, startY) > MIN_SELECTED)
		//	return;

		if (mode == SELECTION_REPLACE)
			device -> removeSelection();
	}


	m_selection = device -> selection();
	QColor c = m_options -> maskColor();
	if (c.isValid())
		m_selection -> setMaskColor(c);

	m_depth = device -> pixelSize();
	m_colorChannels = device -> colorStrategy() -> nColorChannels();

	Q_INT32 x, y;
	device -> exactBounds(x, y, m_width, m_height);

	m_size = m_width * m_height;

	m_oldColor = new QUANTUM[m_depth];
	Q_CHECK_PTR(m_oldColor);

	KisHLineIteratorPixel pixelIt = m_device -> createHLineIterator(startX, startY, startX + 1, false);
	memcpy(m_oldColor, pixelIt.rawData(), m_depth);

	m_map = new bool[m_size];
	Q_CHECK_PTR(m_map);

	for (int i = 0; i < m_size; i++)
		m_map[i] = false;


	floodLine(startX, startY, mode);

	delete m_map;

	delete m_oldColor;

}

void KisToolSelectContiguous::floodLine(int x, int y, enumSelectionMode mode)
{

	qApp -> processEvents();

	int mostRight, mostLeft = x;


	KisHLineIteratorPixel pixelIt = m_device -> createHLineIterator(x, y, m_width, false);

	int lastPixel = m_width;

	if (difference(m_oldColor, pixelIt.rawData()) > m_fuzziness) {
		return;
	}

	mostRight = floodSegment(x, y, x, pixelIt, lastPixel, Right, mode);

	if (lastPixel < pixelIt.x())
		mostRight--;

	if (x > 0) {
		mostLeft--;

		KisHLineIteratorPixel pixelIt = m_device->createHLineIterator(x - 1, y, m_width - 1, false);
		int lastPixel = 0;

		mostLeft = floodSegment(x,y, mostLeft, pixelIt, lastPixel, Left, mode);

		if (pixelIt.x() < lastPixel)
			mostLeft++;
	}

	// yay for stack overflowing:
	for (int i = mostLeft; i <= mostRight; i++) {
		qApp -> processEvents();
		if (y > 0 && !m_map[(y-1)*m_width + i])
			floodLine(i, y-1, mode);
		if (y < m_height - 1 && !m_map[(y+1)*m_width + i])
			floodLine(i, y+1, mode);
	}
}

int KisToolSelectContiguous::floodSegment(int x, int y, int most, KisHLineIteratorPixel& it, int lastPixel, Direction d, enumSelectionMode mode)
{
	bool stop = false;
	QUANTUM diff;
	KisHLineIteratorPixel selIter = m_selection -> createHLineIterator(x, y, m_width - x, true);
	KisStrategyColorSpaceSP colorStrategy = m_selection -> colorStrategy();
	while( ( ( d == Right && it.x() <= lastPixel) || (d == Left && lastPixel <= it.x())) && !stop)
	{

		if (m_map[y * m_width + x])
			break;
		m_map[y * m_width + x] = true;

		KisPixel data = it.pixel();
		diff = difference(m_oldColor, data);
		if (diff < m_fuzziness) {
			//kdDebug() << "Diff: " << QString::number(diff) << ", fuzz: " << m_fuzziness << "\n";
			Q_UINT8 selectedness = selIter.rawData()[0];

			if (mode == SELECTION_ADD || mode == SELECTION_REPLACE) {

				selIter.rawData()[0] = MAX_SELECTED;// - diff;
			} else if (mode == SELECTION_SUBTRACT) {
				if (selectedness > diff) {
					selIter.rawData()[0] = selIter.rawData()[0] - diff;
				} else {
					selIter.rawData()[0] = MIN_SELECTED;
				}
			}

			if (d == Right) {
				++it;
				++selIter;
				x++;
				most++;
			} else {
				//XXX: Iterator decrement has not been implemented
				// for the tile iterators so this was broken. Add
				// decrement operators for this case. AP
				/*
				it--; selection--;
				*/
				x--; most--;
			}
		} else {
			stop = true;
		}
	}

	return most;
}

QUANTUM KisToolSelectContiguous::difference(const QUANTUM* src, KisPixel dst)
{
	QUANTUM max = 0, diff = 0;

	for (int i = 0; i < m_colorChannels; i++) {
		// added extra (QUANTUM) casts just to be on the safe side until that is fixed
		diff = QABS((QUANTUM)src[i] - (QUANTUM)dst[i]);
		if (diff > max)
			max = diff;
	}
	return max;
}

#include "kis_tool_select_contiguous.moc"
