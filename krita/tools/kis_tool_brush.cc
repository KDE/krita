/*
 *  kis_tool_brush.cc - part of Kria
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

#include "kis_cursor.h"
#include "kis_dlg_toolopts.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_view.h"
#include "kis_tool_brush.h"
#include "integerwidget.h"
#include "kis_cmb_composite.h"
#include "kis_brush.h"


KisToolBrush::KisToolBrush()
        : super(),
          m_mode( HOVER ),
	  m_dragDist ( 0 )
{
	setCursor(KisCursor::brushCursor());

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

KisToolBrush::~KisToolBrush()
{
}

void KisToolBrush::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_currentImage = subject -> currentImg();

	super::update(m_subject);
}

void KisToolBrush::mousePress(QMouseEvent *e)
{
        if (!m_subject) return;

        if (!m_subject->currentBrush()) return;

	if (!m_currentImage -> activeDevice()) return;

        if (e->button() == QMouseEvent::LeftButton) {
		m_mode = PAINT;
		initPaint(e -> pos());
		m_painter -> paintAt(e->pos(), PRESSURE_DEFAULT, 0, 0);
		// XXX: get the rect that should be notified
		m_currentImage -> notify( m_painter -> dirtyRect() );
         }
}


void KisToolBrush::mouseRelease(QMouseEvent* e)
{
	if (e->button() == QMouseEvent::LeftButton && m_mode == PAINT) {
		endPaint();
        }
}


void KisToolBrush::mouseMove(QMouseEvent *e)
{
	if (m_mode == PAINT) {
		paintLine(m_dragStart, e -> pos(), PRESSURE_DEFAULT, 0, 0);
	}
}

void KisToolBrush::tabletEvent(QTabletEvent *e)
{
         if (e->device() == QTabletEvent::Stylus) {
		 if (!m_currentImage -> activeDevice()) {
			 e -> accept();
			 return;
		 }

		 if (!m_subject) {
			 e -> accept();
			 return;
		 }

		 if (!m_subject -> currentBrush()) {
			 e->accept();
			 return;
		 }

		 Q_INT32 pressure = e -> pressure();

		 if (pressure < 5 && m_mode == PAINT_STYLUS) {
			 endPaint();
		 } else if (pressure >= 5 && m_mode == HOVER) {
			 m_mode = PAINT_STYLUS;
			 initPaint(e -> pos());
			 m_painter -> paintAt(e -> pos(), e->pressure(), e->xTilt(), e->yTilt());
			 // XXX: Get the rect that should be updated
			 m_currentImage -> notify( m_painter -> dirtyRect() );

		 } else if (pressure >= 5 && m_mode == PAINT_STYLUS) {
			 paintLine(m_dragStart, e -> pos(), pressure, e -> xTilt(), e -> yTilt());
		 }
         }
	 e -> accept();
}


void KisToolBrush::initPaint(const QPoint & pos)
{

	if (!m_currentImage -> activeDevice()) return;
	m_dragStart = pos;
	m_dragDist = 0;

	// Create painter
	KisPaintDeviceSP device;
	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
		if (m_painter)
			delete m_painter;
		m_painter = new KisPainter( device );
		m_painter -> beginTransaction(i18n("brush"));
	}

	m_painter -> setPaintColor(m_subject -> fgColor());
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

void KisToolBrush::endPaint() 
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

	}
}

void KisToolBrush::paintLine(const QPoint & pos1,
			     const QPoint & pos2,
			     const Q_INT32 pressure,
			     const Q_INT32 xtilt,
			     const Q_INT32 ytilt)
{
	if (!m_currentImage -> activeDevice()) return;

	m_dragDist = m_painter -> paintLine(PAINTOP_BRUSH, pos1, pos2, pressure, xtilt, ytilt, m_dragDist);
	m_currentImage -> notify( m_painter -> dirtyRect() );
	m_dragStart = pos2;
}


void KisToolBrush::setup(KActionCollection *collection)
{
        KToggleAction *toggle;
        toggle = new KToggleAction(i18n("&Brush"),
				   "paintbrush", 0, this,
                                   SLOT(activate()), collection,
                                   "tool_brush");
        toggle -> setExclusiveGroup("tools");
}

KDialog *KisToolBrush::options(QWidget * parent)
{
        ToolOptsStruct ts;

        ts.usePattern = false; //m_usePattern;
        ts.useGradient = false; //m_useGradient;
        ts.opacity = OPACITY_OPAQUE; //m_opacity;

        ToolOptionsDialog * d = new ToolOptionsDialog(tt_brushtool, ts, parent);

	return d;
}

QWidget* KisToolBrush::createoptionWidget(QWidget* parent)
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

QWidget* KisToolBrush::optionWidget()
{
	return m_optWidget;
}

void KisToolBrush::slotSetOpacity(int opacityPerCent)
{
	m_opacity = opacityPerCent * OPACITY_OPAQUE / 100;
}

void KisToolBrush::slotSetCompositeMode(int compositeOp)
{
	m_compositeOp = (CompositeOp)compositeOp;
}

#include "kis_tool_brush.moc"
