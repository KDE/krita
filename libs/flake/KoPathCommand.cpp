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

KoPointPropertyCommand::KoPointPropertyCommand( KoPathPoint *point, KoPathPoint::KoPointProperties property, QUndoCommand *parent )
: KoPathBaseCommand( parent )
{
    m_shapes.insert( point->parent() );
    PointPropertyChangeset changeset;
    changeset.point = point;
    changeset.newProperty = property;
    changeset.oldProperty = point->properties();
    changeset.firstControlPoint = point->controlPoint1();
    changeset.secondControlPoint = point->controlPoint2();
    m_changesets.append( changeset );

    setText( i18n( "Set point properties" ) );
}

KoPointPropertyCommand::KoPointPropertyCommand( const QList<KoPathPoint*> &points, const QList<KoPathPoint::KoPointProperties> &properties, QUndoCommand *parent )
: KoPathBaseCommand( parent )
{
    Q_ASSERT(points.size() == properties.size());
    uint pointCount = points.size();
    for( uint i = 0; i < pointCount; ++i )
    {
        PointPropertyChangeset changeset;
        changeset.point = points[i];
        changeset.newProperty = properties[i];
        changeset.oldProperty = changeset.point->properties();
        changeset.firstControlPoint = changeset.point->controlPoint1();
        changeset.secondControlPoint = changeset.point->controlPoint2();
        m_changesets.append( changeset );
        m_shapes.insert( changeset.point->parent() );
    }

    setText( i18n( "Set point properties" ) );
}

void KoPointPropertyCommand::redo()
{
    repaint( false );

    uint pointCount = m_changesets.count();
    for( uint i = 0; i < pointCount; ++ i)
    {
        PointPropertyChangeset &changeset = m_changesets[i];
        KoPathPoint::KoPointProperties newProperties = changeset.newProperty;
        KoPathPoint *point = changeset.point;

        if( newProperties & KoPathPoint::IsSymmetric )
        {
            newProperties &= ~KoPathPoint::IsSmooth;
            point->setProperties( newProperties );

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
        }
        else if( newProperties & KoPathPoint::IsSmooth )
        {
            newProperties &= ~KoPathPoint::IsSymmetric;
            point->setProperties( newProperties );

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
        }
        else
        {
            newProperties &= ~KoPathPoint::IsSymmetric;
            newProperties &= ~KoPathPoint::IsSmooth;
            point->setProperties( newProperties );
        }
    }
    repaint( true );
}

void KoPointPropertyCommand::undo()
{
    repaint( false );

    uint pointCount = m_changesets.count();
    for( uint i = 0; i < pointCount; ++ i)
    {
        KoPathPoint *point = m_changesets[i].point;
        point->setProperties( m_changesets[i].oldProperty );
        point->setControlPoint1( m_changesets[i].firstControlPoint );
        point->setControlPoint2( m_changesets[i].secondControlPoint );
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

    QUndoCommand *cmd = new QUndoCommand( i18n( "Remove points" ) );
    
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

KoPointJoinCommand::KoPointJoinCommand( KoPathShape *shape, KoPathPoint *point1, KoPathPoint *point2, QUndoCommand *parent )
: KoPathBaseCommand( shape, parent )
, m_point1( point1 )
, m_point2( point2 )
, m_joined( false )
{
    setText( i18n( "Join points" ) );
}

void KoPointJoinCommand::redo()
{
    KoPathShape *shape = *m_shapes.begin();
    m_joined = shape->joinBetween( m_point1, m_point2 );
    shape->repaint();
}

void KoPointJoinCommand::undo()
{
    KoPathShape *shape = *m_shapes.begin();
    if( m_joined )
    {
        shape->breakAt( KoPathSegment( m_point1, m_point2 ) );
        shape->repaint();
    }
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
        KoPathPoint *point = it->m_pathShape->pointByIndex(  it->m_pointIndex );
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


KoSegmentTypeCommand::KoSegmentTypeCommand( KoPathShape *shape, const KoPathSegment &segment, bool changeToLine, QUndoCommand *parent )
: KoPathBaseCommand( shape, parent )
, m_changeToLine( changeToLine )
{
    if( segment.first && segment.second )
        m_segments.append( segment );

    setText( i18n( "Change segment type" ) );
}

KoSegmentTypeCommand::KoSegmentTypeCommand( KoPathShape *shape, const QList<KoPathSegment> &segments, bool changeToLine, QUndoCommand *parent )
: KoPathBaseCommand( shape, parent )
, m_changeToLine( changeToLine )
{
    foreach( KoPathSegment segment, segments )
    {
        if( segment.first && segment.second )
            m_segments.append( segment );
    }

    setText( i18n( "Change segment type" ) );
}

void KoSegmentTypeCommand::redo()
{
    repaint( false );

    m_oldPointData.clear();
    foreach( KoPathSegment s, m_segments )
    {
        m_oldPointData.insert( s.first, *s.first );
        m_oldPointData.insert( s.second, *s.second );
    }

    foreach( KoPathSegment s, m_segments )
    {
        if( m_changeToLine )
        {
            s.first->unsetProperty( KoPathPoint::HasControlPoint2 );
            s.second->unsetProperty( KoPathPoint::HasControlPoint1 );
        }
        else
        {
            // check if segment is already a curve
            if( s.first->properties() & KoPathPoint::HasControlPoint2 || s.second->properties() & KoPathPoint::HasControlPoint1 )
                continue;

            QPointF pointDiff = s.second->point() - s.first->point();
            s.first->setControlPoint2( s.first->point() + 0.3 * pointDiff );
            s.second->setControlPoint1( s.first->point() + 0.7 * pointDiff );
        }
    }

    QPointF offset = (*m_shapes.begin())->normalize();
    QMatrix matrix;
    matrix.translate( -offset.x(), -offset.y() );
    QMap<KoPathPoint*, KoPathPoint>::iterator it = m_oldPointData.begin();
    for(; it != m_oldPointData.end(); ++it )
        it.value().map( matrix );

    repaint( false );
}

void KoSegmentTypeCommand::undo()
{
    repaint( false );

    KoPathPoint defaultPoint( 0, QPointF(0,0) );
    foreach( KoPathSegment s, m_segments )
    {
        *s.first = m_oldPointData.value( s.first, defaultPoint );
        *s.second = m_oldPointData.value( s.second, defaultPoint );
    }

    repaint( true );
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
{
    m_shapes.append( shape );

    setText( i18n( "Convert to Path" ) );
}

KoParameterToPathCommand::KoParameterToPathCommand( const QList<KoParameterShape*> &shapes, QUndoCommand *parent )
: QUndoCommand( parent )
, m_shapes( shapes )
{
    setText( i18n( "Convert to Path" ) );
}

KoParameterToPathCommand::~KoParameterToPathCommand()
{
}

void KoParameterToPathCommand::redo()
{
    foreach( KoParameterShape *shape, m_shapes )
    {
        shape->setModified( true );
        // TODO use global handle sizes here when we have them
        shape->repaint( shape->outline().controlPointRect().adjusted( -5.0, -5.0, 5.0, 5.0 ) );
    }
}

void KoParameterToPathCommand::undo()
{
    foreach( KoParameterShape *shape, m_shapes )
    {
        shape->setModified( false );
        // TODO use global handle sizes here when we have them
        shape->repaint( shape->outline().controlPointRect().adjusted( -5.0, -5.0, 5.0, 5.0 ) );
    }
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
