/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPathPointMoveStrategy.h"

#include "commands/KoPathPointMoveCommand.h"
#include "KoPathTool.h"

KoPathPointMoveStrategy::KoPathPointMoveStrategy( KoPathTool *tool, KoCanvasBase *canvas, const QPointF &pos )
: KoInteractionStrategy( tool, canvas )
, m_lastPosition( pos )
, m_move( 0, 0 )
, m_tool( tool )
{
}

KoPathPointMoveStrategy::~KoPathPointMoveStrategy() 
{
}

void KoPathPointMoveStrategy::handleMouseMove( const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers )
{
    QPointF docPoint = m_tool->snapToGrid( mouseLocation, modifiers );
    QPointF move = docPoint - m_lastPosition;
    // as the last position can change when the top left is changed we have
    // to save it in document pos and not in shape pos
    m_lastPosition = docPoint;

    m_move += move;

    KoPathPointMoveCommand cmd( m_tool->m_pointSelection.selectedPointMap(), move );
    cmd.redo();
}

void KoPathPointMoveStrategy::finishInteraction( Qt::KeyboardModifiers modifiers ) 
{ 
    Q_UNUSED( modifiers );
}

QUndoCommand* KoPathPointMoveStrategy::createCommand()
{
    QUndoCommand *cmd = 0;
    if( !m_move.isNull() )
    {
        // as the point is already at the new position we need to undo the command
        cmd = new KoPathPointMoveCommand( m_tool->m_pointSelection.selectedPointMap(), m_move );
        cmd->undo();
    }
    return cmd;
}
