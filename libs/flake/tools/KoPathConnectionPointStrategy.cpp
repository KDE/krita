/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007,2009,2011 Jan Hambrecht <jaham@gmx.net>
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
#include "KoPathConnectionPointStrategy_p.h"
#include "KoConnectionShape.h"
#include "KoCanvasBase.h"
#include "KoShapeManager.h"
#include "KoShapeConnectionChangeCommand.h"

#include <float.h>
#include <math.h>

const int InvalidConnectionPointId = INT_MIN;

KoPathConnectionPointStrategy::KoPathConnectionPointStrategy(KoToolBase *tool, KoConnectionShape *shape, int handleId)
    : KoParameterChangeStrategy(*(new KoPathConnectionPointStrategyPrivate(tool, shape, handleId)))
{
}

KoPathConnectionPointStrategy::~KoPathConnectionPointStrategy()
{
}

void KoPathConnectionPointStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_D(KoPathConnectionPointStrategy);

    const qreal MAX_DISTANCE = 20.0; // TODO make user definable
    const qreal MAX_DISTANCE_SQR = MAX_DISTANCE * MAX_DISTANCE;

    d->newConnectionShape = 0;
    d->newConnectionId = InvalidConnectionPointId;

    QRectF roi(mouseLocation - QPointF(MAX_DISTANCE, MAX_DISTANCE), QSizeF(2*MAX_DISTANCE, 2*MAX_DISTANCE));
    QList<KoShape*> shapes = d->tool->canvas()->shapeManager()->shapesAt(roi, true);
    if (shapes.count() < 2) {
        // we are not near any other shape, so remove the corresponding connection
        if (d->handleId == 0)
            d->connectionShape->connectFirst(0, InvalidConnectionPointId);
        else
            d->connectionShape->connectSecond(0, InvalidConnectionPointId);

        KoParameterChangeStrategy::handleMouseMove(mouseLocation, modifiers);
    } else {
        qreal minimalDistance = DBL_MAX;
        QPointF nearestPoint;
        KoShape *nearestShape = 0;
        int nearestPointId = InvalidConnectionPointId;

        Q_FOREACH (KoShape* shape, shapes) {
            // we do not want to connect to ourself
            if (shape == d->connectionShape)
                continue;

            KoConnectionPoints connectionPoints = shape->connectionPoints();
            if (! connectionPoints.count()) {
                QSizeF size = shape->size();
                connectionPoints[-1] = QPointF(0.0, 0.0);
                connectionPoints[-2] = QPointF(size.width(), 0.0);
                connectionPoints[-3] = QPointF(size.width(), size.height());
                connectionPoints[-4] = QPointF(0.0, size.height());
                connectionPoints[-5] = 0.5 * (connectionPoints[-1].position + connectionPoints[-2].position);
                connectionPoints[-6] = 0.5 * (connectionPoints[-2].position + connectionPoints[-3].position);
                connectionPoints[-7] = 0.5 * (connectionPoints[-3].position + connectionPoints[-4].position);
                connectionPoints[-8] = 0.5 * (connectionPoints[-4].position + connectionPoints[-1].position);
            }
            QPointF localMousePosition = shape->absoluteTransformation().inverted().map(mouseLocation);
            KoConnectionPoints::const_iterator cp = connectionPoints.constBegin();
            KoConnectionPoints::const_iterator lastCp = connectionPoints.constEnd();
            for(; cp != lastCp; ++cp) {
                QPointF difference = localMousePosition - cp.value().position;
                qreal distance = difference.x() * difference.x() + difference.y() * difference.y();
                if (distance > MAX_DISTANCE_SQR)
                    continue;
                if (distance < minimalDistance) {
                    nearestShape = shape;
                    nearestPoint = cp.value().position;
                    nearestPointId = cp.key();
                    minimalDistance = distance;
                }
            }
        }

        if (nearestShape) {
            nearestPoint = nearestShape->absoluteTransformation().map(nearestPoint);
        } else {
            nearestPoint = mouseLocation;
        }
        d->newConnectionShape = nearestShape;
        d->newConnectionId = nearestPointId;
        if (d->handleId == 0)
            d->connectionShape->connectFirst(nearestShape, nearestPointId);
        else
            d->connectionShape->connectSecond(nearestShape, nearestPointId);
        KoParameterChangeStrategy::handleMouseMove(nearestPoint, modifiers);
    }
}

void KoPathConnectionPointStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    KoParameterChangeStrategy::finishInteraction(modifiers);
}

KUndo2Command* KoPathConnectionPointStrategy::createCommand()
{
    Q_D(KoPathConnectionPointStrategy);

    // check if we connect to a shape and if the connection point is already present
    if (d->newConnectionShape && d->newConnectionId < 0 && d->newConnectionId != InvalidConnectionPointId) {
        // map handle position into document coordinates
        QPointF p = d->connectionShape->shapeToDocument(d->connectionShape->handlePosition(d->handleId));
        // and add as connection point in shape coordinates
        d->newConnectionId = d->newConnectionShape->addConnectionPoint(d->newConnectionShape->absoluteTransformation().inverted().map(p));
    }

    KUndo2Command *cmd = KoParameterChangeStrategy::createCommand();
    if (!cmd)
        return 0;

    // change connection
    new KoShapeConnectionChangeCommand(d->connectionShape, static_cast<KoConnectionShape::HandleId>(d->handleId),
                                       d->oldConnectionShape, d->oldConnectionId, d->newConnectionShape, d->newConnectionId, cmd);
    return cmd;
}
