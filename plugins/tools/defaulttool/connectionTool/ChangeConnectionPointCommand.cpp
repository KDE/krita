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

#include "ChangeConnectionPointCommand.h"
#include <KoShape.h>

ChangeConnectionPointCommand::ChangeConnectionPointCommand(KoShape *shape, int connectionPointId, const KoConnectionPoint &oldPoint, const KoConnectionPoint &newPoint, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_connectionPointId(connectionPointId)
    , m_oldPoint(oldPoint)
    , m_newPoint(newPoint)
{
    Q_ASSERT(m_shape);
}

ChangeConnectionPointCommand::~ChangeConnectionPointCommand()
{
}

void ChangeConnectionPointCommand::redo()
{
    updateRoi(m_oldPoint.position);
    m_shape->setConnectionPoint(m_connectionPointId, m_newPoint);
    updateRoi(m_newPoint.position);

    KUndo2Command::redo();
}

void ChangeConnectionPointCommand::undo()
{
    KUndo2Command::undo();

    updateRoi(m_newPoint.position);
    m_shape->setConnectionPoint(m_connectionPointId, m_oldPoint);
    updateRoi(m_oldPoint.position);
}

void ChangeConnectionPointCommand::updateRoi(const QPointF &/*position*/)
{
    // TODO: is there a way we can get at the correct update size?
    //QRectF roi(0, 0, 10, 10);
    //roi.moveCenter(position);
    //m_shape->update(roi);
}
