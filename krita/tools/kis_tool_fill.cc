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
#include "kis_selection.h"
#include "kis_fill_painter.h"

KisToolFill::KisToolFill() 
	: super()
{
	setName("tool_fill");
	m_subject = 0;
	m_oldColor = 0;
	m_threshold = 15;
	m_usePattern = false;
	m_compositeOp = COMPOSITE_OVER;
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
	KisPaintDeviceSP device = m_currentImage->activeDevice();

	KisFillPainter painter(device);
	painter.beginTransaction(i18n("Floodfill"));
	painter.setPaintColor(m_subject -> fgColor());
	painter.setOpacity(OPACITY_OPAQUE); // XXX: make a selector for this?
	painter.setFillThreshold(m_threshold);
	painter.setCompositeOp(m_compositeOp);
	painter.setPattern(*(m_subject -> currentPattern()));
	if (m_usePattern)
		painter.fillPattern(startX, startY);
	else
		painter.fillColor(startX, startY);

	m_currentImage -> notify();
	notifyModified();

	KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
	if (adapter) {
		adapter -> addCommand(painter.endTransaction());
	}

	return true;
}

void KisToolFill::buttonPress(KisButtonPressEvent *e)
{
	if (!m_subject) return;
	if (!m_currentImage || !m_currentImage -> activeDevice()) return;
	if (e->button() != QMouseEvent::LeftButton) return;
	int x, y;
	x = e -> pos().floorX();
	y= e -> pos().floorY();
	if (x >= m_currentImage -> activeDevice() -> width() || x < 0) return;
	if (y >= m_currentImage -> activeDevice() -> height() || y < 0) return;
	flood(x, y);
	notifyModified();
}

QWidget* KisToolFill::createOptionWidget(QWidget* parent)
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
					    "color_fill",
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
