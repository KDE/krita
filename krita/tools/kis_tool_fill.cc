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

KisToolFill::KisToolFill() 
	: super()
{
	setName("tool_fill");
	m_subject = 0;
	m_oldColor = 0;
	m_threshold = 15;
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
	KisLayerSP lay = m_currentImage->activeLayer();
	Q_INT32 depth = ::imgTypeDepth( lay->typeWithoutAlpha() );
	KisTileCommand* ktc = new KisTileCommand("Fill", (KisPaintDeviceSP)lay );
	QUANTUM* color = new QUANTUM[depth];
	m_map = new bool[lay->width()*lay->height()];
	for (int i = 0; i < lay->width()*lay->height(); i++)
		m_map[i] = false;

	// opacity?
	lay->colorStrategy()->nativeColor(m_subject -> fgColor(), color);

	floodLine(startX, startY, depth, lay, ktc, color);

	m_currentImage->undoAdapter()->addCommand( ktc );
	m_currentImage->notify();

	delete[] color;
	delete[] m_oldColor; m_oldColor = 0;
	delete[] m_map;
	return true;
}

/* RGB-only I fear */
QUANTUM KisToolFill::difference(QUANTUM* src, KisPixelRepresentation dst, QUANTUM threshold, int depth)
{
	QUANTUM max = 0, diff = 0;
	for (int i = 0; i < depth; i++) {
		diff = QABS(src[i] - dst[i]);
		if (diff > max)
			max = diff;
	}
	return (diff > threshold) ? 255 : 0;
}

void KisToolFill::floodLine(int x, int y, Q_INT32 depth, KisLayerSP lay,
							KisTileCommand* ktc, QUANTUM* color) {
	bool stop = false;
	int mostRight = x, mostLeft = x;
	
	KisIteratorLinePixel lineIt = lay->iteratorPixelSelectionBegin( ktc, x,-1, y);

	KisIteratorPixel pixelIt = *lineIt;
	KisIteratorPixel lastPixel = lineIt.end();
	if (!m_oldColor) {
		m_oldColor = new QUANTUM[depth];
		for (int i = 0; i < depth; i++)
			m_oldColor[i] = pixelIt[i];
	} else {
		if (difference(m_oldColor, pixelIt, m_threshold, depth) != 0)
			return;
	}

	while( pixelIt <= lastPixel && !stop)
	{
		KisPixelRepresentation data = pixelIt;
		if (difference(m_oldColor, data, m_threshold, depth) == 0) {
			for( int i = 0; i < depth; i++)
			{
				data[i] = color[i];
			}
			m_map[y*lay->width()+mostRight] = true;
			mostRight++;
		} else {
			stop = true;
		}
		++pixelIt;
	}
	if (lastPixel < pixelIt) mostRight--;
	stop = false;
	delete lineIt;
	if (x > 0) {
		KisIteratorLinePixel lineIt2 = lay->iteratorPixelSelectionBegin(ktc, 0,x-1, y);
		KisIteratorPixel lastPixel = lineIt2.begin();
		KisIteratorPixel pixelIt = lineIt2.end();
		if (difference(m_oldColor, pixelIt, m_threshold, depth) == 0) {
			mostLeft--;
			while( lastPixel <= pixelIt && !stop)
			{
				KisPixelRepresentation data = pixelIt;
				if (difference(m_oldColor, data, m_threshold, depth) == 0) {
					for( int i = 0; i < depth; i++)
					{
						data[i] = color[i];
					}
					m_map[y*lay->width()+mostLeft] = true;
					mostLeft--;
				} else {
					stop = true;
				}
				--pixelIt;
			}
			if (pixelIt < lastPixel) mostLeft++;
		}
		delete lineIt;
	}

	for (int i = mostLeft; i <= mostRight; i++) {
		if (y > 0 && ! m_map[(y-1)*lay->width()+i])
			floodLine(i, y-1, depth, lay, ktc, color);
		if (y < lay->height() - 1 && ! m_map[(y+1)*lay->width()+i])
			floodLine(i, y+1, depth, lay, ktc, color);
	}
}

void KisToolFill::buttonPress(KisButtonPressEvent *e)
{
	if (!m_subject) return;
	if (!m_currentImage -> activeDevice()) return;
	if (e->button() != QMouseEvent::LeftButton) return;

	flood(e -> pos().floorX(), e -> pos().floorY());
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

	QGridLayout *optionLayout = new QGridLayout(m_optWidget, 4, 2);

	optionLayout -> addWidget(m_lbThreshold, 1, 0);
	optionLayout -> addWidget(m_slThreshold, 1, 1);

	optionLayout -> addWidget(m_lbComposite, 2, 0);
	optionLayout -> addWidget(m_cmbComposite, 2, 1);

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

void KisToolFill::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("&Filler Tool"), 
					    "fill",
					    0, 
					    this, 
					    SLOT(activate()),
					    collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

#include "kis_tool_fill.moc"
