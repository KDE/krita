/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ConnectionTool.h"

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoViewConverter.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoShapeConnection.h>

#include <kdebug.h>
#include <QPainter>

ConnectionTool::ConnectionTool(KoCanvasBase *canvas)
: KoTool(canvas),
m_connection(0)
{
}

ConnectionTool::~ConnectionTool() {
}


void ConnectionTool::paint( QPainter &painter, const KoViewConverter &converter ) {
    QList<KoShape*> repaints = m_shapesPaintedWithConnections;
    if(m_connection && m_connection->shape1())
        repaints.append(m_connection->shape1());
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    foreach(KoShape *shape, repaints) {
        // TODO check if they are not clipped.
        QMatrix matrix = shape->absoluteTransformation(&converter);
        matrix.scale(zoomX, zoomY);
        foreach(QPointF point, shape->connectors()) {
            point = matrix.map(point);
            painter.fillRect(QRectF(QPointF(point.x() - 2, point.y() -2) , QSizeF(4, 4)), QBrush(Qt::red));
        }
    }

    if(m_connection)
    {
        m_connection->paint(painter, converter);
    }
}

void ConnectionTool::mousePressEvent( KoPointerEvent *event ) {
    QRectF region = m_canvas->viewConverter()->viewToDocument(QRectF(0, 0, 20, 20));
    region.moveTo(event->point.x() - region.width() / 2, event->point.y() - region.height() / 2);
    m_lastMousePos = event->point;
    bool created = false;

    foreach(KoShape *shape, m_canvas->shapeManager()->shapesAt(region)) {
        QMatrix matrix = shape->absoluteTransformation(0);
        int index = 0;
        foreach(QPointF point, shape->connectors()) {
            QPointF p = matrix.map(point);
            QPointF distance = m_canvas->viewConverter()->documentToView(p - event->point);
            if(qAbs(distance.x()) < 10 && qAbs(distance.y()) < 10) { // distance is in pixels.
                if(m_connection == 0) {
                    m_connection = createConnection(shape, index, event->point);
                    created = true;
                }
                else {
                    m_connection->setEndPoint(shape, index);
                    m_connection = 0;
                }
                return;
            }

            index++;
        }
    }

    if(event->button() == Qt::LeftButton)
    {
        if(m_connection && !created)
        {
            m_connection->appendControlPoint(event->point);
        }
    }
    else if(event->button() == Qt::RightButton)
    {
        if(m_connection)
        {
            m_connection->setEndPoint(event->point);
            m_connection = 0;
        }
    }
}

void ConnectionTool::mouseMoveEvent( KoPointerEvent *event ) {
    QRectF region = m_canvas->viewConverter()->viewToDocument(QRectF(0, 0, 50, 50));
    region.moveTo(event->point.x() - region.width() / 2, event->point.y() - region.height() / 2);
    QList<KoShape*> oldList = m_shapesPaintedWithConnections;
    m_shapesPaintedWithConnections.clear();
    foreach(KoShape *shape, m_canvas->shapeManager()->shapesAt(region)) {
        if(shape->connectors().isEmpty())
            continue;
        if(! oldList.contains(shape))
// TODO do a for loop to repaint only the actual points.
            shape->update(); // because it will have connections painted on top.
        m_shapesPaintedWithConnections.append(shape);
    }
    foreach(KoShape *shape, oldList) {
        if(! m_shapesPaintedWithConnections.contains(shape))
// TODO do a for loop to repaint only the actual points.
            shape->update(); // because it used to have connections painted, but no longer will.
    }

    m_lastMousePos = event->point;

    if(m_connection)
    {
        // TODO Only update the rect that's needed
        m_canvas->updateCanvas(m_canvas->canvasWidget()->geometry());
        m_connection->setEndPoint (event->point);
    }
}

void ConnectionTool::mouseReleaseEvent( KoPointerEvent *event ) {
}

void ConnectionTool::activate (bool temporary) {
    Q_UNUSED(temporary);
    useCursor(Qt::ArrowCursor, true);
    m_connection = 0;
}

void ConnectionTool::deactivate() {
    m_connection = 0;
}

void ConnectionTool::createConnection(KoShape *shape1, int gluePointIndex1, KoShape *shape2, int gluePointIndex2) {
    kDebug() <<"create Connection!";
    new KoShapeConnection(shape1, gluePointIndex1, shape2, gluePointIndex2); // will add itself.
}

KoShapeConnection* ConnectionTool::createConnection(KoShape *shape, int gluePointIndex, const QPointF& endPoint)
{
    kDebug() <<"create Connection!";
    return new KoShapeConnection(shape, gluePointIndex, endPoint); // will add itself.
}

#include "ConnectionTool.moc"
