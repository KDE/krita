/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPathSegmentBreakCommand.h"
#include <klocale.h>
#include <math.h>

KoPathSegmentBreakCommand::KoPathSegmentBreakCommand( const KoPathPointData & pointData, QUndoCommand *parent )
: QUndoCommand( parent )
, m_pointData( pointData )
, m_startIndex( -1, -1 )
, m_broken( false )
{
    if ( m_pointData.m_pathShape->isClosedSubpath( m_pointData.m_pointIndex.first ) )
    {
        m_startIndex = m_pointData.m_pointIndex;
        KoPathPoint * before = m_pointData.m_pathShape->pointByIndex( m_startIndex );
        if ( before->properties() & KoPathPoint::CloseSubpath )
        {
            m_startIndex.second = 0;
        }
        else
        {
            ++m_startIndex.second;
        }
    }
    setText( i18n( "Break subpath" ) );
}

KoPathSegmentBreakCommand::~KoPathSegmentBreakCommand()
{
}

void KoPathSegmentBreakCommand::redo()
{
    QUndoCommand::redo();
    // a repaint before is needed as the shape can shrink during the break
    m_pointData.m_pathShape->repaint();
    if ( m_startIndex.first != -1 )
    {
        m_startIndex = m_pointData.m_pathShape->openSubpath( m_startIndex );
        m_pointData.m_pathShape->normalize();
        m_pointData.m_pathShape->repaint();
    }
    else
    {
        m_broken = m_pointData.m_pathShape->breakAfter( m_pointData.m_pointIndex );
        if ( m_broken )
        {
            m_pointData.m_pathShape->normalize();
            m_pointData.m_pathShape->repaint();
        }
    }
}

void KoPathSegmentBreakCommand::undo()
{
    QUndoCommand::undo();
    if ( m_startIndex.first != -1 )
    {
        m_startIndex = m_pointData.m_pathShape->closeSubpath( m_startIndex );
        m_pointData.m_pathShape->normalize();
        m_pointData.m_pathShape->repaint();
    }
    else if ( m_broken )
    {
        m_pointData.m_pathShape->join( m_pointData.m_pointIndex.first );
        m_pointData.m_pathShape->normalize();
        m_pointData.m_pathShape->repaint();
    }
}
