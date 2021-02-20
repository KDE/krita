/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoPathControlPointMoveStrategy.h"
#include "KoCanvasBase.h"
#include "KoSnapGuide.h"

#include "KoPathTool.h"
#include "commands/KoPathControlPointMoveCommand.h"

KoPathControlPointMoveStrategy::KoPathControlPointMoveStrategy(KoPathTool *tool, const KoPathPointData &pointData, KoPathPoint::PointType type, const QPointF &pos)
        : KoInteractionStrategy(tool)
        , m_lastPosition(pos)
        , m_move(0, 0)
        , m_tool(tool)
        , m_pointData(pointData)
        , m_pointType(type)
{
}

KoPathControlPointMoveStrategy::~KoPathControlPointMoveStrategy()
{
}

void KoPathControlPointMoveStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    QPointF docPoint = m_tool->canvas()->snapGuide()->snap(mouseLocation, modifiers);
    QPointF move = docPoint - m_lastPosition;
    // as the last position can change when the top left is changed we have
    // to save it in document pos and not in shape pos
    m_lastPosition = docPoint;

    m_move += move;

    KoPathControlPointMoveCommand cmd(m_pointData, move, m_pointType);
    cmd.redo();
}

void KoPathControlPointMoveStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
}

KUndo2Command* KoPathControlPointMoveStrategy::createCommand()
{
    KUndo2Command *cmd = 0;
    if (!m_move.isNull()) {
        cmd = new KoPathControlPointMoveCommand(m_pointData, m_move, m_pointType);
        cmd->undo();
    }
    return cmd;
}
