/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

KoParameterChangeStrategy::KoParameterChangeStrategy( KoTool *tool, KoCanvasBase *canvas, KoParameterShape * parameterShape, int handleId )
: KoInteractionStrategy( tool, canvas )
, m_parameterShape( parameterShape )
, m_handleId( handleId )    
, m_startPoint( m_parameterShape->shapeToDocument( m_parameterShape->handlePosition( handleId ) ) )
{
}

KoParameterChangeStrategy::~KoParameterChangeStrategy()
{
}

void KoParameterChangeStrategy::handleMouseMove( const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers )
{
    m_parameterShape->moveHandle( m_handleId, mouseLocation, modifiers );
}

QUndoCommand* KoParameterChangeStrategy::createCommand()
{
    KoParameterHandleMoveCommand *cmd = 0;
    // check if handle position changed
    if ( m_startPoint != m_parameterShape->handlePosition( m_handleId ) )
    {
        cmd = new KoParameterHandleMoveCommand( m_parameterShape, m_handleId, m_startPoint, m_parameterShape->shapeToDocument( m_parameterShape->handlePosition( m_handleId ) ) );
    }
    return cmd;
}

