/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tool_pan.h"

#include <QScrollBar>
#include <QPainter>

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoViewConverter.h>

#include <kis_canvas2.h>
#include "kis_canvas_controller.h"

#include "kis_cursor.h"
#include "kis_coordinates_converter.h"


const qreal KisToolPan::m_checkerRadius = 50;

KisToolPan::KisToolPan(KoCanvasBase * canvas)
        :  KisTool(canvas, KisCursor::openHandCursor())
{
    setObjectName("tool_pan");
    m_rotationMode = false;
    m_defaultCursor = KisCursor::openHandCursor();
}

KisToolPan::~KisToolPan()
{
}

bool KisToolPan::wantsAutoScroll() const
{
    return false;
}

void KisToolPan::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    if(m_rotationMode) {
        const KisCoordinatesConverter *converter = kritaCanvas()->coordinatesConverter();
        QPointF centerPoint = converter->flakeCenterPoint();

        QBrush fillBrush(QColor(0,0,0,100));
        QPen checkerPen(QColor(255,255,255,100), 5., Qt::SolidLine, Qt::RoundCap);

        gc.save();

        gc.setPen(Qt::NoPen);
        gc.setBrush(fillBrush);
        gc.drawEllipse(centerPoint, m_checkerRadius, m_checkerRadius);

        gc.setPen(checkerPen);
        gc.drawLine(centerPoint, centerPoint + QPointF(0, -m_checkerRadius));

        gc.restore();
    }
}

KisCanvas2* KisToolPan::kritaCanvas() const
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);
    return kritaCanvas;
}

KisCanvasController* KisToolPan::kritaCanvasController() const
{
    KisCanvasController *controller =
        dynamic_cast<KisCanvasController*>(canvas()->canvasController());
    Q_ASSERT(controller);
    return controller;
}

bool KisToolPan::isInCheckerArea(QPointF pt)
{
    QPointF centerPoint = widgetCenterInWidgetPixels();
    pt -= centerPoint;

    return pt.x() * pt.x() + pt.y() * pt.y() <= m_checkerRadius * m_checkerRadius;
}

qreal KisToolPan::calculateAngle(QPointF oldPoint,
                                 QPointF newPoint)
{
    QPointF centerPoint = widgetCenterInWidgetPixels();
    oldPoint -= centerPoint;
    newPoint -= centerPoint;

    qreal oldAngle = atan2(oldPoint.y(), oldPoint.x());
    qreal newAngle = atan2(newPoint.y(), newPoint.x());

    return (180 / 3.14) * (newAngle - oldAngle);
}

void KisToolPan::mousePressEvent(KoPointerEvent *e)
{
    m_lastPosition = convertDocumentToWidget(e->point);
    e->accept();

    m_defaultCursor = KisCursor::closedHandCursor();
    adjustCursor();

    if(m_rotationMode && isInCheckerArea(m_lastPosition)) {
        kritaCanvasController()->resetCanvasTransformations();
    }
}

void KisToolPan::adjustCursor()
{
    QPoint pt = canvas()->canvasWidget()->mapFromGlobal(QCursor::pos());

    if(m_rotationMode && isInCheckerArea(pt)) {
        useCursor(KisCursor::pointingHandCursor());
    }
    else {
        useCursor(m_defaultCursor);
    }
}

void KisToolPan::mouseMoveEvent(KoPointerEvent *e)
{
    Q_ASSERT(canvas());
    Q_ASSERT(canvas()->canvasController());

    QPointF actualPosition = convertDocumentToWidget(e->point);

    adjustCursor();

    if (!e->buttons())
        return;
    e->accept();

    if(e->modifiers() & Qt::ShiftModifier) {
        if(!isInCheckerArea(actualPosition)) {
            qreal angle = calculateAngle(m_lastPosition, actualPosition);
            kritaCanvasController()->rotateCanvas(angle);
        }
    }
    else {
        QPointF distance(m_lastPosition - actualPosition);
        kritaCanvasController()->pan(distance.toPoint());
    }

    m_lastPosition = actualPosition;
}

void KisToolPan::mouseReleaseEvent(KoPointerEvent *e)
{
    Q_UNUSED(e);
    m_defaultCursor = KisCursor::openHandCursor();
    kritaCanvasController()->rotateCanvas(0.0);
    adjustCursor();
}

void KisToolPan::keyPressEvent(QKeyEvent *event)
{
    KoCanvasControllerWidget *canvasControllerWidget = kritaCanvasController();

    switch (event->key()) {
        case Qt::Key_Up:
            canvasControllerWidget->pan(QPoint(0, -canvasControllerWidget->verticalScrollBar()->singleStep()));
            break;
        case Qt::Key_Down:
            canvasControllerWidget->pan(QPoint(0, canvasControllerWidget->verticalScrollBar()->singleStep()));
            break;
        case Qt::Key_Left:
            canvasControllerWidget->pan(QPoint(-canvasControllerWidget->horizontalScrollBar()->singleStep(), 0));
            break;
        case Qt::Key_Right:
            canvasControllerWidget->pan(QPoint(canvasControllerWidget->horizontalScrollBar()->singleStep(), 0));
            break;
        case Qt::Key_Shift:
            m_rotationMode = true;
            kritaCanvas()->updateCanvas();
            adjustCursor();
            break;
    }
    event->accept();
}

void KisToolPan::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Shift:
            m_rotationMode = false;
            kritaCanvas()->updateCanvas();
            adjustCursor();
            break;
    }
}

#include "kis_tool_pan.moc"
