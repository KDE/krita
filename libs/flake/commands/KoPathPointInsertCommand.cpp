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

#include "KoPathPointInsertCommand.h"
#include <klocale.h>

KoPathPointInsertCommand::KoPathPointInsertCommand( const QList<KoPathPointData> & pointDataList, double insertPosition, QUndoCommand *parent )
: QUndoCommand( parent )
, m_deletePoints( true )
{
    if ( insertPosition < 0 )
        insertPosition = 0;
    if ( insertPosition > 1 )
        insertPosition = 1;

    //TODO the list needs to be sorted

    QList<KoPathPointData>::const_iterator it( pointDataList.begin() );
    for ( ; it != pointDataList.end(); ++it )
    {
        KoPathShape * pathShape = it->m_pathShape;

        KoPathSegment segment = pathShape->segmentByIndex( it->m_pointIndex );

        // should not happen but to be sure
        if ( !segment.first || !segment.second )
            continue;

        m_pointDataList.append( *it );
        if ( segment.first->activeControlPoint2() || segment.second->activeControlPoint1() )
        {
            QPointF q[4] =
            {
               segment.first->point(),
               segment.first->activeControlPoint2() ? segment.first->controlPoint2() : segment.first->point(),
               segment.second->activeControlPoint1() ? segment.second->controlPoint1() : segment.second->point(),
               segment.second->point()
            };

            QPointF p[3];
            // the De Casteljau algorithm.
            for( unsigned short j = 1; j <= 3; ++j )
            {
                for( unsigned short i = 0; i <= 3 - j; ++i )
                {
                    q[i] = ( 1.0 - insertPosition ) * q[i] + insertPosition * q[i + 1];
                }
                // modify the new segment.
                p[j - 1] = q[0];
            }
            KoPathPoint * splitPoint = new KoPathPoint( pathShape, p[2], KoPathPoint::CanHaveControlPoint1|KoPathPoint::CanHaveControlPoint2 );
            splitPoint->setControlPoint1( p[1] );
            splitPoint->setControlPoint2( q[1] );

            m_points.append( splitPoint );
            m_controlPoints.append( QPair<QPointF, QPointF>( p[0], q[2] ) );
        }
        else
        {
            QPointF splitPointPos = segment.first->point() + insertPosition * ( segment.second->point() - segment.first->point());
            m_points.append( new KoPathPoint( pathShape, splitPointPos, KoPathPoint::CanHaveControlPoint1|KoPathPoint::CanHaveControlPoint2 ) );
            m_controlPoints.append( QPair<QPointF, QPointF>( segment.first->controlPoint2(), segment.second->controlPoint1() ) );
        }
    }
}

KoPathPointInsertCommand::~KoPathPointInsertCommand()
{
    if ( m_deletePoints )
    {
        qDeleteAll( m_points );
    }
}

void KoPathPointInsertCommand::redo()
{
    for ( int i = m_pointDataList.size() - 1; i >= 0; --i )
    {
        KoPathPointData pointData = m_pointDataList.at( i );
        KoPathShape * pathShape = pointData.m_pathShape;

        KoPathSegment segment = pathShape->segmentByIndex( pointData.m_pointIndex );

        ++pointData.m_pointIndex.second;

        if ( segment.first->activeControlPoint2() )
        {
            QPointF controlPoint2 = segment.first->controlPoint2();
            qSwap( controlPoint2, m_controlPoints[i].first );
            segment.first->setControlPoint2( controlPoint2 );
        }

        if ( segment.second->activeControlPoint1() )
        {
            QPointF controlPoint1 = segment.second->controlPoint1();
            qSwap( controlPoint1, m_controlPoints[i].second );
            segment.second->setControlPoint1( controlPoint1 );
        }

        pathShape->insertPoint( m_points.at( i ), pointData.m_pointIndex );
        pathShape->repaint();
    }
    m_deletePoints = false;
}

void KoPathPointInsertCommand::undo()
{
    for ( int i = 0; i < m_pointDataList.size(); ++i )
    {
        const KoPathPointData &pdBefore = m_pointDataList.at( i );
        KoPathShape * pathShape = pdBefore.m_pathShape;
        KoPathPointIndex piAfter = pdBefore.m_pointIndex;
        ++piAfter.second;

        KoPathPoint * before = pathShape->pointByIndex( pdBefore.m_pointIndex );

        m_points[i] = pathShape->removePoint( piAfter );

        if ( m_points[i]->properties() & KoPathPoint::CloseSubpath )
        {
            piAfter.second = 0;
        }

        KoPathPoint * after = pathShape->pointByIndex( piAfter );

        if ( before->activeControlPoint2() )
        {
            QPointF controlPoint2 = before->controlPoint2();
            qSwap( controlPoint2, m_controlPoints[i].first );
            before->setControlPoint2( controlPoint2 );
        }

        if ( after->activeControlPoint1() )
        {
            QPointF controlPoint1 = after->controlPoint1();
            qSwap( controlPoint1, m_controlPoints[i].second );
            after->setControlPoint1( controlPoint1 );
        }
        pathShape->repaint();
    }
    m_deletePoints = true;
}

