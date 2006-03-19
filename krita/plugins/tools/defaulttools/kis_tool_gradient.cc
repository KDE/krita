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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qpainter.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <knuminput.h>

#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_canvas_subject.h"
#include "kis_cmb_composite.h"
#include "kis_cursor.h"
#include "kis_double_widget.h"
#include "kis_gradient_painter.h"
#include "kis_move_event.h"
#include "kis_painter.h"
#include "kis_progress_display_interface.h"
#include "kis_tool_gradient.h"
#include "kis_undo_adapter.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"

KisToolGradient::KisToolGradient()
    : super(i18n("Gradient")),
      m_dragging( false )
{
    setName("tool_gradient");
    setCursor(KisCursor::load("tool_gradient_cursor.png", 6, 6));

    m_startPos = KisPoint(0, 0);
    m_endPos = KisPoint(0, 0);

    m_reverse = false;
    m_shape = KisGradientPainter::GradientShapeLinear;
    m_repeat = KisGradientPainter::GradientRepeatNone;
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

void KisToolGradient::paint(KisCanvasPainter& gc)
{
    if (m_dragging)
        paintLine(gc);
}

void KisToolGradient::paint(KisCanvasPainter& gc, const QRect&)
{
    if (m_dragging)
        paintLine(gc);
}

void KisToolGradient::buttonPress(KisButtonPressEvent *e)
{
    if (!m_subject || !m_subject->currentImg()) {
        return;
    }

    if (e->button() == QMouseEvent::LeftButton) {
        m_dragging = true;
        m_startPos = e->pos();
        m_endPos = e->pos();
    }
}

void KisToolGradient::move(KisMoveEvent *e)
{
    if (m_dragging) {
        if (m_startPos != m_endPos) {
            paintLine();
        }

        if ((e->state() & Qt::ShiftButton) == Qt::ShiftButton) {
            m_endPos = straightLine(e->pos());
        }
        else {
            m_endPos = e->pos();
        }

        paintLine();
    }
}

void KisToolGradient::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_dragging && e->button() == QMouseEvent::LeftButton) {

        KisCanvasController *controller = m_subject->canvasController();
        KisImageSP img = m_subject->currentImg();

        m_dragging = false;

        if (m_startPos == m_endPos) {
            controller->updateCanvas();
            m_dragging = false;
            return;
        }

        if ((e->state() & Qt::ShiftButton) == Qt::ShiftButton) {
            m_endPos = straightLine(e->pos());
        }
        else {
            m_endPos = e->pos();
        }

        KisPaintDeviceSP device;

        if (img && (device = img->activeDevice())) {

            KisGradientPainter painter(device);

            if (img->undo())  painter.beginTransaction(i18n("Gradient"));

            painter.setPaintColor(m_subject->fgColor());
            painter.setGradient(*(m_subject->currentGradient()));
            painter.setOpacity(m_opacity);
            painter.setCompositeOp(m_compositeOp);

            KisProgressDisplayInterface *progress = m_subject->progressDisplay();

            if (progress) {
                progress->setSubject(&painter, true, true);
            }

            bool painted = painter.paintGradient(m_startPos, m_endPos, m_shape, m_repeat, m_antiAliasThreshold, m_reverse, 0, 0, m_subject->currentImg()->width(), m_subject->currentImg()->height());

            if (painted) {
                // does whole thing at moment
                device->setDirty(painter.dirtyRect());

                notifyModified();

                if (img->undo()) {
                    img->undoAdapter()->addCommand(painter.endTransaction());
                }
            }

            /* remove remains of the line drawn while moving */
            if (controller->kiscanvas()) {
                controller->kiscanvas()->update();
            }

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
        KisCanvasController *controller = m_subject->canvasController();
        KisCanvas *canvas = controller->kiscanvas();
        KisCanvasPainter gc(canvas);

        paintLine(gc);
    }
}

void KisToolGradient::paintLine(KisCanvasPainter& gc)
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();

        KisPoint start = controller->windowToView(m_startPos);
        KisPoint end = controller->windowToView(m_endPos);

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
    QWidget *widget = super::createOptionWidget(parent);
    Q_CHECK_PTR(widget);

    m_lbShape = new QLabel(i18n("Shape:"), widget);
    m_lbRepeat = new QLabel(i18n("Repeat:"), widget);

    m_ckReverse = new QCheckBox(i18n("Reverse"), widget, "reverse_check");
    connect(m_ckReverse, SIGNAL(toggled(bool)), this, SLOT(slotSetReverse(bool)));

    m_cmbShape = new QComboBox(false, widget, "shape_combo");
    connect(m_cmbShape, SIGNAL(activated(int)), this, SLOT(slotSetShape(int)));
    m_cmbShape->insertItem(i18n("Linear"));
    m_cmbShape->insertItem(i18n("Bi-Linear"));
    m_cmbShape->insertItem(i18n("Radial"));
    m_cmbShape->insertItem(i18n("Square"));
    m_cmbShape->insertItem(i18n("Conical"));
    m_cmbShape->insertItem(i18n("Conical Symmetric"));

    m_cmbRepeat = new QComboBox(false, widget, "repeat_combo");
    connect(m_cmbRepeat, SIGNAL(activated(int)), this, SLOT(slotSetRepeat(int)));
    m_cmbRepeat->insertItem(i18n("None"));
    m_cmbRepeat->insertItem(i18n("Forwards"));
    m_cmbRepeat->insertItem(i18n("Alternating"));

    addOptionWidgetOption(m_cmbShape, m_lbShape);

    addOptionWidgetOption(m_cmbRepeat, m_lbRepeat);

    addOptionWidgetOption(m_ckReverse);

    m_lbAntiAliasThreshold = new QLabel(i18n("Anti-alias threshold:"), widget);

    m_slAntiAliasThreshold = new KDoubleNumInput(widget, "threshold_slider");
    m_slAntiAliasThreshold->setRange( 0, 1); 
    m_slAntiAliasThreshold->setValue(m_antiAliasThreshold);
    connect(m_slAntiAliasThreshold, SIGNAL(valueChanged(double)), this, SLOT(slotSetAntiAliasThreshold(double)));

    addOptionWidgetOption(m_slAntiAliasThreshold, m_lbAntiAliasThreshold);

    return widget;
}

void KisToolGradient::slotSetShape(int shape)
{
    m_shape = static_cast<KisGradientPainter::enumGradientShape>(shape);
}

void KisToolGradient::slotSetRepeat(int repeat)
{
    m_repeat = static_cast<KisGradientPainter::enumGradientRepeat>(repeat);
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
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Gradient"),
                        "tool_gradient", Qt::Key_G, this,
                        SLOT(activate()), collection,
                        name());
        m_action->setToolTip(i18n("Draw a gradient"));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_gradient.moc"

