
/*
 *  kis_tool_select_rectangular.cc -- part of Krita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *                2001 John Califf <jcaliff@compuzone.net>
 *                2002 Patrick Julien <freak@codepimps.org>
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

#include <QApplication>
#include <QPainter>
#include <QPen>
#include <QLayout>
#include <QVBoxLayout>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_tool_select_rectangular.h"
#include "kis_undo_adapter.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include <kis_selected_transaction.h>
#include "kis_canvas.h"
#include "QPainter"

KisToolSelectRectangular::KisToolSelectRectangular()
    : super(i18n("Rectangular Select Tool"))
{
    setObjectName("tool_select_rectangular");
    setCursor(KisCursor::load("tool_rectangular_selection_cursor.png", 6, 6));
    m_subject = 0;
    m_selecting = false;
    m_centerPos = KisPoint(0, 0);
    m_startPos = KisPoint(0, 0);
    m_endPos = KisPoint(0, 0);
    m_optWidget = 0;
    m_selectAction = SELECTION_ADD;
}

KisToolSelectRectangular::~KisToolSelectRectangular()
{
}

void KisToolSelectRectangular::activate()
{
    super::activate();

    if (!m_optWidget)
        return;

    m_optWidget->slotActivated();
}

void KisToolSelectRectangular::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(m_subject);
}

void KisToolSelectRectangular::paint(QPainter& gc)
{
    if (m_selecting)
        paintOutline(gc, QRect());
}

void KisToolSelectRectangular::paint(QPainter& gc, const QRect& rc)
{
    if (m_selecting)
        paintOutline(gc, rc);
}

void KisToolSelectRectangular::clearSelection()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        KisImageSP img = m_subject->currentImg();

        Q_ASSERT(controller);

        m_centerPos = KisPoint(0, 0);
        m_startPos = KisPoint(0, 0);
        m_endPos = KisPoint(0, 0);
        m_selecting = false;
    }
}

void KisToolSelectRectangular::buttonPress(KisButtonPressEvent *e)
{
    if (m_subject) {
        KisImageSP img = m_subject->currentImg();

        if (img && img->activeDevice() && e->button() == Qt::LeftButton) {
            clearSelection();
            m_startPos = m_endPos = m_centerPos = e->pos();
            m_selecting = true;
        }
    }
}

void KisToolSelectRectangular::move(KisMoveEvent *e)
{
    if (m_subject && m_selecting) {
        paintOutline();
        // move (alt) or resize rectangle
        if (e->modifiers() & Qt::AltModifier) {
            KisPoint trans = e->pos() - m_endPos;
            m_startPos += trans;
            m_endPos += trans;
        } else {
            KisPoint diag = e->pos() - (e->modifiers() & Qt::ControlModifier
                    ? m_centerPos : m_startPos);
            // square?
            if (e->modifiers() & Qt::ShiftModifier) {
                double size = qMax(fabs(diag.x()), fabs(diag.y()));
                double w = diag.x() < 0 ? -size : size;
                double h = diag.y() < 0 ? -size : size;
                diag = KisPoint(w, h);
            }

            // resize around center point?
            if (e->modifiers() & Qt::ControlModifier) {
                m_startPos = m_centerPos - diag;
                m_endPos = m_centerPos + diag;
            } else {
                m_endPos = m_startPos + diag;
            }
        }
        paintOutline();
        m_centerPos = KisPoint((m_startPos.x() + m_endPos.x()) / 2,
                (m_startPos.y() + m_endPos.y()) / 2);
    }
}

void KisToolSelectRectangular::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_subject && m_selecting && e->button() == Qt::LeftButton) {

        paintOutline();

        if (m_startPos == m_endPos) {
            clearSelection();
        } else {
            KisImageSP img = m_subject->currentImg();

            if (!img)
                return;

            if (m_endPos.y() < 0)
                m_endPos.setY(0);

            if (m_endPos.y() > img->height())
                m_endPos.setY(img->height());

            if (m_endPos.x() < 0)
                m_endPos.setX(0);

            if (m_endPos.x() > img->width())
                m_endPos.setX(img->width());
            if (img && img->activeDevice()) {

                QApplication::setOverrideCursor(KisCursor::waitCursor());
                KisPaintDeviceSP dev = img->activeDevice();
                bool hasSelection = dev->hasSelection();

                KisSelectedTransaction *t = 0;
                if (img->undo())  t = new KisSelectedTransaction(i18n("Rectangular Selection"), dev);
                KisSelectionSP selection = dev->selection();
                QRect rc(m_startPos.floorQPoint(), m_endPos.floorQPoint());
                rc = rc.normalized();

                // We don't want the border of the 'rectangle' to be included in our selection
                rc.setSize(rc.size() - QSize(1,1));

                if(! hasSelection)
                {
                    selection->clear();
                    if(m_selectAction==SELECTION_SUBTRACT)
                        selection->invert();
                }

                KisSelectionSP tmpSel = KisSelectionSP(new KisSelection(dev));
                tmpSel->select(rc);
                switch(m_selectAction)
                {
                    case SELECTION_ADD:
                        dev->addSelection(tmpSel);
                        break;
                    case SELECTION_SUBTRACT:
                        dev->subtractSelection(tmpSel);
                        break;
                    default:
                        break;
                }

                if(hasSelection)
                    dev->emitSelectionChanged(rc);
                else
                    dev->emitSelectionChanged();

                if (img->undo())
                    img->undoAdapter()->addCommand(t);

                QApplication::restoreOverrideCursor();
            }
        }

        m_selecting = false;
    }
}

void KisToolSelectRectangular::paintOutline()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        KisCanvas *canvas = controller->kiscanvas();
        QPainter gc(canvas->canvasWidget());
        QRect rc;

        paintOutline(gc, rc);
    }
}

void KisToolSelectRectangular::paintOutline(QPainter& gc, const QRect&)
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        //RasterOp op = gc.rasterOp();
        QPen old = gc.pen();
        QPen pen(Qt::DotLine);
        QPoint start;
        QPoint end;

        Q_ASSERT(controller);
        start = controller->windowToView(m_startPos.floorQPoint());
        end = controller->windowToView(m_endPos.floorQPoint());

        //gc.setRasterOp(Qt::NotROP);
        gc.setPen(pen);
        gc.drawRect(QRect(start, end));
        //gc.setRasterOp(op);
        gc.setPen(old);
    }
}

void KisToolSelectRectangular::slotSetAction(int action) {
    if (action >= SELECTION_ADD && action <= SELECTION_SUBTRACT)
        m_selectAction =(enumSelectionMode)action;
}

void KisToolSelectRectangular::setup(KActionCollection *collection)
{
    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("tool_rect_selection"),
                               i18n("&Rectangular Selection"),
                               collection,
                               objectName());
        Q_CHECK_PTR(m_action);
        m_action->setShortcut(Qt::Key_R);
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setActionGroup(actionGroup());
        m_action->setToolTip(i18n("Select a rectangular area"));
        m_ownAction = true;
    }
}

QWidget* KisToolSelectRectangular::createOptionWidget(QWidget* parent)
{
    m_optWidget = new KisSelectionOptions(parent, m_subject);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setWindowTitle(i18n("Rectangular Selection"));

    connect (m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));

    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    Q_ASSERT(l);
    if (l) {
        l->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    }

    return m_optWidget;
}

QWidget* KisToolSelectRectangular::optionWidget()
{
        return m_optWidget;
}




#include "kis_tool_select_rectangular.moc"
