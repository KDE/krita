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

#include "RemoveConnectionPointCommand.h"
#include <KoShape.h>

RemoveConnectionPointCommand::RemoveConnectionPointCommand(KoShape *shape, int connectionPointId, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_shape(shape)
    , m_connectionPointId(connectionPointId)
{
    Q_ASSERT(m_shape);
    m_connectionPoint = m_shape->connectionPoint(m_connectionPointId);
}

RemoveConnectionPointCommand::~RemoveConnectionPointCommand()
{
}

void RemoveConnectionPointCommand::redo()
{
    // TODO: check if there is a connection shape attached
    // and handle that appropriately
    m_shape->removeConnectionPoint(m_connectionPointId);
    updateRoi();

    KUndo2Command::redo();
}

void RemoveConnectionPointCommand::undo()
{
    KUndo2Command::undo();

    m_shape->setConnectionPoint(m_connectionPointId, m_connectionPoint);
    updateRoi();
}

void RemoveConnectionPointCommand::updateRoi()
{
    // TODO: is there a way we can get at the correct update size?
    QRectF roi(0, 0, 10, 10);
    roi.moveCenter(m_connectionPoint.position);
    m_shape->update(roi);
}
