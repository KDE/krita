/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "KoParameterChangeStrategy.h"

#include "KoParameterShape.h"
#include "commands/KoParameterHandleMoveCommand.h"

KoParameterChangeStrategy::KoParameterChangeStrategy(KoToolBase *tool, KoParameterShape *parameterShape, int handleId)
        : KoInteractionStrategy(tool)
        , m_parameterShape(parameterShape)
        , m_handleId(handleId)
        , m_startPoint(m_parameterShape->shapeToDocument(m_parameterShape->handlePosition(handleId)))
        , m_lastModifierUsed(0)
{
    // initialize release point with start point position to prevent
    // change when just clicking a handle without moving the mouse
    m_releasePoint = m_startPoint;
}

KoParameterChangeStrategy::~KoParameterChangeStrategy()
{
}

void KoParameterChangeStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    m_parameterShape->moveHandle(m_handleId, mouseLocation, modifiers);
    m_lastModifierUsed = modifiers;
    m_releasePoint = mouseLocation;
}

QUndoCommand* KoParameterChangeStrategy::createCommand()
{
    KoParameterHandleMoveCommand *cmd = 0;
    // check if handle position changed
    if (m_startPoint != QPointF(0, 0) && m_startPoint != m_releasePoint) {
        cmd = new KoParameterHandleMoveCommand(m_parameterShape, m_handleId, m_startPoint, m_releasePoint, m_lastModifierUsed);
    }
    return cmd;
}

