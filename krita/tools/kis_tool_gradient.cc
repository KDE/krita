/*
 *  kis_tool_gradient.cc - part of Krita
 *
 *  Copyright (c) 2000 John Califf
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_tool_gradient.h"
#include "kis_view.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "integerwidget.h"
#include "kis_cmb_composite.h"
#include "kis_double_widget.h"
#include "kis_progress_display_interface.h"

KisToolGradient::KisToolGradient()
	: super(),
	  m_dragging( false ),
	  m_opacity(OPACITY_OPAQUE),
	  m_compositeOp(COMPOSITE_OVER)
{
	setName("tool_gradient");
	setCursor(KisCursor::arrowCursor());

	m_startPos = KisPoint(0, 0);
	m_endPos = KisPoint(0, 0);

	m_reverse = false;
	m_shape = KisPainter::GradientShapeLinear;
	m_repeat = KisPainter::GradientRepeatNone;
	m_antiAliasThreshold = 0.2;
}

KisToolGradient::~KisToolGradient()
{
}

void KisToolGradient::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	super::update(m_subject);
}

void KisToolGradient::paint(QPainter& gc)
{
	if (m_dragging)
		paintLine(gc);
}

void KisToolGradient::paint(QPainter& gc, const QRect&)
{
	if (m_dragging)
		paintLine(gc);
}

void KisToolGradient::buttonPress(KisButtonPressEvent *e)
{
	if (!m_subject || !m_subject -> currentImg()) {
		return;
	}

	if (e -> button() == QMouseEvent::LeftButton) {
		m_dragging = true;
		m_startPos = e -> pos();
		m_endPos = e -> pos();
	}
}

void KisToolGradient::move(KisMoveEvent *e)
{
	if (m_dragging) {
		if (m_startPos != m_endPos) {
			paintLine();
		}

		if ((e -> state() & Qt::ShiftButton) == Qt::ShiftButton) {
			m_endPos = straightLine(e -> pos());
		}
		else {
			m_endPos = e -> pos();
		}

		paintLine();
	}
}

void KisToolGradient::buttonRelease(KisButtonReleaseEvent *e)
{
	if (m_dragging && e -> button() == QMouseEvent::LeftButton) {

		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		KisImageSP img = m_subject -> currentImg();

		m_dragging = false;

		if (m_startPos == m_endPos) {
			controller -> updateCanvas();
			m_dragging = false;
			return;
		}

		if ((e -> state() & Qt::ShiftButton) == Qt::ShiftButton) {
			m_endPos = straightLine(e -> pos());
		}
		else {
			m_endPos = e -> pos();
		}

		KisPaintDeviceSP device;

		if (img && (device = img -> activeDevice())) {

			KisPainter painter(device);

			painter.beginTransaction(i18n("Gradient"));

			painter.setPaintColor(m_subject -> fgColor());
			painter.setGradient(*(m_subject -> currentGradient()));
			painter.setOpacity(m_opacity);
			painter.setCompositeOp(m_compositeOp);

			KisProgressDisplayInterface *progress = m_subject -> progressDisplay();

			if (progress) {
				progress -> setSubject(&painter, true, true);
			}

			bool painted = painter.paintGradient(m_startPos, m_endPos, m_shape, m_repeat, m_antiAliasThreshold, m_reverse);

			if (painted) {
				// does whole thing at moment
				img -> notify(/* m_painter -> dirtyRect() */);

				notifyModified();

				KisUndoAdapter *adapter = img -> undoAdapter();

				if (adapter) {
					adapter -> addCommand(painter.endTransaction());
				}
			}

			/* remove remains of the line drawn while moving */
			if (controller -> canvas()) {
				controller -> canvas() -> update();
			}

		} else {
			// m_painter can be 0 here...!!!
			//controller -> updateCanvas(m_painter -> dirtyRect()); // Removes the last remaining line.
		}
	}
}

KisPoint KisToolGradient::straightLine(KisPoint point)
{
	KisPoint comparison = point - m_startPos;
	KisPoint result;
	
	if ( fabs(comparison.x()) > fabs(comparison.y())) {
		result.setX(point.x());
		result.setY(m_startPos.y());
	} else {
		result.setX( m_startPos.x() );
		result.setY( point.y() );
	}
	
	return result;
}

void KisToolGradient::paintLine()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		QWidget *canvas = controller -> canvas();
		QPainter gc(canvas);

		paintLine(gc);
	}
}

void KisToolGradient::paintLine(QPainter& gc)
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();

		KisPoint start = controller -> windowToView(m_startPos);
		KisPoint end = controller -> windowToView(m_endPos);

		RasterOp op = gc.rasterOp();
		QPen old = gc.pen();
		QPen pen(Qt::SolidLine);

		gc.setRasterOp(Qt::NotROP);
		gc.setPen(pen);
		gc.drawLine(start.floorQPoint(), end.floorQPoint());
		gc.setRasterOp(op);
		gc.setPen(old);
	}
}

QWidget* KisToolGradient::createOptionWidget(QWidget* parent)
{
	m_optWidget = new QWidget(parent);
	m_optWidget -> setCaption(i18n("Gradient"));
	
	m_lbOpacity = new QLabel(i18n("Opacity: "), m_optWidget);
	m_slOpacity = new IntegerWidget( 0, 100, m_optWidget, "int_widget");
	m_slOpacity -> setTickmarks(QSlider::Below);
	m_slOpacity -> setTickInterval(10);
	m_slOpacity -> setValue(m_opacity / OPACITY_OPAQUE * 100);
	connect(m_slOpacity, SIGNAL(valueChanged(int)), this, SLOT(slotSetOpacity(int)));

	m_lbComposite = new QLabel(i18n("Mode: "), m_optWidget);
	m_cmbComposite = new KisCmbComposite(m_optWidget);
	connect(m_cmbComposite, SIGNAL(activated(int)), this, SLOT(slotSetCompositeMode(int)));

	QGridLayout *optionLayout = new QGridLayout(m_optWidget, 8, 2);

	optionLayout -> addWidget(m_lbOpacity, 1, 0);
	optionLayout -> addWidget(m_slOpacity, 1, 1);

	optionLayout -> addWidget(m_lbComposite, 2, 0);
	optionLayout -> addWidget(m_cmbComposite, 2, 1);                 

	m_lbShape = new QLabel(i18n("Shape: "), m_optWidget);
	m_lbRepeat = new QLabel(i18n("Repeat: "), m_optWidget);

	m_ckReverse = new QCheckBox(i18n("Reverse"), m_optWidget, "reverse_check");
	connect(m_ckReverse, SIGNAL(toggled(bool)), this, SLOT(slotSetReverse(bool)));

	m_cmbShape = new QComboBox(false, m_optWidget, "shape_combo");
	connect(m_cmbShape, SIGNAL(activated(int)), this, SLOT(slotSetShape(int)));
	m_cmbShape -> insertItem(i18n("Linear"));
	m_cmbShape -> insertItem(i18n("Bi-Linear"));
	m_cmbShape -> insertItem(i18n("Radial"));
	m_cmbShape -> insertItem(i18n("Square"));
	m_cmbShape -> insertItem(i18n("Conical"));
	m_cmbShape -> insertItem(i18n("Conical Symetric"));

	m_cmbRepeat = new QComboBox(false, m_optWidget, "repeat_combo");
	connect(m_cmbRepeat, SIGNAL(activated(int)), this, SLOT(slotSetRepeat(int)));
	m_cmbRepeat -> insertItem(i18n("None"));
	m_cmbRepeat -> insertItem(i18n("Forwards"));
	m_cmbRepeat -> insertItem(i18n("Alternating"));

	optionLayout -> addWidget(m_lbShape, 3, 0);
	optionLayout -> addWidget(m_cmbShape, 3, 1);

	optionLayout -> addWidget(m_lbRepeat, 4, 0);
	optionLayout -> addWidget(m_cmbRepeat, 4, 1);

	optionLayout -> addWidget(m_ckReverse, 5, 0);

	m_lbAntiAliasThreshold = new QLabel(i18n("Anti-alias Threshold: "), m_optWidget);

	m_slAntiAliasThreshold = new KisDoubleWidget(0, 1, m_optWidget, "threshold_slider");
	m_slAntiAliasThreshold -> setTickmarks(QSlider::Below);
	m_slAntiAliasThreshold -> setTickInterval(0.1);
	m_slAntiAliasThreshold -> setValue(m_antiAliasThreshold);
	connect(m_slAntiAliasThreshold, SIGNAL(valueChanged(double)), this, SLOT(slotSetAntiAliasThreshold(double)));

	optionLayout -> addWidget(m_lbAntiAliasThreshold, 6, 0);
	optionLayout -> addWidget(m_slAntiAliasThreshold, 6, 1);

	return m_optWidget;
}

QWidget *KisToolGradient::optionWidget()
{
	return m_optWidget;
}

void KisToolGradient::slotSetOpacity(int opacityPerCent)
{
	m_opacity = opacityPerCent * OPACITY_OPAQUE / 100;
}

void KisToolGradient::slotSetCompositeMode(int compositeOp)
{
	m_compositeOp = (CompositeOp)compositeOp;
}

void KisToolGradient::slotSetShape(int shape)
{
	m_shape = static_cast<KisPainter::enumGradientShape>(shape);
}

void KisToolGradient::slotSetRepeat(int repeat)
{
	m_repeat = static_cast<KisPainter::enumGradientRepeat>(repeat);
}

void KisToolGradient::slotSetReverse(bool state)
{
	m_reverse = state;
}

void KisToolGradient::slotSetAntiAliasThreshold(double value)
{
	m_antiAliasThreshold = value;
}

void KisToolGradient::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Gradient"),
					    "gradient", Qt::Key_G, this,
					    SLOT(activate()), collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

#include "kis_tool_gradient.moc"

