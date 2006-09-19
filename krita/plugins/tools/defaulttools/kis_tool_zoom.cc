/*
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include <kaction.h>
#include <klocale.h>
#include <kapplication.h>

#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_cursor.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_tool_zoom.h"


KisToolZoom::KisToolZoom()
    : super(i18n("Zoom Tool"))
{
    setName("tool_zoom");
    m_subject = 0;
    m_dragging = false;
    m_startPos = QPoint(0, 0);
    m_endPos = QPoint(0, 0);
    m_plusCursor = KisCursor::load("tool_zoom_plus_cursor.png", 8, 8);
    m_minusCursor = KisCursor::load("tool_zoom_minus_cursor.png", 8, 8);
    setCursor(m_plusCursor);
    connect(&m_timer, SIGNAL(timeout()), SLOT(slotTimer()));
}

KisToolZoom::~KisToolZoom()
{
}

void KisToolZoom::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(m_subject);
}

void KisToolZoom::paint(KisCanvasPainter& gc)
{
    if (m_dragging)
        paintOutline(gc, QRect());
}

void KisToolZoom::paint(KisCanvasPainter& gc, const QRect& rc)
{
    if (m_dragging)
        paintOutline(gc, rc);
}

void KisToolZoom::buttonPress(KisButtonPressEvent *e)
{
    if (m_subject && m_subject->currentImg() && !m_dragging) {
        if (e->button() == Qt::LeftButton) {
            m_startPos = e->pos().floorQPoint();
            m_endPos = e->pos().floorQPoint();
            m_dragging = true;
        }
    }
}

void KisToolZoom::move(KisMoveEvent *e)
{
    if (m_subject && m_dragging) {
        if (m_startPos != m_endPos)
            paintOutline();

        m_endPos = e->pos().floorQPoint();
        paintOutline();
    }
}

void KisToolZoom::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_subject && m_dragging && e->button() == Qt::LeftButton) {

        KisCanvasController *controller = m_subject->canvasController();
        m_endPos = e->pos().floorQPoint();
        m_dragging = false;

        QPoint delta = m_endPos - m_startPos;

        if (sqrt(delta.x() * delta.x() + delta.y() * delta.y()) < 10) {
            if (e->state() & Qt::ControlButton) {
                controller->zoomOut(m_endPos.x(), m_endPos.y());
            } else {
                controller->zoomIn(m_endPos.x(), m_endPos.y());
            }
        } else {
            controller->zoomTo(QRect(m_startPos, m_endPos));
        }
    }
}

void KisToolZoom::activate()
{
    super::activate();
    m_timer.start(50);
}

void KisToolZoom::deactivate()
{
    m_timer.stop();
}

void KisToolZoom::slotTimer()
{
#if KDE_IS_VERSION(3,4,0)
    int state = kapp->keyboardMouseState() & (Qt::ShiftButton|Qt::ControlButton|Qt::AltButton);
#else
    int state = kapp->keyboardModifiers() & (KApplication::ShiftModifier
            |KApplication::ControlModifier|KApplication::Modifier1);
#endif

    if (state & Qt::ControlButton) {
        m_subject->canvasController()->setCanvasCursor(m_minusCursor);
    } else {
        m_subject->canvasController()->setCanvasCursor(m_plusCursor);
    }
}

void KisToolZoom::paintOutline()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        KisCanvas *canvas = controller->kiscanvas();
        KisCanvasPainter gc(canvas);
        QRect rc;

        paintOutline(gc, rc);
    }
}

void KisToolZoom::paintOutline(KisCanvasPainter& gc, const QRect&)
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        RasterOp op = gc.rasterOp();
        QPen old = gc.pen();
        QPen pen(Qt::DotLine);
        QPoint start;
        QPoint end;

        Q_ASSERT(controller);
        start = controller->windowToView(m_startPos);
        end = controller->windowToView(m_endPos);

        gc.setRasterOp(Qt::NotROP);
        gc.setPen(pen);
        gc.drawRect(QRect(start, end));
        gc.setRasterOp(op);
        gc.setPen(old);
    }
}

void KisToolZoom::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Zoom"), "tool_zoom", Qt::Key_Z, this, SLOT(activate()), collection, name());
        m_action->setToolTip(i18n("Zoom"));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_zoom.moc"
