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

#include "KoPathBreakAtPointCommand.h"
#include <klocale.h>

/*
 * The algorithm to break a multiple open or closed subpaths is:
 * Subpath is closed
 * - open behind the last point in the subpath 
 * - go on like as described in Not closed 
 * Not closed 
 * - break from the back of the subpath
 */
KoPathBreakAtPointCommand::KoPathBreakAtPointCommand( const QList<KoPathPointData> & pointDataList, QUndoCommand *parent )
: QUndoCommand( parent )
, m_deletePoints( true )
{
    QList<KoPathPointData> sortedPointDataList( pointDataList );
    qSort( sortedPointDataList );
    setText( i18n( "Break subpath at points" ) );

    QList<KoPathPointData>::const_iterator it( sortedPointDataList.begin() );
    for ( ; it != sortedPointDataList.end(); ++it )
    {
        KoPathPoint *point = it->m_pathShape->pointByIndex( it->m_pointIndex );
        if ( point )
        {
            m_pointDataList.append( *it );
            m_points.push_back( new KoPathPoint( *point ) );
            m_closedIndex.push_back( KoPathPointIndex( -1, 0 ) );
        }
    }

    KoPathPointData last( 0, KoPathPointIndex( -1, -1 ) );
    for ( int i = m_pointDataList.size() - 1; i >= 0; --i )
    {
        const KoPathPointData &current = m_pointDataList.at( i );

        if ( last.m_pathShape != current.m_pathShape || last.m_pointIndex.first != current.m_pointIndex.first )
        {
            last = current;
            if ( current.m_pathShape->isClosedSubpath( current.m_pointIndex.first ) )
            {
                // the break will happen before the inserted point so we have to increment by 1
                m_closedIndex[i] = current.m_pointIndex;
                ++m_closedIndex[i].second;
            }
        }
    }
}

KoPathBreakAtPointCommand::~KoPathBreakAtPointCommand()
{
    if ( m_deletePoints )
    {
        qDeleteAll( m_points );
    }
}

void KoPathBreakAtPointCommand::redo()
{
    QUndoCommand::redo();
    KoPathPointData last( 0, KoPathPointIndex( -1, -1 ) );

    // offset, needed when path was opened
    int offset = 0;
    for ( int i = m_pointDataList.size() - 1; i >= 0; --i )
    {
        const KoPathPointData & pd = m_pointDataList.at( i );
        KoPathShape * pathShape = pd.m_pathShape;

        KoPathPointIndex pointIndex = pd.m_pointIndex;
        if ( last.m_pathShape != pathShape || last.m_pointIndex.first != pointIndex.first )
        {
            offset = 0;
        }

        pointIndex.second = pointIndex.second + offset + 1;
        pathShape->insertPoint( m_points[i], pointIndex );

        if ( m_closedIndex.at( i ).first != -1 )
        {
            m_closedIndex[i] = pathShape->openSubpath( m_closedIndex.at( i ) );
            offset = m_closedIndex.at( i ).second;
        }
        else
        {
            KoPathPointIndex breakIndex = pd.m_pointIndex;
            breakIndex.second += offset;
            pathShape->breakAfter( breakIndex );
            m_closedIndex[i].second = offset;
        }

        if ( last.m_pathShape != pathShape )
        {
            if ( last.m_pathShape )
            {
                last.m_pathShape->repaint();
            }
            last = pd;
        }
    }
    if ( last.m_pathShape )
    {
        last.m_pathShape->repaint();
    }

    m_deletePoints = false;
}

void KoPathBreakAtPointCommand::undo()
{
    QUndoCommand::undo();
    KoPathShape * lastPathShape = 0;

    for ( int i = 0; i < m_pointDataList.size(); ++i )
    {
        const KoPathPointData & pd = m_pointDataList.at( i );
        KoPathShape * pathShape = pd.m_pathShape; 
        KoPathPointIndex pointIndex = pd.m_pointIndex;
        ++pointIndex.second;
        if ( m_closedIndex.at( i ).first != -1 )
        {
            m_closedIndex[i] = pathShape->closeSubpath( m_closedIndex.at( i ) );
        }
        else
        {
            pointIndex.second = pointIndex.second + m_closedIndex.at( i ).second;
            pathShape->join( pd.m_pointIndex.first );
        }
        m_points[i] = pathShape->removePoint( pointIndex );

        if ( lastPathShape != pathShape )
        {
            if ( lastPathShape )
            {
                lastPathShape->repaint();
            }
            lastPathShape = pathShape;
        }
    }
    if ( lastPathShape )
    {
        lastPathShape->repaint();
    }

    m_deletePoints = true;
}
