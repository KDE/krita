/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007,2009 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPathConnectionPointStrategy.h"

#include "KoPathTool.h"
#include "KoCanvasBase.h"
#include "KoShapeManager.h"
#include "KoFlake.h"

#include <kdebug.h>

#include <float.h>
#include <math.h>

const int InvalidConnectionPointId = INT_MIN;

KoPathConnectionPointStrategy::KoPathConnectionPointStrategy(KoPathTool *tool,
        KoConnectionShape *shape, int handleId)
        : KoParameterChangeStrategy(tool, shape, handleId)
        , m_tool(tool)
        , m_connectionShape(shape)
        , m_handleId(handleId)
        , m_startPoint(m_connectionShape->shapeToDocument(m_connectionShape->handlePosition(handleId)))
{
    if (handleId == 0) {
        m_oldConnectionShape = m_connectionShape->firstShape();
        m_oldConnectionId = m_connectionShape->firstConnectionId();
    } else {
        m_oldConnectionShape = m_connectionShape->secondShape();
        m_oldConnectionId = m_connectionShape->secondConnectionId();
    }
}

KoPathConnectionPointStrategy::~KoPathConnectionPointStrategy()
{
}

void KoPathConnectionPointStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    const qreal MAX_DISTANCE = 20.0; // TODO make user definable
    const qreal MAX_DISTANCE_SQR = MAX_DISTANCE * MAX_DISTANCE;

    m_newConnectionShape = 0;
    m_newConnectionId = InvalidConnectionPointId;

    QRectF roi(mouseLocation - QPointF(MAX_DISTANCE, MAX_DISTANCE), QSizeF(2*MAX_DISTANCE, 2*MAX_DISTANCE));
    QList<KoShape*> shapes = m_tool->canvas()->shapeManager()->shapesAt(roi, true);
    if (shapes.count() < 2) {
        // we are not near any other shape, so remove the corresponding connection
        if (m_handleId == 0)
            m_connectionShape->connectFirst(0, InvalidConnectionPointId);
        else
            m_connectionShape->connectSecond(0, InvalidConnectionPointId);

        KoParameterChangeStrategy::handleMouseMove(mouseLocation, modifiers);
    } else {
        qreal minimalDistance = DBL_MAX;
        QPointF nearestPoint;
        KoShape *nearestShape = 0;
        int nearestPointId = InvalidConnectionPointId;

        foreach(KoShape* shape, shapes) {
            // we do not want to connect to ourself
            if (shape == m_connectionShape)
                continue;

            QTransform m = shape->absoluteTransformation(0);
            KoConnectionPoints connectionPoints = shape->connectionPoints();
            if (! connectionPoints.count()) {
                QSizeF size = shape->size();
                connectionPoints[-1] = QPointF(0.0, 0.0);
                connectionPoints[-2] = QPointF(size.width(), 0.0);
                connectionPoints[-3] = QPointF(size.width(), size.height());
                connectionPoints[-4] = QPointF(0.0, size.height());
                connectionPoints[-5] = 0.5 * (connectionPoints[-1] + connectionPoints[-2]);
                connectionPoints[-6] = 0.5 * (connectionPoints[-2] + connectionPoints[-3]);
                connectionPoints[-7] = 0.5 * (connectionPoints[-3] + connectionPoints[-4]);
                connectionPoints[-8] = 0.5 * (connectionPoints[-4] + connectionPoints[-1]);
            }
            QPointF localMousePosition = shape->absoluteTransformation(0).inverted().map(mouseLocation);
            KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
            KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
            for(; cp != lastCp; ++cp) {
                QPointF difference = localMousePosition - cp.value();
                qreal distance = difference.x() * difference.x() + difference.y() * difference.y();
                if (distance > MAX_DISTANCE_SQR)
                    continue;
                if (distance < minimalDistance) {
                    nearestShape = shape;
                    nearestPoint = cp.value();
                    nearestPointId = cp.key();
                    minimalDistance = distance;
                }
            }
        }

        if (nearestShape) {
            nearestPoint = nearestShape->absoluteTransformation(0).map(nearestPoint);
        } else {
            nearestPoint = mouseLocation;
        }
        m_newConnectionShape = nearestShape;
        m_newConnectionId = nearestPointId;
        if (m_handleId == 0)
            m_connectionShape->connectFirst(nearestShape, nearestPointId);
        else
            m_connectionShape->connectSecond(nearestShape, nearestPointId);
        KoParameterChangeStrategy::handleMouseMove(nearestPoint, modifiers);
    }
}

void KoPathConnectionPointStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    KoParameterChangeStrategy::finishInteraction(modifiers);
}

QUndoCommand* KoPathConnectionPointStrategy::createCommand()
{
    // check if we connect to a shape and if the connection point is already present
    if (m_newConnectionShape && m_newConnectionId < 0 && m_newConnectionId != InvalidConnectionPointId) {
        // map handle position into document coordinates
        QPointF p = m_connectionShape->shapeToDocument(m_connectionShape->handlePosition(m_handleId));
        // and add as connection point in shape coordinates
        m_newConnectionId = m_newConnectionShape->addConnectionPoint(m_newConnectionShape->absoluteTransformation(0).inverted().map(p));
    }

    // set the connection corresponding to the handle we are working on
    if (m_handleId == 0)
        m_connectionShape->connectFirst(m_newConnectionShape, m_newConnectionId);
    else
        m_connectionShape->connectSecond(m_newConnectionShape, m_newConnectionId);

    // TODO create a connection command
    return KoParameterChangeStrategy::createCommand();
}

