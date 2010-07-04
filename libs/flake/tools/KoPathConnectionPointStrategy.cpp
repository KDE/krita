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
        m_oldConnectionIndex = m_connectionShape->firstConnectionIndex();
    } else {
        m_oldConnectionShape = m_connectionShape->secondShape();
        m_oldConnectionIndex = m_connectionShape->secondConnectionIndex();
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
    m_newConnectionIndex = -1;

    QRectF roi(mouseLocation - QPointF(MAX_DISTANCE, MAX_DISTANCE), QSizeF(2*MAX_DISTANCE, 2*MAX_DISTANCE));
    QList<KoShape*> shapes = m_tool->canvas()->shapeManager()->shapesAt(roi, true);
    if (shapes.count() < 2) {
        // we are not near any other shape, so remove the corresponding connection
        if (m_handleId == 0)
            m_connectionShape->connectFirst(0, -1);
        else
            m_connectionShape->connectSecond(0, -1);

        KoParameterChangeStrategy::handleMouseMove(mouseLocation, modifiers);
    } else {
        qreal minimalDistance = DBL_MAX;
        QPointF nearestPoint;
        KoShape *nearestShape = 0;
        int nearestPointIndex = -1;
        bool nearestAlreadyPresent = false;

        foreach(KoShape* shape, shapes) {
            // we do not want to connect to ourself
            if (shape == m_connectionShape)
                continue;

            bool alreadyPresent = true;
            QTransform m = shape->absoluteTransformation(0);
            QList<QPointF> connectionPoints = shape->connectionPoints();
            if (! connectionPoints.count()) {
                QSizeF size = shape->size();
                connectionPoints.append(QPointF(0.0, 0.0));
                connectionPoints.append(QPointF(size.width(), 0.0));
                connectionPoints.append(QPointF(size.width(), size.height()));
                connectionPoints.append(QPointF(0.0, size.height()));
                connectionPoints.append(0.5 * (connectionPoints[0] + connectionPoints[1]));
                connectionPoints.append(0.5 * (connectionPoints[1] + connectionPoints[2]));
                connectionPoints.append(0.5 * (connectionPoints[2] + connectionPoints[3]));
                connectionPoints.append(0.5 * (connectionPoints[3] + connectionPoints[0]));
                alreadyPresent = false;
            }
            QPointF localMousePosition = shape->absoluteTransformation(0).inverted().map(mouseLocation);
            int connectionPointCount = connectionPoints.count();
            for (int i = 0; i < connectionPointCount; ++i) {
                QPointF difference = localMousePosition - connectionPoints[i];
                qreal distance = difference.x() * difference.x() + difference.y() * difference.y();
                if (distance > MAX_DISTANCE_SQR)
                    continue;
                if (distance < minimalDistance) {
                    nearestShape = shape;
                    nearestPoint = connectionPoints[i];
                    nearestPointIndex = i;
                    nearestAlreadyPresent = alreadyPresent;
                    minimalDistance = distance;
                }
            }
        }

        if (nearestShape) {
            if (! nearestAlreadyPresent) {
                //nearestShape->addConnectionPoint( nearestPoint );
                nearestPointIndex = -1;
            }
            nearestPoint = nearestShape->absoluteTransformation(0).map(nearestPoint);
        } else {
            nearestPoint = mouseLocation;
            nearestPointIndex = -1;
        }
        m_newConnectionShape = nearestShape;
        m_newConnectionIndex = nearestPointIndex;
        if (m_handleId == 0)
            m_connectionShape->connectFirst(nearestShape, nearestPointIndex);
        else
            m_connectionShape->connectSecond(nearestShape, nearestPointIndex);
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
    if (m_newConnectionShape && m_newConnectionIndex == -1) {
        // map handle position into document coordinates
        QPointF p = m_connectionShape->shapeToDocument(m_connectionShape->handlePosition(m_handleId));
        // and add as connection point in shape coordinates
        m_newConnectionShape->addConnectionPoint(m_newConnectionShape->absoluteTransformation(0).inverted().map(p));
        m_newConnectionIndex = m_newConnectionShape->connectionPoints().count() - 1;
    }

    // set the connection corresponding to the handle we are working on
    if (m_handleId == 0)
        m_connectionShape->connectFirst(m_newConnectionShape, m_newConnectionIndex);
    else
        m_connectionShape->connectSecond(m_newConnectionShape, m_newConnectionIndex);

    // TODO create a connection command
    return KoParameterChangeStrategy::createCommand();
}

