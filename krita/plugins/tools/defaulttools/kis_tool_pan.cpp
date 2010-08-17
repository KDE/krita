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

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasControllerWidget.h>

#include <kis_canvas2.h>

#include "kis_cursor.h"
#include "kis_coordinates_converter.h"


KisToolPan::KisToolPan(KoCanvasBase * canvas)
        :  KisTool(canvas, KisCursor::openHandCursor())
{
    setObjectName("tool_pan");
    m_rotationMode = false;
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

        const qreal checkerRadius = 50;
        QBrush fillBrush(QColor(0,0,0,100));
        QPen checkerPen(QColor(255,255,255,100), 5., Qt::SolidLine, Qt::RoundCap);

        gc.save();

        gc.setPen(Qt::NoPen);
        gc.setBrush(fillBrush);
        gc.drawEllipse(centerPoint, checkerRadius, checkerRadius);

        gc.setPen(checkerPen);
        gc.drawLine(centerPoint, centerPoint + QPointF(0, -checkerRadius));

        gc.restore();
    }
}

KisCanvas2* KisToolPan::kritaCanvas() const
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);
    return kritaCanvas;
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
    useCursor(KisCursor::closedHandCursor());
}

void KisToolPan::mouseMoveEvent(KoPointerEvent *e)
{
    Q_ASSERT(canvas());
    Q_ASSERT(canvas()->canvasController());

    if (!e->buttons())
        return;
    e->accept();

    QPointF actualPosition = convertDocumentToWidget(e->point);

    if(e->modifiers() & Qt::ShiftModifier) {
        qreal angle = calculateAngle(m_lastPosition, actualPosition);
        kritaCanvas()->rotateCanvas(angle);
    }
    else {
        QPointF distance(m_lastPosition - actualPosition);
        canvas()->canvasController()->pan(distance.toPoint());
    }

    m_lastPosition = actualPosition;
}

void KisToolPan::mouseReleaseEvent(KoPointerEvent *e)
{
    Q_UNUSED(e);
    useCursor(KisCursor::openHandCursor());
}

void KisToolPan::keyPressEvent(QKeyEvent *event)
{
    KoCanvasControllerWidget *canvasControllerWidget = dynamic_cast<KoCanvasControllerWidget*>(canvas()->canvasController());
    if (!canvasControllerWidget) {
        return;
    }
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
            break;
    }
}

#include "kis_tool_pan.moc"
