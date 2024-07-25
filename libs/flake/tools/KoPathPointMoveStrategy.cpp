/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoPathPointMoveStrategy.h"
#include "KoInteractionStrategy_p.h"

#include "commands/KoPathPointMoveCommand.h"
#include "KoPathTool.h"
#include "KoPathToolSelection.h"
#include "KoSnapGuide.h"
#include "KoCanvasBase.h"
#include "kis_global.h"
#include "kis_command_utils.h"

KoPathPointMoveStrategy::KoPathPointMoveStrategy(KoPathTool *tool, const QPointF &mousePosition, const QPointF &pointPosition)
    : KoInteractionStrategy(*(new KoInteractionStrategyPrivate(tool))),
    m_startMousePosition(mousePosition),
    m_startPointPosition(pointPosition),
    m_tool(tool)
{
}

KoPathPointMoveStrategy::~KoPathPointMoveStrategy()
{
}

void KoPathPointMoveStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    QPointF deltaMovement = mouseLocation - m_startMousePosition;
    QPointF newPosition = m_tool->canvas()->snapGuide()->snap(m_startPointPosition + deltaMovement, modifiers);
    QPointF move = newPosition - m_startPointPosition;

    if (modifiers & Qt::ShiftModifier) {
        // Limit change to one direction only
        move = snapToClosestAxis(move);
    }

    KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());
    if (! selection)
        return;

    KoPathPointMoveCommand *cmd = new KoPathPointMoveCommand(selection->selectedPointsData(), move - m_move);

    cmd->redo();
    if (m_intermediateCommand) {
        m_intermediateCommand->mergeWith(cmd);
    } else {
        m_intermediateCommand.reset(cmd);
    }
    m_move = move;
}

void KoPathPointMoveStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
}

KUndo2Command* KoPathPointMoveStrategy::createCommand()
{
    if (m_intermediateCommand) {
        return new KisCommandUtils::SkipFirstRedoWrapper(m_intermediateCommand.take());
    }
    return nullptr;
}
