/*
 *  kis_tool_ellipse.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
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

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>

#include "kis_painter.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_tool_ellipse.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_paintop_registry.h"
#include "kis_undo_adapter.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_cursor.h"

KisToolEllipse::KisToolEllipse()
    : super(i18n ("Ellipse")),
      m_dragging (false),
      m_currentImage (0)
{
    setName("tool_ellipse");
    setCursor(KisCursor::load("tool_ellipse_cursor.png", 6, 6));
}

KisToolEllipse::~KisToolEllipse()
{
}

void KisToolEllipse::update (KisCanvasSubject *subject)
{
    super::update (subject);
    if (m_subject)
        m_currentImage = m_subject->currentImg ();
}

void KisToolEllipse::buttonPress(KisButtonPressEvent *event)
{
    if (m_currentImage && event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStart = m_dragCenter = m_dragEnd = event->pos();
        draw(m_dragStart, m_dragEnd);
    }
}

void KisToolEllipse::move(KisMoveEvent *event)
{
    if (m_dragging) {
        // erase old lines on canvas
        draw(m_dragStart, m_dragEnd);
        // move (alt) or resize ellipse
        if (event->state() & Qt::AltModifier) {
            KisPoint trans = event->pos() - m_dragEnd;
            m_dragStart += trans;
            m_dragEnd += trans;
        } else {
            KisPoint diag = event->pos() - (event->state() & Qt::ControlModifier
                                              ? m_dragCenter : m_dragStart);
            // circle?
            if (event->state() & Qt::ShiftModifier) {
                double size = qMax(fabs(diag.x()), fabs(diag.y()));
                double w = diag.x() < 0 ? -size : size;
                double h = diag.y() < 0 ? -size : size;
                diag = KisPoint(w, h);
            }

            // resize around center point?
            if (event->state() & Qt::ControlModifier) {
                m_dragStart = m_dragCenter - diag;
                m_dragEnd = m_dragCenter + diag;
            } else {
                m_dragEnd = m_dragStart + diag;
            }
        }
        // draw new lines on canvas
        draw(m_dragStart, m_dragEnd);
        m_dragCenter = KisPoint((m_dragStart.x() + m_dragEnd.x()) / 2,
                                (m_dragStart.y() + m_dragEnd.y()) / 2);
    }
}

void KisToolEllipse::buttonRelease(KisButtonReleaseEvent *event)
{
    if (!m_subject || !m_currentImage)
        return;

    if (m_dragging && event->button() == Qt::LeftButton) {
        // erase old lines on canvas
        draw(m_dragStart, m_dragEnd);
        m_dragging = false;

        if (m_dragStart == m_dragEnd)
            return;

        if (!m_currentImage)
            return;

        if (!m_currentImage->activeDevice())
            return;

        KisPaintDeviceSP device = m_currentImage->activeDevice ();
        KisPainter painter (device);
        if (m_currentImage->undo()) painter.beginTransaction (i18n ("Ellipse"));

        painter.setPaintColor(m_subject->fgColor());
        painter.setBackgroundColor(m_subject->bgColor());
        painter.setFillStyle(fillStyle());
        painter.setBrush(m_subject->currentBrush());
        painter.setPattern(m_subject->currentPattern());
        painter.setOpacity(m_opacity);
        painter.setCompositeOp(m_compositeOp);
        KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_subject->currentPaintop(), m_subject->currentPaintopSettings(), &painter);
        painter.setPaintOp(op); // Painter takes ownership

        painter.paintEllipse(m_dragStart, m_dragEnd, PRESSURE_DEFAULT/*event->pressure()*/, event->xTilt(), event->yTilt());
        device->setDirty( painter.dirtyRect() );
        notifyModified();

        KisUndoAdapter *adapter = m_currentImage->undoAdapter();
        if (adapter) {
            adapter->addCommand(painter.endTransaction());
        }
    }
}

void KisToolEllipse::draw(const KisPoint& start, const KisPoint& end )
{
    if (!m_subject || !m_currentImage)
        return;

    KisCanvasController *controller = m_subject->canvasController ();
    KisCanvas *canvas = controller->kiscanvas();
    KisCanvasPainter p (canvas);

    //p.setRasterOp (Qt::NotROP);
    p.drawEllipse (QRect (controller->windowToView (start).floorQPoint(), controller->windowToView (end).floorQPoint()));
    p.end ();
}

void KisToolEllipse::setup(KActionCollection *collection)
{
    m_action = collection->action(name());

    if (m_action == 0) {
        KShortcut shortcut(Qt::Key_Plus);
        shortcut.append(KKeySequence(KKey(Qt::Key_F7)));
        m_action = new KAction(i18n("&Ellipse"),
                                    "tool_ellipse",
                                    shortcut,
                                    this,
                                    SLOT(activate()),
                                    collection,
                                    name());
        m_action->setToolTip(i18n("Draw an ellipse"));
        m_action->setActionGroup(actionGroup());
        m_ownAction = true;
    }
}

#include "kis_tool_ellipse.moc"
