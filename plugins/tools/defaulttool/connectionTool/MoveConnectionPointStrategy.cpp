/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#include "MoveConnectionPointStrategy.h"
#include "ChangeConnectionPointCommand.h"
#include <KoShape.h>
#include <KoToolBase.h>
#include <KoCanvasBase.h>

MoveConnectionPointStrategy::MoveConnectionPointStrategy(KoShape *shape, int connectionPointId, KoToolBase *parent)
    : KoInteractionStrategy(parent)
    , m_shape(shape)
    , m_connectionPointId(connectionPointId)
{
    Q_ASSERT(m_shape);
    m_oldPoint = m_newPoint = m_shape->connectionPoint(m_connectionPointId);
}

MoveConnectionPointStrategy::~MoveConnectionPointStrategy()
{
}

void MoveConnectionPointStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    KoInteractionStrategy::paint(painter, converter);
}

void MoveConnectionPointStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers /*modifiers*/)
{
    m_newPoint.position = m_shape->documentToShape(mouseLocation);
    m_shape->setConnectionPoint(m_connectionPointId, m_newPoint);
}

void MoveConnectionPointStrategy::cancelInteraction()
{
    KoInteractionStrategy::cancelInteraction();
    m_shape->setConnectionPoint(m_connectionPointId, m_oldPoint);
}

void MoveConnectionPointStrategy::finishInteraction(Qt::KeyboardModifiers /*modifiers*/)
{
}

KUndo2Command *MoveConnectionPointStrategy::createCommand()
{
    int grabDistance = grabSensitivity();
    const qreal dx = m_newPoint.position.x() - m_oldPoint.position.x();
    const qreal dy = m_newPoint.position.y() - m_oldPoint.position.y();
    // check if we have moved the connection point at least a little bit
    if (dx * dx + dy * dy < grabDistance * grabDistance) {
        return 0;
    }

    return new ChangeConnectionPointCommand(m_shape, m_connectionPointId, m_oldPoint, m_newPoint);
}
