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

#include "KoPathCommand.h"
#include "KoParameterShape.h"
#include "KoShapeControllerBase.h"
#include "KoShapeController.h"
#include "KoShapeContainer.h"
#include <klocale.h>
#include <kdebug.h>
#include <math.h>

KoPathBaseCommand::KoPathBaseCommand( QUndoCommand *parent )
: QUndoCommand( parent )
{
}

KoPathBaseCommand::KoPathBaseCommand( KoPathShape *shape, QUndoCommand *parent )
: QUndoCommand( parent )
{
    m_shapes.insert( shape );
}

void KoPathBaseCommand::repaint( bool normalizeShapes )
{
    foreach( KoPathShape *shape, m_shapes )
    {
        if( normalizeShapes )
            shape->normalize();
        // TODO use the proper adjustment if the actual point size could be retrieved
        shape->repaint( shape->outline().controlPointRect().adjusted( -5.0, -5.0, 5.0, 5.0 ) );
    }
}

KoPointMoveCommand::KoPointMoveCommand( const KoPathShapePointMap &pointMap, const QPointF &offset, QUndoCommand *parent )
: QUndoCommand( parent )
, m_pointMap( pointMap )
, m_offset( offset )
{
    setText( i18n( "Move points" ) );
}

void KoPointMoveCommand::redo()
{
    KoPathShapePointMap::iterator it( m_pointMap.begin() );
    for ( ; it != m_pointMap.end(); ++it )
    {
        QPointF offset = it.key()->documentToShape( m_offset ) - it.key()->documentToShape( QPointF( 0, 0 ) );
        QMatrix matrix;
        matrix.translate( offset.x(), offset.y() );

        // repaint old bounding rect
        it.key()->repaint();
        foreach( KoPathPoint *p, it.value() )
        {
            p->map( matrix, true );
        }
        it.key()->normalize();
        // repaint new bounding rect
        it.key()->repaint();
    }
}

void KoPointMoveCommand::undo()
{
    m_offset *= -1.0;
    redo();
    m_offset *= -1.0;
}

KoControlPointMoveCommand::KoControlPointMoveCommand( const KoPathPointData &pointData, const QPointF &offset, 
                                                      KoPathPoint::KoPointType pointType, QUndoCommand *parent )
: QUndoCommand( parent )
, m_pointData( pointData )
, m_pointType( pointType )
{
    KoPathShape * pathShape = m_pointData.m_pathShape;
    KoPathPoint * point = pathShape->pointByIndex( m_pointData.m_pointIndex );
    if ( point )
    {
        m_offset = point->parent()->documentToShape( offset ) - point->parent()->documentToShape( QPointF( 0, 0 ) );
    }

    setText( i18n( "Move control point" ) );
}

void KoControlPointMoveCommand::redo()
{
    KoPathShape * pathShape = m_pointData.m_pathShape;
    KoPathPoint * point = pathShape->pointByIndex( m_pointData.m_pointIndex );
    if ( point )
    {
        pathShape->repaint();

        if ( m_pointType == KoPathPoint::ControlPoint1 )
        {
            point->setControlPoint1( point->controlPoint1() + m_offset );
            if( point->properties() & KoPathPoint::IsSymmetric )
            {
                // set the other control point so that it lies on the line between the moved
                // control point and the point, with the same distance to the point as the moved point
                point->setControlPoint2( 2.0 * point->point() - point->controlPoint1() );
            }
            else if( point->properties() & KoPathPoint::IsSmooth )
            {
                // move the other control point so that it lies on the line through point and control point
                // keeping its distance to the point
                QPointF direction = point->point() - point->controlPoint1();
                direction /= sqrt( direction.x()*direction.x() + direction.y()*direction.y() );
                QPointF distance = point->point() - point->controlPoint2();
                qreal length = sqrt( distance.x()*distance.x() + distance.y()*distance.y() );
                point->setControlPoint2( point->point() + length * direction );
            }
        }
        else if( m_pointType == KoPathPoint::ControlPoint2 )
        {
            point->setControlPoint2( point->controlPoint2() + m_offset );
            if( point->properties() & KoPathPoint::IsSymmetric )
            {
                // set the other control point so that it lies on the line between the moved
                // control point and the point, with the same distance to the point as the moved point
                point->setControlPoint1( 2.0 * point->point() - point->controlPoint2() );
            }
            else if( point->properties() & KoPathPoint::IsSmooth )
            {
                // move the other control point so that it lies on the line through point and control point
                // keeping its distance to the point
                QPointF direction = point->point() - point->controlPoint2();
                direction /= sqrt( direction.x()*direction.x() + direction.y()*direction.y() );
                QPointF distance = point->point() - point->controlPoint1();
                qreal length = sqrt( distance.x()*distance.x() + distance.y()*distance.y() );
                point->setControlPoint1( point->point() + length * direction );
            }
        }

        pathShape->normalize();
        pathShape->repaint();
    }
}

void KoControlPointMoveCommand::undo()
{
    m_offset *= -1.0;
    redo();
    m_offset *= -1.0;
}

KoPointTypeCommand::KoPointTypeCommand( const QList<KoPathPointData> & pointDataList, PointType pointType, QUndoCommand *parent )
: KoPathBaseCommand( parent )
, m_pointType( pointType )
{
    QList<KoPathPointData>::const_iterator it( pointDataList.begin() );
    for ( ; it != pointDataList.end(); ++it )
    {
        KoPathPoint *point = it->m_pathShape->pointByIndex( it->m_pointIndex );
        if ( point )
        {
            PointData pointData( *it );
            pointData.m_oldControlPoint1 = point->controlPoint1();
            pointData.m_oldControlPoint2 = point->controlPoint2();
            pointData.m_oldProperties = point->properties();

            m_oldPointData.append( pointData );
            m_shapes.insert( it->m_pathShape );
        }
    }
    setText( i18n( "Set point type" ) );
}

KoPointTypeCommand::~KoPointTypeCommand()
{
}

void KoPointTypeCommand::redo()
{
    repaint( false );

    QList<PointData>::iterator it( m_oldPointData.begin() );
    for ( ; it != m_oldPointData.end(); ++it )
    {
        KoPathPoint *point = it->m_pointData.m_pathShape->pointByIndex( it->m_pointData.m_pointIndex );;
        KoPathPoint::KoPointProperties properties = point->properties();

        switch ( m_pointType )
        {
            case Symmetric:
            {
                properties &= ~KoPathPoint::IsSmooth;
                properties |= KoPathPoint::IsSymmetric;
                
                // calculate vector from node point to first control point and normalize it
                QPointF directionC1 = point->controlPoint1() - point->point();
                qreal dirLengthC1 = sqrt( directionC1.x()*directionC1.x() + directionC1.y()*directionC1.y() );
                directionC1 /= dirLengthC1;
                // calculate vector from node point to second control point and normalize it
                QPointF directionC2 = point->controlPoint2() - point->point();
                qreal dirLengthC2 = sqrt( directionC2.x()*directionC2.x() + directionC2.y()*directionC2.y() );
                directionC2 /= dirLengthC2;
                // calculate the average distance of the control points to the node point
                qreal averageLength = 0.5 * (dirLengthC1 + dirLengthC2);
                // compute position of the control points so that they lie on a line going through the node point
                // the new distance of the control points is the average distance to the node point
                point->setControlPoint1( point->point() + 0.5 * averageLength * (directionC1 - directionC2) );
                point->setControlPoint2( point->point() + 0.5 * averageLength * (directionC2 - directionC1) );
            }   break;
            case Smooth:
            {
                properties &= ~KoPathPoint::IsSymmetric;
                properties |= KoPathPoint::IsSmooth;

                // calculate vector from node point to first control point and normalize it
                QPointF directionC1 = point->controlPoint1() - point->point();
                qreal dirLengthC1 = sqrt( directionC1.x()*directionC1.x() + directionC1.y()*directionC1.y() );
                directionC1 /= dirLengthC1;
                // calculate vector from node point to second control point and normalize it
                QPointF directionC2 = point->controlPoint2() - point->point();
                qreal dirLengthC2 = sqrt( directionC2.x()*directionC2.x() + directionC2.y()*directionC2.y() );
                directionC2 /= dirLengthC2;
                // compute position of the control points so that they lie on a line going through the node point
                // the new distance of the control points is the average distance to the node point
                point->setControlPoint1( point->point() + 0.5 * dirLengthC1 * (directionC1 - directionC2) );
                point->setControlPoint2( point->point() + 0.5 * dirLengthC2 * (directionC2 - directionC1) );
            }   break;
            case Corner:
            default:
                properties &= ~KoPathPoint::IsSymmetric;
                properties &= ~KoPathPoint::IsSmooth;
                break;
        }
        point->setProperties( properties );
    }
    repaint( true );
}

void KoPointTypeCommand::undo()
{
    repaint( false );

    QList<PointData>::iterator it( m_oldPointData.begin() );
    for ( ; it != m_oldPointData.end(); ++it )
    {
        KoPathPoint *point = it->m_pointData.m_pathShape->pointByIndex( it->m_pointData.m_pointIndex );;

        point->setProperties( it->m_oldProperties );
        point->setControlPoint1( it->m_oldControlPoint1 );
        point->setControlPoint2( it->m_oldControlPoint2 );
    }
    repaint( true );
}

QUndoCommand * KoPointRemoveCommand::createCommand( const QList<KoPathPointData> & pointDataList, KoShapeController * shapeController, 
                                                    QUndoCommand *parent )
{
    QList<KoPathPointData> sortedPointData( pointDataList );
    qSort( sortedPointData );

    KoPathPointData last( 0, KoPathPointIndex( -1, -1 ) );
    // add last at the end so that the point date before last will also be put in 
    // the right places.
    sortedPointData.append( last );

    QList<KoPathPointData> tmp;
    QList<KoPathPointData> tmpPoints;
    QList<KoPathPointData> tmpSubpaths;
    QList<KoPathPointData> pointsToDelete;
    QList<KoPathPointData> subpathToDelete;
    QList<KoShape*> shapesToDelete;

    int deletePointCount = 0;
    QList<KoPathPointData>::const_iterator it( sortedPointData.begin() );
    for ( ; it != sortedPointData.end(); ++it )
    {
        if ( last.m_pathShape != it->m_pathShape || last.m_pointIndex.first != it->m_pointIndex.first )
        {
            if ( last.m_pathShape->pointCountSubpath( last.m_pointIndex.first )  == tmp.size() )
            {
                tmpSubpaths.append( tmp.first() );
            }
            else
            {
                foreach ( KoPathPointData pd, tmp )
                {
                    tmpPoints.append( pd );
                }
            }
            deletePointCount += tmp.size();
            tmp.clear();
        }

        if ( last.m_pathShape != 0 && last.m_pathShape != it->m_pathShape )
        {
            if ( last.m_pathShape->pointCount() == deletePointCount )
            {
                shapesToDelete.append( last.m_pathShape );
            }
            else
            {
                foreach ( KoPathPointData pd, tmpSubpaths )
                {
                    subpathToDelete.append( pd );
                }
                foreach ( KoPathPointData pd, tmpPoints )
                {
                    pointsToDelete.append( pd );
                }
            }
            tmpSubpaths.clear();
            tmpPoints.clear();
            deletePointCount = 0;
        }
        last = *it;
        tmp.append( *it );
    }

    QUndoCommand *cmd = new QUndoCommand( i18n( "Remove points" ), parent );
    
    if ( pointsToDelete.size() > 0 )
    {
        new KoPointRemoveCommand( pointsToDelete, cmd );
    }
    foreach ( const KoPathPointData & pd, subpathToDelete )
    {
        new KoRemoveSubpathCommand( pd.m_pathShape, pd.m_pointIndex.first, cmd );
    }
    if ( shapesToDelete.size() > 0 )
    {
        shapeController->removeShapes( shapesToDelete, cmd );
    }

    return cmd;
}

KoPointRemoveCommand::KoPointRemoveCommand( const QList<KoPathPointData> & pointDataList, QUndoCommand *parent )
: QUndoCommand( parent )
, m_deletePoints( false )
{
    QList<KoPathPointData>::const_iterator it( pointDataList.begin() );
    for ( ; it != pointDataList.end(); ++it )
    {
        KoPathPoint *point = it->m_pathShape->pointByIndex( it->m_pointIndex );
        if ( point )
        {
            m_pointDataList.append( *it );
            m_points.append( 0 );
        }
    }
    qSort( m_pointDataList );
    setText( i18n( "Remove points" ) );
}

KoPointRemoveCommand::~KoPointRemoveCommand()
{
    if ( m_deletePoints )
    {
        qDeleteAll( m_points );
    }
}

void KoPointRemoveCommand::redo()
{
    KoPathShape * lastPathShape = 0;
    int updateBefore = m_pointDataList.size();
    for ( int i = m_pointDataList.size() - 1; i >= 0; --i )
    {
        const KoPathPointData &pd = m_pointDataList.at( i );
        pd.m_pathShape->repaint();
        m_points[i] = pd.m_pathShape->removePoint( pd.m_pointIndex );

        if ( lastPathShape != pd.m_pathShape )
        {
            if ( lastPathShape )
            {
                QPointF offset = lastPathShape->normalize();

                QMatrix matrix;
                matrix.translate( -offset.x(), -offset.y() );
                for ( int j = i + 1; j < updateBefore; ++j )
                {
                    m_points.at( j )->map( matrix );
                }
                lastPathShape->repaint();
                updateBefore = i + 1;
            }
            lastPathShape = pd.m_pathShape;
        }
    }

    if ( lastPathShape )
    {
        QPointF offset = lastPathShape->normalize();

        QMatrix matrix;
        matrix.translate( -offset.x(), -offset.y() );
        for ( int j = 0; j < updateBefore; ++j )
        {
            m_points.at( j )->map( matrix );
        }
        lastPathShape->repaint();
    }

    m_deletePoints = true;
}

void KoPointRemoveCommand::undo()
{
    KoPathShape * lastPathShape = 0;
    for ( int i = 0; i < m_pointDataList.size(); ++i )
    {
        const KoPathPointData &pd = m_pointDataList.at( i );
        if ( lastPathShape && lastPathShape != pd.m_pathShape )
        {
            lastPathShape->normalize();
            lastPathShape->repaint();
        }
        pd.m_pathShape->insertPoint( m_points[i], pd.m_pointIndex );
        lastPathShape = pd.m_pathShape;
    }
    if ( lastPathShape )
    {
        lastPathShape->normalize();
        lastPathShape->repaint();
    }
    m_deletePoints = false;
}

KoRemoveSubpathCommand::KoRemoveSubpathCommand( KoPathShape *pathShape, int subpathIndex, QUndoCommand *parent )
: QUndoCommand( parent )
, m_pathShape( pathShape )
, m_subpathIndex( subpathIndex )
, m_subpath( 0 )                                
{
    setText( i18n( "Remove subpath" ) );
}

KoRemoveSubpathCommand::~KoRemoveSubpathCommand()
{
    if ( m_subpath )
    {
        qDeleteAll( *m_subpath );
        delete m_subpath;
    }
}

void KoRemoveSubpathCommand::redo()
{
    m_pathShape->repaint();
    m_subpath = m_pathShape->removeSubpath( m_subpathIndex );
    if ( m_subpath )
    {
        QPointF offset = m_pathShape->normalize();

        QMatrix matrix;
        matrix.translate( -offset.x(), -offset.y() );
        foreach ( KoPathPoint *point, *m_subpath )
        {
            point->map( matrix );
        }
        m_pathShape->repaint();
    }
}

void KoRemoveSubpathCommand::undo()
{
    if ( m_subpath )
    {
        m_pathShape->addSubpath( m_subpath, m_subpathIndex );
        m_pathShape->normalize();
        m_pathShape->repaint();
        m_subpath = 0;
    }
}

KoSplitSegmentCommand::KoSplitSegmentCommand( const QList<KoPathPointData> & pointDataList, double splitPosition, QUndoCommand *parent )
: QUndoCommand( parent )
, m_deletePoints( true )
{
    if ( splitPosition < 0 )
        splitPosition = 0;
    if ( splitPosition > 1 )
        splitPosition = 1;

    //TODO the list needs to be sorted

    QList<KoPathPointData>::const_iterator it( pointDataList.begin() );
    for ( ; it != pointDataList.end(); ++it )
    {
        KoPathShape * pathShape = it->m_pathShape;

        KoPathPointIndex pi( it->m_pointIndex );
        KoPathPoint * before = pathShape->pointByIndex( pi );

        if ( before->properties() & KoPathPoint::CloseSubpath )
        {
            pi.second = 0;
        }
        else
        {
            ++pi.second;
        }
        KoPathPoint * after = pathShape->pointByIndex( pi );

        // should not happen but to be sure
        if ( !before || !after )
            continue;

        m_pointDataList.append( *it );
        if ( before->activeControlPoint2() || after->activeControlPoint1() )
        {
            QPointF q[4] =
            { 
               before->point(), 
               before->activeControlPoint2() ? before->controlPoint2() : before->point(), 
               after->activeControlPoint1() ? after->controlPoint1() : after->point(), 
               after->point()
            };

            QPointF p[3];
            // the De Casteljau algorithm.
            for( unsigned short j = 1; j <= 3; ++j )
            {
                for( unsigned short i = 0; i <= 3 - j; ++i )
                {
                    q[i] = ( 1.0 - splitPosition ) * q[i] + splitPosition * q[i + 1];
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
            QPointF splitPointPos = before->point() + splitPosition * ( after->point() - before->point());
            m_points.append( new KoPathPoint( pathShape, splitPointPos, KoPathPoint::CanHaveControlPoint1|KoPathPoint::CanHaveControlPoint2 ) );
            m_controlPoints.append( QPair<QPointF, QPointF>( before->controlPoint2(), after->controlPoint1() ) );
        }
    }
}

KoSplitSegmentCommand::~KoSplitSegmentCommand()
{
    if ( m_deletePoints )
    {
        qDeleteAll( m_points );
    }
}

void KoSplitSegmentCommand::redo()
{
    for ( int i = m_pointDataList.size() - 1; i >= 0; --i )
    {
        const KoPathPointData &pdBefore = m_pointDataList.at( i );
        KoPathShape * pathShape = pdBefore.m_pathShape;
        KoPathPointIndex piAfter = pdBefore.m_pointIndex;

        KoPathPoint * before = pathShape->pointByIndex( pdBefore.m_pointIndex );

        if ( before->properties() & KoPathPoint::CloseSubpath )
        {
            piAfter.second = 0;
        }
        else
        {
            ++piAfter.second;
        }
        KoPathPoint * after = pathShape->pointByIndex( piAfter );
        piAfter.second = pdBefore.m_pointIndex.second + 1;

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

        pathShape->insertPoint( m_points.at( i ), piAfter );
        pathShape->repaint();
    }
    m_deletePoints = false;
}

void KoSplitSegmentCommand::undo()
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

KoBreakSegmentCommand::KoBreakSegmentCommand( const KoPathPointData & pointData, QUndoCommand *parent )
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

KoBreakSegmentCommand::~KoBreakSegmentCommand()
{
}

void KoBreakSegmentCommand::redo()
{
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

void KoBreakSegmentCommand::undo()
{
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


/*
 * The algorithm to break a multiple open or closed subpaths is:
 * Subpath is closed
 * - open behind the last point in the subpath 
 * - go on like as described in Not closed 
 * Not closed 
 * - break from the back of the subpath
 */
KoBreakAtPointCommand::KoBreakAtPointCommand( const QList<KoPathPointData> & pointDataList, QUndoCommand *parent )
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

KoBreakAtPointCommand::~KoBreakAtPointCommand()
{
    if ( m_deletePoints )
    {
        qDeleteAll( m_points );
    }
}

void KoBreakAtPointCommand::redo()
{
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

void KoBreakAtPointCommand::undo()
{
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

KoSegmentTypeCommand::KoSegmentTypeCommand( const QList<KoPathPointData> & pointDataList, SegmentType segmentType, 
                                            QUndoCommand *parent )
: QUndoCommand( parent )
, m_segmentType( segmentType )    
{
    QList<KoPathPointData>::const_iterator it( pointDataList.begin() ); 
    for ( ; it != pointDataList.end(); ++it )
    {
        KoPathSegment segment = it->m_pathShape->segmentByIndex( it->m_pointIndex );
        if ( segment.first && segment.second )
        {
            if ( m_segmentType == Curve )
            {
                if ( segment.first->activeControlPoint2() || segment.second->activeControlPoint1() )
                    continue;
            }
            else 
            {
                if ( ! segment.first->activeControlPoint2() && ! segment.second->activeControlPoint1() )
                    continue;
            }

            m_pointDataList.append( *it );
            SegmentTypeData segmentData;

            KoPathShape * pathShape = segment.first->parent();

            if ( m_segmentType == Curve )
            {
                segmentData.m_controlPoint2 = pathShape->shapeToDocument( segment.first->controlPoint2() );
                segmentData.m_controlPoint1 = pathShape->shapeToDocument( segment.second->controlPoint1() );
            }
            segmentData.m_properties2 = segment.first->properties();
            segmentData.m_properties1 = segment.second->properties();
            m_segmentData.append( segmentData );
        }
    }

    if ( m_segmentType == Curve )
    {
        setText(  i18n(  "Change segments to curves" ) );
    }
    else
    {
        setText(  i18n(  "Change segments to lines" ) );
    }
}

KoSegmentTypeCommand::~KoSegmentTypeCommand()
{
}

void KoSegmentTypeCommand::redo()
{
    QList<KoPathPointData>::const_iterator it( m_pointDataList.begin() ); 
    for ( ; it != m_pointDataList.end(); ++it )
    {
        KoPathShape * pathShape = it->m_pathShape;
        pathShape->repaint();

        KoPathSegment segment = pathShape->segmentByIndex( it->m_pointIndex );

        if ( m_segmentType == Curve )
        {
            QPointF pointDiff = segment.second->point() - segment.first->point();
            segment.first->setControlPoint2( segment.first->point() + pointDiff / 3.0 );
            segment.second->setControlPoint1( segment.first->point() + pointDiff * 2.0 / 3.0 );
            segment.first->setProperties( segment.first->properties() | KoPathPoint::HasControlPoint2 );
            segment.second->setProperties( segment.second->properties() | KoPathPoint::HasControlPoint1 );
        }
        else
        {
            segment.first->setProperties( segment.first->properties() & ~KoPathPoint::HasControlPoint2 );
            segment.second->setProperties( segment.second->properties() & ~KoPathPoint::HasControlPoint1 );
        }

        pathShape->normalize();
        pathShape->repaint();
    }
}

void KoSegmentTypeCommand::undo()
{
    for ( int i = 0; i < m_pointDataList.size(); ++i )
    {
        const KoPathPointData & pd = m_pointDataList.at( i );
        pd.m_pathShape->repaint();
        KoPathSegment segment = pd.m_pathShape->segmentByIndex( pd.m_pointIndex );
        const SegmentTypeData segmentData( m_segmentData.at( i ) );
        if ( m_segmentType == Curve )
        {
            segment.first->setControlPoint2( pd.m_pathShape->documentToShape( segmentData.m_controlPoint2 ) );
            segment.second->setControlPoint1( pd.m_pathShape->documentToShape( segmentData.m_controlPoint1 ) );
        }
        segment.first->setProperties( segmentData.m_properties2 );
        segment.second->setProperties( segmentData.m_properties1 );

        pd.m_pathShape->normalize();
        pd.m_pathShape->repaint();
    }
}

KoPathCombineCommand::KoPathCombineCommand( KoShapeControllerBase *controller, const QList<KoPathShape*> &paths,
                                            QUndoCommand *parent )
: QUndoCommand( parent )
, m_controller( controller )
, m_paths( paths )
, m_combinedPath( 0 )
, m_isCombined( false )
{
    setText( i18n( "Combine paths" ) );
}

KoPathCombineCommand::~KoPathCombineCommand()
{
    if( m_isCombined && m_controller )
    {
        foreach( KoPathShape* path, m_paths )
            delete path;
    }
    else
        delete m_combinedPath;
}

void KoPathCombineCommand::redo()
{
    if( ! m_paths.size() )
        return;

    if( ! m_combinedPath )
    {
        m_combinedPath = new KoPathShape();
        KoShapeContainer * parent = m_paths.first()->parent();
        if(parent)
            parent->addChild(m_combinedPath);
        m_combinedPath->setBorder( m_paths.first()->border() );
        m_combinedPath->setShapeId( m_paths.first()->shapeId() );
        // combine the paths
        foreach( KoPathShape* path, m_paths )
            m_combinedPath->combine( path );
    }

    m_isCombined = true;

    if( m_controller )
    {
        foreach( KoPathShape* p, m_paths )
            m_controller->removeShape( p );

        m_controller->addShape( m_combinedPath );
    }
}

void KoPathCombineCommand::undo()
{
    if( ! m_paths.size() )
        return;

    m_isCombined = false;

    if( m_controller )
    {
        m_controller->removeShape( m_combinedPath );
        foreach( KoPathShape* p, m_paths )
        {
            m_controller->addShape( p );
        }
    }
}

KoParameterChangeCommand::KoParameterChangeCommand( KoParameterShape *shape, int handleId, const QPointF &startPoint, const QPointF &endPoint, QUndoCommand *parent )
: QUndoCommand( parent )
, m_shape( shape )
, m_handleId( handleId )    
, m_startPoint( startPoint )    
, m_endPoint( endPoint )    
{
    setText( i18n( "Change parameter" ) );
}

KoParameterChangeCommand::~KoParameterChangeCommand()
{
}

/// redo the command
void KoParameterChangeCommand::redo()
{
    m_shape->repaint();
    m_shape->moveHandle( m_handleId, m_endPoint );
    m_shape->repaint();
}

/// revert the actions done in redo
void KoParameterChangeCommand::undo()
{
    m_shape->repaint();
    m_shape->moveHandle( m_handleId, m_startPoint );
    m_shape->repaint();
}

KoParameterToPathCommand::KoParameterToPathCommand( KoParameterShape *shape, QUndoCommand *parent )
: QUndoCommand( parent )
, m_newPointsActive( false )    
{
    m_shapes.append( shape );

    setText( i18n( "Convert to Path" ) );
}

KoParameterToPathCommand::KoParameterToPathCommand( const QList<KoParameterShape*> &shapes, QUndoCommand *parent )
: QUndoCommand( parent )
, m_shapes( shapes )
, m_newPointsActive( false )    
{
    foreach( KoParameterShape *shape, m_shapes )
    {
        KoSubpathList subpaths = shape->m_subpaths;
        KoSubpathList newSubpaths;
        // make a deep copy of the subpaths
        KoSubpathList::const_iterator pathIt( subpaths.begin() );
        for (  ; pathIt != subpaths.end(); ++pathIt )
        {
            KoSubpath * newSubpath = new KoSubpath();
            newSubpaths.append( newSubpath );
            KoSubpath::const_iterator it(  (  *pathIt )->begin() );
            for (  ; it != (  *pathIt )->end(); ++it )
            {
                newSubpath->append( new KoPathPoint( **it ) );
            }
        }
        m_oldSubpaths.append( subpaths );
        m_newSubpaths.append( newSubpaths );
    }
    setText( i18n( "Convert to Path" ) );
}

KoParameterToPathCommand::~KoParameterToPathCommand()
{
    // clear the no longer needed points
    if ( m_newPointsActive )
    {
        QList<KoSubpathList>::iterator it( m_oldSubpaths.begin() );
        for ( ; it != m_oldSubpaths.end(); ++it )
        {
            KoSubpathList::iterator pathIt( it->begin() );
            for ( ; pathIt != it->end(); ++pathIt )
            {
                qDeleteAll( **pathIt );
            }
            qDeleteAll( *it );
        }
    }
    else
    {
        QList<KoSubpathList>::iterator it( m_newSubpaths.begin() );
        for ( ; it != m_newSubpaths.end(); ++it )
        {
            KoSubpathList::iterator pathIt( it->begin() );
            for ( ; pathIt != it->end(); ++pathIt )
            {
                qDeleteAll( **pathIt );
            }
            qDeleteAll( *it );
        }
    }
}

void KoParameterToPathCommand::redo()
{
    for ( int i = 0; i < m_shapes.size(); ++i )
    {
        KoParameterShape * parameterShape = m_shapes.at( i );
        parameterShape->setModified( true );
        parameterShape->m_subpaths = m_newSubpaths[i];
        parameterShape->repaint();
    }
    m_newPointsActive = true;
}

void KoParameterToPathCommand::undo()
{
    for ( int i = 0; i < m_shapes.size(); ++i )
    {
        KoParameterShape * parameterShape = m_shapes.at( i );
        parameterShape->setModified( false );
        parameterShape->m_subpaths = m_oldSubpaths[i];
        parameterShape->repaint();
    }
    m_newPointsActive = false;
}

KoPathSeparateCommand::KoPathSeparateCommand( KoShapeControllerBase *controller, const QList<KoPathShape*> &paths,
                                              QUndoCommand *parent )
: QUndoCommand( parent )
, m_controller( controller )
, m_paths( paths )
, m_isSeparated( false )
{
    setText( i18n( "Separate paths" ) );
}

KoPathSeparateCommand::~KoPathSeparateCommand()
{
    if( m_isSeparated && m_controller )
    {
        foreach( KoPathShape* p, m_paths )
            delete p;
    }
    else
    {
        foreach( KoPathShape* p, m_separatedPaths )
            delete p;
    }
}

void KoPathSeparateCommand::redo()
{
    if( m_separatedPaths.isEmpty() )
    {
        foreach( KoPathShape* p, m_paths )
        {
            QList<KoPathShape*> separatedPaths;
            if( p->separate( separatedPaths ) )
                m_separatedPaths << separatedPaths;
        }
    }

    m_isSeparated = true;

    if( m_controller )
    {
        foreach( KoPathShape* p, m_paths )
            m_controller->removeShape( p );
        foreach( KoPathShape *p, m_separatedPaths )
            m_controller->addShape( p );
    }
    foreach( KoPathShape* p, m_paths )
        p->repaint();
}

void KoPathSeparateCommand::undo()
{
    if( m_controller )
    {
        foreach( KoPathShape *p, m_separatedPaths )
            m_controller->removeShape( p );
        foreach( KoPathShape* p, m_paths )
            m_controller->addShape( p );
    }

    m_isSeparated = false;

    foreach( KoPathShape* p, m_paths )
        p->repaint();
}
