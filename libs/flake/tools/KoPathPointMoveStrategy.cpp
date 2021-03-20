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

KoPathPointMoveStrategy::KoPathPointMoveStrategy(KoPathTool *tool, const QPointF &pos)
    : KoInteractionStrategy(*(new KoInteractionStrategyPrivate(tool))),
    m_originalPosition(pos),
    m_tool(tool)
{
}

KoPathPointMoveStrategy::~KoPathPointMoveStrategy()
{
}

void KoPathPointMoveStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    QPointF newPosition = m_tool->canvas()->snapGuide()->snap(mouseLocation, modifiers);
    QPointF move = newPosition - m_originalPosition;

    if (modifiers & Qt::ShiftModifier) {
        // Limit change to one direction only
        move = snapToClosestAxis(move);
    }

    KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());
    if (! selection)
        return;

    KoPathPointMoveCommand cmd(selection->selectedPointsData(), move - m_move);
    cmd.redo();
    m_move = move;
}

void KoPathPointMoveStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
}

KUndo2Command* KoPathPointMoveStrategy::createCommand()
{
    KoPathToolSelection * selection = dynamic_cast<KoPathToolSelection*>(m_tool->selection());
    if (! selection)
        return 0;

    KUndo2Command *cmd = 0;
    if (!m_move.isNull()) {
        // as the point is already at the new position we need to undo the change
        KoPathPointMoveCommand revert(selection->selectedPointsData(), -m_move);
        revert.redo();
        cmd = new KoPathPointMoveCommand(selection->selectedPointsData(), m_move);
    }
    return cmd;
}
