/*
 *  kis_tool_fill.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcommand.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <koColor.h>

#include "kis_layer.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_view.h"
#include "kis_tool_brush.h"
#include "integerwidget.h"
#include "kis_cmb_composite.h"
#include "kis_tool_fill.h"
#include "kis_iterators_pixel.h"
#include "color_strategy/kis_strategy_colorspace.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_pattern.h"
#include "kis_iterators_infinite.h"

KisToolFill::KisToolFill() 
	: super()
{
	setName("tool_fill");
	m_subject = 0;
	m_oldColor = 0;
	m_threshold = 15;
	m_usePattern = false;
	m_samplemerged = false;

	// set custom cursor.
	setCursor(KisCursor::fillerCursor());
}

void KisToolFill::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_currentImage = subject -> currentImg();

	super::update(m_subject);
}

KisToolFill::~KisToolFill() 
{
}

bool KisToolFill::flood(int startX, int startY)
{
	m_lay = m_currentImage->activeLayer();
	m_depth = m_lay->depth();
	m_ktc = new KisTileCommand("Fill", (KisPaintDeviceSP)m_lay );
	m_color = new QUANTUM[m_depth];
	m_map = new bool[m_lay->width()*m_lay->height()];
	for (int i = 0; i < m_lay->width()*m_lay->height(); i++)
		m_map[i] = false;

	// opacity?
	m_lay->colorStrategy()->nativeColor(m_subject -> fgColor(), QUANTUM_MAX, m_color);
	// set up the 'cached' color iterator if necessary, speeds things up...
	if (!m_usePattern)
		m_replaceWithIt = new KisIteratorInfinitePixel(m_lay->colorStrategy(), m_color);

	floodLine(startX, startY);

	if (!m_usePattern)
		delete m_replaceWithIt;

	m_currentImage->undoAdapter()->addCommand( m_ktc );
	m_currentImage->notify();

	delete[] m_color;
	delete[] m_oldColor; m_oldColor = 0;
	delete[] m_map;
	return true;
}

/* RGB-only I fear */
QUANTUM KisToolFill::difference(QUANTUM* src, KisPixelRepresentation dst, QUANTUM threshold, int depth)
{
	QUANTUM max = 0, diff = 0;
	for (int i = 0; i < depth; i++) {
		// added extra (QUANTUM) casts just to be on the safe side until that is fixed
		diff = QABS((QUANTUM)src[i] - (QUANTUM)dst[i]);
		if (diff > max)
			max = diff;
	}
	return (max > threshold) ? 255 : 0;
}

int KisToolFill::floodSegment(int x, int y, int most, KisIteratorPixel* src, KisIteratorPixel* it, KisIteratorPixel* lastPixel, Direction d) {
	bool stop = false;

	while( ( ( d == Right && *it <= *lastPixel) || (d == Left && *lastPixel <= *it)) && !stop)
	{
		KisPixelRepresentation data = *it;
		KisPixelRepresentation source = *src;
		if (difference(m_oldColor, data, m_threshold, m_depth) == 0) {
			for( int i = 0; i < m_depth; i++)
			{
				data[i] = (QUANTUM) source[i]; // explicit (QUANTUM) cast to prevent weirdness
			}
			m_map[y*m_lay->width()+x] = true;
			if (d == Right) {
				it->inc();
				src->inc();
				x++; most++;
			} else {
				it->dec();
				src->dec();
				x--; most--;
			}
		} else {
			stop = true;
		}
	}
	
	return most;
}

void KisToolFill::floodLine(int x, int y) {
	int mostRight, mostLeft = x;
	
	KisIteratorLinePixel lineIt = m_lay->iteratorPixelSelectionBegin( m_ktc, x,-1, y);

	KisIteratorPixel pixelIt = *lineIt;
	KisIteratorPixel lastPixel = lineIt.end();

	if (m_usePattern) {
		m_replaceWithIt = new KisIteratorInfinitePixel(
			(KisPaintDeviceSP)(m_subject->currentPattern()->image(m_lay->colorStrategy())),
			0, y, x );
	}

	if (!m_oldColor) {
		m_oldColor = new QUANTUM[m_depth];
		for (int i = 0; i < m_depth; i++)
			m_oldColor[i] = pixelIt[i];
	} else {
		if (difference(m_oldColor, pixelIt, m_threshold, m_depth) != 0)
			return;
	}

	mostRight = floodSegment(x, y, x, m_replaceWithIt, &pixelIt, &lastPixel, Right);
	
	if (lastPixel < pixelIt) mostRight--;
	
	delete lineIt;

	if (x > 0) {
		mostLeft--;
		if (m_usePattern) {
			delete m_replaceWithIt;
			m_replaceWithIt = new KisIteratorInfinitePixel(
				(KisPaintDeviceSP)(m_subject->currentPattern()->image(m_lay->colorStrategy())), 0, y, x - 1);
		}
		KisIteratorLinePixel lineIt2 = m_lay->iteratorPixelSelectionBegin(m_ktc, 0,x-1, y);
		KisIteratorPixel lastPixel = lineIt2.begin();
		KisIteratorPixel pixelIt = lineIt2.end();
	
		mostLeft = floodSegment(x,y, mostLeft, m_replaceWithIt, &pixelIt, &lastPixel, Left);
	
		if (pixelIt < lastPixel)
			mostLeft++;
		delete lineIt;
	}
	
	if (m_usePattern)
		delete m_replaceWithIt;

	// yay for stack overflowing:
	for (int i = mostLeft; i <= mostRight; i++) {
		if (y > 0 && ! m_map[(y-1)*m_lay->width()+i])
			floodLine(i, y-1);
		if (y < m_lay->height() - 1 && ! m_map[(y+1)*m_lay->width()+i])
			floodLine(i, y+1);
	}
}

void KisToolFill::buttonPress(KisButtonPressEvent *e)
{
	if (!m_subject) return;
	if (!m_currentImage || !m_currentImage -> activeDevice()) return;
	if (e->button() != QMouseEvent::LeftButton) return;
	
	flood(e -> pos().floorX(), e -> pos().floorY());
	notifyModified();
}

QWidget* KisToolFill::createoptionWidget(QWidget* parent)
{
	m_optWidget = new QWidget(parent);
	m_optWidget -> setCaption(i18n("Fill"));
	
	m_lbThreshold = new QLabel(i18n("Threshold: "), m_optWidget);
	m_slThreshold = new IntegerWidget( 0, 255, m_optWidget, "int_widget");
	m_slThreshold -> setTickmarks(QSlider::Below);
	m_slThreshold -> setTickInterval(32);
	m_slThreshold -> setValue(m_threshold);
	connect(m_slThreshold, SIGNAL(valueChanged(int)), this, SLOT(slotSetThreshold(int)));

	m_lbComposite = new QLabel(i18n("Mode: "), m_optWidget);
	m_cmbComposite = new KisCmbComposite(m_optWidget);
	connect(m_cmbComposite, SIGNAL(activated(int)), this, SLOT(slotSetCompositeMode(int)));

	m_checkUsePattern = new QCheckBox("Use pattern", m_optWidget);
	m_checkUsePattern->setChecked(m_usePattern);
	connect(m_checkUsePattern, SIGNAL(stateChanged(int)), this, SLOT(slotSetUsePattern(int)));

	QGridLayout *optionLayout = new QGridLayout(m_optWidget, 4, 3);

	optionLayout -> addWidget(m_lbThreshold, 1, 0);
	optionLayout -> addWidget(m_slThreshold, 1, 1);

	optionLayout -> addWidget(m_lbComposite, 2, 0);
	optionLayout -> addWidget(m_cmbComposite, 2, 1);

	optionLayout -> addWidget(m_checkUsePattern, 3, 0);

	return m_optWidget;
}

QWidget* KisToolFill::optionWidget()
{
	return m_optWidget;
}

void KisToolFill::slotSetThreshold(int threshold)
{
	m_threshold = threshold;
}

void KisToolFill::slotSetCompositeMode(int compositeOp)
{
	m_compositeOp = (CompositeOp)compositeOp;
}

void KisToolFill::slotSetUsePattern(int state)
{
	if (state == QButton::NoChange)
		return;
	m_usePattern = (state == QButton::On);
}

void KisToolFill::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Fill"), 
					    "fill",
					    Qt::Key_F, 
					    this, 
					    SLOT(activate()),
					    collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

#include "kis_tool_fill.moc"
