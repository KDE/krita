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

#include "KoSubpathJoinCommand.h"
#include <klocale.h>

KoSubpathJoinCommand::KoSubpathJoinCommand( const KoPathPointData &pointData1, const KoPathPointData &pointData2, QUndoCommand *parent )
: QUndoCommand( parent )
, m_pointData1( pointData1 )
, m_pointData2( pointData2 )
, m_splitIndex( KoPathPointIndex( -1, -1 ) )
, m_oldProperties1( KoPathPoint::Normal )
, m_oldProperties2( KoPathPoint::Normal )
, m_reverse( 0 )
{
    Q_ASSERT( m_pointData1.m_pathShape == m_pointData2.m_pathShape );
    KoPathShape * pathShape = m_pointData1.m_pathShape;
    Q_ASSERT( !pathShape->isClosedSubpath( m_pointData1.m_pointIndex.first ) );
    Q_ASSERT( m_pointData1.m_pointIndex.second == 0 || 
              m_pointData1.m_pointIndex.second == pathShape->pointCountSubpath( m_pointData1.m_pointIndex.first ) - 1 );
    Q_ASSERT( !pathShape->isClosedSubpath( m_pointData2.m_pointIndex.first ) );
    Q_ASSERT( m_pointData2.m_pointIndex.second == 0 || 
              m_pointData2.m_pointIndex.second == pathShape->pointCountSubpath( m_pointData2.m_pointIndex.first ) - 1 );
    //TODO check that points are not the same

    if ( m_pointData2 < m_pointData1 )
        qSwap( m_pointData1, m_pointData2 );

    if ( m_pointData1.m_pointIndex.first != m_pointData2.m_pointIndex.first )
    {
        if ( m_pointData1.m_pointIndex.second == 0 && pathShape->pointCountSubpath( m_pointData1.m_pointIndex.first ) > 1 )
            m_reverse |= ReverseFirst;
        if ( m_pointData2.m_pointIndex.second != 0 )
            m_reverse |= ReverseSecond;
        setText( i18n( "Close subpath" ) );
    }
    else
    {
        setText( i18n( "Join subpaths" ) );
    }

    KoPathPoint * point1 = pathShape->pointByIndex( m_pointData1.m_pointIndex );
    KoPathPoint * point2 = pathShape->pointByIndex( m_pointData2.m_pointIndex );

    m_oldControlPoint1 = QPointF( pathShape->shapeToDocument( m_reverse & 1 ? point1->controlPoint1() : point1->controlPoint2() ) );
    m_oldControlPoint2 = QPointF( pathShape->shapeToDocument( m_reverse & 2 ? point2->controlPoint1() : point2->controlPoint2() ) );
    m_oldProperties1 = point1->properties();
    m_oldProperties2 = point2->properties();
}

KoSubpathJoinCommand::~KoSubpathJoinCommand()
{
}

void KoSubpathJoinCommand::redo()
{
    QUndoCommand::redo();
    KoPathShape * pathShape = m_pointData1.m_pathShape;

    bool closeSubpath = m_pointData1.m_pointIndex.first == m_pointData2.m_pointIndex.first; 

    KoPathPoint * point1 = pathShape->pointByIndex( m_pointData1.m_pointIndex );
    KoPathPoint * point2 = pathShape->pointByIndex( m_pointData2.m_pointIndex );

    // if the endpoint is has a control point create a contol point for the new segment to be 
    // at the symetric position to the exiting one
    if ( m_reverse & ReverseFirst || closeSubpath )
        if ( point1->activeControlPoint2() )
            point1->setControlPoint1( 2.0 * point1->point() - point1->controlPoint2() );
    else
        if ( point1->activeControlPoint1() )
            point1->setControlPoint2( 2.0 * point1->point() - point1->controlPoint1() );
    if ( m_reverse & ReverseSecond || closeSubpath )
        if ( point2->activeControlPoint1() )
            point2->setControlPoint2( 2.0 * point2->point() - point2->controlPoint1() );
    else
        if ( point2->activeControlPoint2() )
            point2->setControlPoint1( 2.0 * point2->point() - point2->controlPoint2() );

    if ( closeSubpath )
    {
        pathShape->closeSubpath( m_pointData1.m_pointIndex );
    }
    else
    {
        if ( m_reverse & ReverseFirst )
        {
            pathShape->reverseSubpath( m_pointData1.m_pointIndex.first );
        }
        if ( m_reverse & ReverseSecond )
        {
            pathShape->reverseSubpath( m_pointData2.m_pointIndex.first );
        }
        pathShape->moveSubpath( m_pointData2.m_pointIndex.first, m_pointData1.m_pointIndex.first + 1 );
        m_splitIndex = m_pointData1.m_pointIndex;
        m_splitIndex.second = pathShape->pointCountSubpath( m_pointData1.m_pointIndex.first ) - 1;
        pathShape->join( m_pointData1.m_pointIndex.first );
    }
    pathShape->normalize();
    pathShape->repaint();
}

void KoSubpathJoinCommand::undo()
{
    QUndoCommand::undo();
    KoPathShape * pathShape = m_pointData1.m_pathShape;
    pathShape->repaint();
    if ( m_pointData1.m_pointIndex.first == m_pointData2.m_pointIndex.first )
    {
        pathShape->openSubpath( m_pointData1.m_pointIndex );
    }
    else
    {
        pathShape->breakAfter( m_splitIndex );
        pathShape->moveSubpath( m_pointData1.m_pointIndex.first + 1, m_pointData2.m_pointIndex.first );

        if ( m_reverse & ReverseSecond )
        {
            pathShape->reverseSubpath( m_pointData2.m_pointIndex.first );
        }
        if ( m_reverse & ReverseFirst )
        {
            pathShape->reverseSubpath( m_pointData1.m_pointIndex.first );
        }
    }
    KoPathPoint * point1 = pathShape->pointByIndex( m_pointData1.m_pointIndex );
    KoPathPoint * point2 = pathShape->pointByIndex( m_pointData2.m_pointIndex );

    // restore the old end points
    if ( m_reverse & ReverseFirst )
        point1->setControlPoint1( pathShape->documentToShape( m_oldControlPoint1 ) );
    else
        point1->setControlPoint2( pathShape->documentToShape( m_oldControlPoint1 ) );

    point1->setProperties( m_oldProperties1 );

    if ( m_reverse & ReverseSecond )
        point2->setControlPoint1( pathShape->documentToShape( m_oldControlPoint2 ) );
    else
        point2->setControlPoint2( pathShape->documentToShape( m_oldControlPoint2 ) );

    point2->setProperties( m_oldProperties2 );

    pathShape->normalize();
    pathShape->repaint();
}

