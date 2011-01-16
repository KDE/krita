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
#include "MoveConnectionPointCommand.h"
#include <KoShape.h>
#include <KoToolBase.h>
#include <KoCanvasBase.h>
#include <KoResourceManager.h>

MoveConnectionPointStrategy::MoveConnectionPointStrategy(KoShape *shape, int connectionPointId, KoToolBase* parent)
: KoInteractionStrategy(parent), m_shape(shape), m_connectionPointId(connectionPointId)
{
    Q_ASSERT(m_shape);
    m_oldPosition = m_newPosition = m_shape->connectionPoint(m_connectionPointId).position;
}

MoveConnectionPointStrategy::~MoveConnectionPointStrategy()
{

}

void MoveConnectionPointStrategy::paint(QPainter& painter, const KoViewConverter& converter)
{
    KoInteractionStrategy::paint(painter, converter);
}

void MoveConnectionPointStrategy::handleMouseMove(const QPointF& mouseLocation, Qt::KeyboardModifiers /*modifiers*/)
{
    m_newPosition = m_shape->documentToShape(mouseLocation);
    m_shape->setConnectionPointPosition(m_connectionPointId, m_newPosition);
}

void MoveConnectionPointStrategy::cancelInteraction()
{
    KoInteractionStrategy::cancelInteraction();
    m_shape->setConnectionPointPosition(m_connectionPointId, m_oldPosition);
}

void MoveConnectionPointStrategy::finishInteraction(Qt::KeyboardModifiers /*modifiers*/)
{
}

QUndoCommand* MoveConnectionPointStrategy::createCommand()
{
    int grabDistance = tool()->canvas()->resourceManager()->grabSensitivity();
    const qreal dx = m_newPosition.x()-m_oldPosition.x();
    const qreal dy = m_newPosition.y()-m_oldPosition.y();
    // check if we have moved the connection point at least a little bit
    if(dx*dx+dy*dy < grabDistance*grabDistance)
        return 0;

    return new MoveConnectionPointCommand(m_shape, m_connectionPointId, m_oldPosition, m_newPosition);
}
