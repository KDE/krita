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

#include <kis_cursor.h>
#include <kis_canvas_subject.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_button_press_event.h>
#include <kis_canvas_subject.h>
#include <kis_selection_options.h>
#include <kis_selection.h>
#include <kis_conversions.h>
#include <kis_paint_device.h>
#include <kis_iterators_pixel.h>

#include "kis_tool_selectpicker.h"


Q_UINT8 matchColors(const QColor & c, const QColor & c2, Q_UINT8 fuzziness)
{
#if 1
	// XXX: Is it enough to compare just hue, or should we compare saturation
	// and value too?
	int h1, s1, v1, h2, s2, v2;
	rgb_to_hsv(c.red(), c.green(), c.blue(), &h1, &s1, &v1);
	rgb_to_hsv(c2.red(), c2.green(), c2.blue(), &h2, &s2, &v2);

	//kdDebug() << "Hue 1: " << h1 << ", hue 2: " << h2 << "\n";

	int diff = QMAX(QABS(v1 - v2), QMAX(QABS(s1 - s2), QABS(h1 - h2)));

	if (diff > fuzziness) return 0;

	if (diff == 0) return 255;

	return 255 - (diff / fuzziness * 255);
#else //XXX: See doc/colordiff for this formulate. I don't know how to map the long values to a 0-255 range.
	long r,g,b;
	long rmean;

	rmean = ( (int)c.red() + (int)c2.red() ) / 2;
	
	r = (int)c.red() - (int)c2.red();
	g = (int)c.green() - (int)c2.green();
	b = (int)c.blue() - (int)c2.blue();

	long diff = (((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8);
					    
        if (diff > fuzziness) return 0;

        if (diff == 0,0) return 255;

	kdDebug() << "Diff: " << QString::number(diff) << "\n";
	return (Q_UINT8) diff;
	
#endif
}

void selectByColor(KisPaintDeviceSP dev, KisSelectionSP selection, const QColor & c, int fuzziness, enumSelectionMode mode)
{
	// XXX: Multithread this!
	Q_INT32 x, y, w, h;
	dev -> exactBounds(x, y, w, h);
	
	KisStrategyColorSpaceSP cs = dev -> colorStrategy();
	KisProfileSP profile = dev -> profile();
	
	for (int y2 = y; y2 < h - y; ++y2) {
		KisHLineIterator hiter = dev -> createHLineIterator(x, y2, w, false);
		KisHLineIterator selIter = selection -> createHLineIterator(x, y2, w, true);
		while (!hiter.isDone()) {
			// Clean up as we go, if necessary
			if (mode == SELECTION_REPLACE) memset (selIter.rawData(), 0, 1); // Selections are hard-coded one byte big.
	
			QColor c2;
			
			cs -> toQColor(hiter.rawData(), &c2, profile);

 			Q_UINT8 match = matchColors(c, c2, fuzziness);
			//kdDebug() << " Match: " << QString::number(match) << ", mode: " << mode << "\n";
			if (mode == SELECTION_REPLACE) {
				*(selIter.rawData()) =  match;
			}
			else if (mode == SELECTION_ADD) { 
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
	m_fuzziness = 100;
	m_selectAction = SELECTION_REPLACE;
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

		dev -> pixel(pos.x(), pos.y(), &c, &opacity);
		kdDebug() << "Going to select colors similar to: " << c.red() << ", " << c.green() << ", "<< c.blue() << "\n";
		selectByColor(dev, dev -> selection(), c, m_fuzziness, m_selectAction);
		m_subject -> canvasController() -> updateCanvas();
		
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
	m_selectAction =(enumSelectionMode)action;
}

QWidget* KisToolSelectPicker::createOptionWidget(QWidget* parent)
{
	m_optWidget = new QWidget(parent);
	m_optWidget -> setCaption(i18n("Selection Picker"));

	QVBoxLayout * l = new QVBoxLayout(m_optWidget);
	
	KisSelectionOptions * options = new KisSelectionOptions(m_optWidget, m_subject);
	l -> addWidget( options);
	connect (options, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));
	
	QHBoxLayout * hbox = new QHBoxLayout(l);
	
	QLabel * lbl = new QLabel(i18n("Fuzziness: "), m_optWidget);
	hbox -> addWidget(lbl);
	
	KIntNumInput * input = new KIntNumInput(m_optWidget, "fuzziness");
	input -> setRange(0, 200, 10, true);
	input -> setValue(20);
	hbox -> addWidget(input);
	connect(input, SIGNAL(valueChanged(int)), this, SLOT(slotSetFuzziness(int)));

	m_optWidget -> setCaption(i18n("Selection Picker"));

	return m_optWidget;
}

QWidget* KisToolSelectPicker::optionWidget()
{
	return m_optWidget;
}

#include "kis_tool_selectpicker.moc"
