/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_tool_polyline.h"

#include <math.h>

#include <opengl/kis_opengl.h>

#include <QPainter>
#include <QSpinBox>
#include <QKeyEvent>
#include <QMenu>

#include <kaction.h>
#include <kicon.h>
#include <klocale.h>
#include <kis_debug.h>
#include <knuminput.h>

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoCanvasController.h>
#include <KoPathShape.h>
#include <KoShapeController.h>
#include <KoLineBorder.h>

#include <kis_paint_layer.h>
#include <kis_selection.h>
#include <kis_painter.h>
#include <kis_cursor.h>
#include <kis_paint_information.h>
#include <kis_paintop_preset.h>

#define PREVIEW_LINE_WIDTH 1

KisToolPolyline::KisToolPolyline(KoCanvasBase * canvas)
        : KisToolPaint(canvas, KisCursor::load("tool_polyline_cursor.png", 6, 6)),
        m_dragging(false)
{
    setObjectName("tool_polyline");

    KAction *action = new KAction(i18n("&Finish Polyline"), this);
    addAction("finish_polyline", action);
    connect(action, SIGNAL(triggered()), this, SLOT(finish()));
    action = new KAction(KIcon("dialog-cancel"), i18n("&Cancel"), this);
    addAction("cancel_polyline", action);
    connect(action, SIGNAL(triggered()), this, SLOT(cancel()));


    QList<QAction*> list;
    list.append(this->action("finish_polyline"));
    list.append(this->action("cancel_polyline"));
    setPopupActionList(list);
}

KisToolPolyline::~KisToolPolyline()
{
}

void KisToolPolyline::mousePressEvent(KoPointerEvent *event)
{
    if (currentImage()) {
        if (event->button() == Qt::LeftButton && event->modifiers() != Qt::ShiftModifier) {

            m_dragging = true;

            if (m_points.isEmpty()) {
                m_dragStart = convertToPixelCoord(event);
                m_dragEnd = convertToPixelCoord(event);
                m_points.append(m_dragStart);
                m_boundingRect = QRectF(m_dragStart.x(), m_dragStart.y(), 0, 0);
            } else {
                m_dragStart = m_dragEnd;
                m_dragEnd = convertToPixelCoord(event);
            }
        } else if (event->button() == Qt::LeftButton && event->modifiers() == Qt::ShiftModifier) {
            finish();
        }
    }
}

void KisToolPolyline::deactivate()
{
    m_points.clear();
    m_dragging = false;
}

void KisToolPolyline::finish()
{
    m_dragging = false;

    if (!currentNode())
        return;

    if (!currentNode()->inherits("KisShapeLayer")) {
        KisPaintDeviceSP device = currentNode()->paintDevice();
        if (!device) return;

        KisPainter painter(device, currentSelection());
        painter.beginTransaction(i18n("Polyline"));
        if (KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(currentNode().data())) {
            painter.setChannelFlags(l->channelFlags());
            if (l->alphaLocked()) {
                painter.setLockAlpha(l->alphaLocked());
            }
        }
        painter.setPaintColor(currentFgColor());
        painter.setOpacity(m_opacity);
        painter.setCompositeOp(m_compositeOp);
        painter.setPaintOpPreset(currentPaintOpPreset(), currentImage()); // Painter takes ownership

        QPointF start, end;
        KoPointVector::iterator it;
        for (it = m_points.begin(); it != m_points.end(); ++it) {
            if (it == m_points.begin()) {
                start = (*it);
            } else {
                end = (*it);
                painter.paintLine(start, end);
                start = end;
            }
        }
        device->setDirty(painter.dirtyRegion());   
        m_canvas->addCommand(painter.endTransaction());
    } else {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);

        QMatrix resolutionMatrix;
        resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
        path->moveTo(resolutionMatrix.map(m_points[0]));
        for (int i = 1; i < m_points.count(); i++)
            path->lineTo(resolutionMatrix.map(m_points[i]));
        path->normalize();

        KoLineBorder* border = new KoLineBorder(1.0, currentFgColor().toQColor());
        path->setBorder(border);

        QUndoCommand * cmd = m_canvas->shapeController()->addShape(path);
        m_canvas->addCommand(cmd);
    }
    m_points.clear();

    notifyModified();
}

void KisToolPolyline::cancel()
{
    m_dragging = false;
    m_points.clear();
}

void KisToolPolyline::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        // erase old lines on canvas
        QRectF updateRect = dragBoundingRect();
        // get current mouse position
        m_dragEnd = convertToPixelCoord(event);
        // draw new lines on canvas
        updateRect |= dragBoundingRect();
        updateCanvasViewRect(updateRect);
    }
}

void KisToolPolyline::mouseReleaseEvent(KoPointerEvent *event)
{
    if (!m_canvas || !currentImage())
        return;

    if (m_dragging && event->button() == Qt::LeftButton)  {
        m_dragging = false;
        m_points.append(m_dragEnd);
        m_boundingRect = m_boundingRect.unite(QRectF(m_dragStart.x(), m_dragStart.y(), m_dragEnd.x() - m_dragStart.x(), m_dragEnd.y() - m_dragStart.y()));
    }
    m_canvas->updateCanvas(m_boundingRect);
}


void KisToolPolyline::mouseDoubleClickEvent(KoPointerEvent *)
{
    finish();
}

void KisToolPolyline::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    if (!m_canvas || !currentImage())
        return;

    QPointF start, end;
    QPointF startPos;
    QPointF endPos;

#if defined(HAVE_OPENGL)
    if (isCanvasOpenGL()) {
        beginOpenGL();

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_XOR);
        glColor3f(0.501961, 1.0, 0.501961);

        if (m_dragging) {
            startPos = pixelToView(m_dragStart);
            endPos = pixelToView(m_dragEnd);
            glBegin(GL_LINES);
            glVertex2f(startPos.x(), startPos.y());
            glVertex2f(endPos.x(), endPos.y());
            glEnd();
        }

        glBegin(GL_LINES);
        for (KoPointVector::iterator it = m_points.begin(); it != m_points.end(); ++it) {

            if (it == m_points.begin()) {
                start = (*it);
            } else {
                end = (*it);

                startPos = pixelToView(start);
                endPos = pixelToView(end);

                glVertex2f(startPos.x(), startPos.y());
                glVertex2f(endPos.x(), endPos.y());

                start = end;
            }
        }
        glEnd();

        glDisable(GL_COLOR_LOGIC_OP);
        glDisable(GL_LINE_SMOOTH);

        endOpenGL();
    } else
#endif

#ifdef INDEPENDENT_CANVAS
    {
        QPainterPath path;
        if (m_dragging) {
            startPos = pixelToView(m_dragStart);
            endPos = pixelToView(m_dragEnd);
            path.moveTo(startPos);
            path.lineTo(endPos);
        }
        for (KoPointVector::iterator it = m_points.begin(); it != m_points.end(); ++it) {

            if (it == m_points.begin()) {
                start = (*it);
            } else {
                end = (*it);

                startPos = pixelToView(start);
                endPos = pixelToView(end);

                path.moveTo(startPos);
                path.lineTo(endPos);

                start = end;
            }
        }
        paintToolOutline(&gc, path);

    }
#else
    {
        QPen pen(Qt::SolidLine);
        pen.setWidth(PREVIEW_LINE_WIDTH);
        gc.setPen(pen);

        if (m_dragging) {
            startPos = pixelToView(m_dragStart);
            endPos = pixelToView(m_dragEnd);
            gc.drawLine(startPos, endPos);
        }
        for (KoPointVector::iterator it = m_points.begin(); it != m_points.end(); ++it) {

            if (it == m_points.begin()) {
                start = (*it);
            } else {
                end = (*it);

                startPos = pixelToView(start);
                endPos = pixelToView(end);

                gc.drawLine(startPos, endPos);

                start = end;
            }
        }
    }
#endif
}

QString KisToolPolyline::quickHelp() const
{
    return i18n("Press shift-mouseclick to end the polyline.");
}

void KisToolPolyline::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        cancel();
}

QRectF KisToolPolyline::dragBoundingRect()
{
    QRectF rect = pixelToView(QRectF(m_dragStart, m_dragEnd).normalized());
    rect.adjust(-PREVIEW_LINE_WIDTH, -PREVIEW_LINE_WIDTH, PREVIEW_LINE_WIDTH, PREVIEW_LINE_WIDTH);
    return rect;
}

#include "kis_tool_polyline.moc"
