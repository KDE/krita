/*
 *  kis_tool_brush.cc - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_painter.h"
#include "kis_tool_freehand.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_view.h"
#include "integerwidget.h"
#include "kis_cmb_composite.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"

KisToolFreeHand::KisToolFreeHand(QString transactionText)
		: super(),
		m_dragDist ( 0 ),
		m_transactionText(transactionText),
		m_mode( HOVER )
{
	m_painter = 0;
	m_currentImage = 0;
	m_optWidget = 0;

	m_lbOpacity = 0;
	m_slOpacity = 0;
	m_lbComposite= 0;
	m_cmbComposite = 0;

	m_opacity = OPACITY_OPAQUE;
	m_compositeOp = COMPOSITE_OVER;
}

KisToolFreeHand::~KisToolFreeHand()
{
}

void KisToolFreeHand::update(KisCanvasSubject *subject)
{
	super::update(subject);
	m_currentImage = m_subject -> currentImg();
}

void KisToolFreeHand::buttonPress(KisButtonPressEvent *e)
{
        if (!m_subject) return;

        if (!m_subject -> currentBrush()) return;

	if (!m_currentImage || !m_currentImage -> activeDevice()) return;

        if (e -> button() == QMouseEvent::LeftButton) {

		initPaint(e);

		paintAt(e -> pos(), e -> pressure(), e -> xTilt(), e -> yTilt());

		m_prevPos = e -> pos();
		m_prevPressure = e -> pressure();
		m_prevXTilt = e -> xTilt();
		m_prevYTilt = e -> yTilt();

		m_currentImage -> notify(m_painter -> dirtyRect());
         }
}

void KisToolFreeHand::buttonRelease(KisButtonReleaseEvent* e)
{
	if (e -> button() == QMouseEvent::LeftButton && m_mode == PAINT) {
		endPaint();
        }
}

void KisToolFreeHand::move(KisMoveEvent *e)
{
	if (m_mode == PAINT) {
		paintLine(m_prevPos, m_prevPressure, m_prevXTilt, m_prevYTilt, e -> pos(), e -> pressure(), e -> xTilt(), e -> yTilt());

		m_prevPos = e -> pos();
		m_prevPressure = e -> pressure();
		m_prevXTilt = e -> xTilt();
		m_prevYTilt = e -> yTilt();

		m_currentImage -> notify(m_painter -> dirtyRect());
	}
}

void KisToolFreeHand::initPaint(KisEvent *)
{
	if (!m_currentImage || !m_currentImage -> activeDevice()) return;

	m_mode = PAINT;
	m_dragDist = 0;

	// Create painter
	KisPaintDeviceSP device;
	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
		if (m_painter)
			delete m_painter;
		m_painter = new KisPainter( device );
		m_painter -> beginTransaction(m_transactionText);
	}

	m_painter -> setPaintColor(m_subject -> fgColor());
	m_painter -> setBackgroundColor(m_subject -> bgColor());
	m_painter -> setBrush(m_subject -> currentBrush());
	m_painter -> setOpacity(m_opacity);
	m_painter -> setCompositeOp(m_compositeOp);

	// Set the cursor -- ideally. this should be a mask created from the brush,
	// now that X11 can handle colored cursors.
#if 0
	// Setting cursors has no effect until the tool is selected again; this
	// should be fixed.
	setCursor(KisCursor::brushCursor());
#endif
}

void KisToolFreeHand::endPaint() 
{
	m_mode = HOVER;
	KisPaintDeviceSP device;
	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
		KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
		if (adapter && m_painter) {
			// If painting in mouse release, make sure painter
			// is destructed or end()ed
			adapter -> addCommand(m_painter->endTransaction());
		}
		delete m_painter;
		m_painter = 0;
		notifyModified();
	}
}

QWidget* KisToolFreeHand::createoptionWidget(QWidget* parent)
{
	m_optWidget = new QWidget(parent);
	m_optWidget -> setCaption(i18n("Brush"));
	
	m_lbOpacity = new QLabel(i18n("Opacity: "), m_optWidget);
	m_slOpacity = new IntegerWidget( 0, 100, m_optWidget, "int_widget");
	m_slOpacity -> setTickmarks(QSlider::Below);
	m_slOpacity -> setTickInterval(10);
	m_slOpacity -> setValue(m_opacity / OPACITY_OPAQUE * 100);
	connect(m_slOpacity, SIGNAL(valueChanged(int)), this, SLOT(slotSetOpacity(int)));

	m_lbComposite = new QLabel(i18n("Mode: "), m_optWidget);
	m_cmbComposite = new KisCmbComposite(m_optWidget);
	connect(m_cmbComposite, SIGNAL(activated(int)), this, SLOT(slotSetCompositeMode(int)));

	QGridLayout *optionLayout = new QGridLayout(m_optWidget, 4, 2);

	optionLayout -> addWidget(m_lbOpacity, 1, 0);
	optionLayout -> addWidget(m_slOpacity, 1, 1);

	optionLayout -> addWidget(m_lbComposite, 2, 0);
	optionLayout -> addWidget(m_cmbComposite, 2, 1);

	return m_optWidget;
}

QWidget* KisToolFreeHand::optionWidget()
{
	return m_optWidget;
}

void KisToolFreeHand::slotSetOpacity(int opacityPerCent)
{
	m_opacity = opacityPerCent * OPACITY_OPAQUE / 100;
}

void KisToolFreeHand::slotSetCompositeMode(int compositeOp)
{
	m_compositeOp = (CompositeOp)compositeOp;
}

KisImageSP KisToolFreeHand::currentImage()
{
	return m_currentImage;
}

#include "kis_tool_freehand.moc"

