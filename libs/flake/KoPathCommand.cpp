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

#include "KoPathCommand.h"
#include "KoParameterShape.h"
#include "KoShapeControllerBase.h"
#include <klocale.h>
#include <kdebug.h>
#include <math.h>

KoPathBaseCommand::KoPathBaseCommand( KoPathShape *shape )
: m_shape( shape )
{
}

void KoPathBaseCommand::repaint( const QRectF &oldControlPointRect )
{
    // the bounding rect has changed -> normalize
    QPointF offset = m_shape->normalize();

    // adjust the old control rect as the repainting is relative to the new shape position
    QRectF repaintRect = oldControlPointRect.translated( -offset ).unite( m_shape->outline().controlPointRect() );
    // TODO use the proper adjustment if the actual point size could be retrieved
    repaintRect.adjust( -5.0, -5.0, 5.0, 5.0 );
    m_shape->repaint( repaintRect );
}

KoPointMoveCommand::KoPointMoveCommand( const KoPathShapePointMap &pointMap, const QPointF &offset )
: m_pointMap( pointMap )
, m_offset( offset )
{
}

void KoPointMoveCommand::execute()
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

void KoPointMoveCommand::unexecute()
{
    m_offset *= -1.0;
    execute();
    m_offset *= -1.0;
}

QString KoPointMoveCommand::name() const
{
    return i18n( "Move points" );
}

KoControlPointMoveCommand::KoControlPointMoveCommand( KoPathPoint *point, const QPointF &offset, KoPathPoint::KoPointType pointType )
: m_point( point )
, m_offset( point->parent()->documentToShape( offset ) - point->parent()->documentToShape( QPointF( 0, 0 ) ) )
, m_pointType( pointType )
{
}

void KoControlPointMoveCommand::execute()
{
    KoPathShape * pathShape = m_point->parent();
    pathShape->repaint();

    if ( m_pointType == KoPathPoint::ControlPoint1 )
    {
        m_point->setControlPoint1( m_point->controlPoint1() + m_offset );
        if( m_point->properties() & KoPathPoint::IsSymmetric )
        {
            // set the other control point so that it lies on the line between the moved
            // control point and the point, with the same distance to the point as the moved point
            m_point->setControlPoint2( 2.0 * m_point->point() - m_point->controlPoint1() );
        }
        else if( m_point->properties() & KoPathPoint::IsSmooth )
        {
            // move the other control point so that it lies on the line through point and control point
            // keeping its distance to the point
            QPointF direction = m_point->point() - m_point->controlPoint1();
            direction /= sqrt( direction.x()*direction.x() + direction.y()*direction.y() );
            QPointF distance = m_point->point() - m_point->controlPoint2();
            qreal length = sqrt( distance.x()*distance.x() + distance.y()*distance.y() );
            m_point->setControlPoint2( m_point->point() + length * direction );
        }
    }
    else if( m_pointType == KoPathPoint::ControlPoint2 )
    {
        m_point->setControlPoint2( m_point->controlPoint2() + m_offset );
        if( m_point->properties() & KoPathPoint::IsSymmetric )
        {
            // set the other control point so that it lies on the line between the moved
            // control point and the point, with the same distance to the point as the moved point
            m_point->setControlPoint1( 2.0 * m_point->point() - m_point->controlPoint2() );
        }
        else if( m_point->properties() & KoPathPoint::IsSmooth )
        {
            // move the other control point so that it lies on the line through point and control point
            // keeping its distance to the point
            QPointF direction = m_point->point() - m_point->controlPoint2();
            direction /= sqrt( direction.x()*direction.x() + direction.y()*direction.y() );
            QPointF distance = m_point->point() - m_point->controlPoint1();
            qreal length = sqrt( distance.x()*distance.x() + distance.y()*distance.y() );
            m_point->setControlPoint1( m_point->point() + length * direction );
        }
    }

    pathShape->normalize();
    pathShape->repaint();
}

void KoControlPointMoveCommand::unexecute()
{
    m_offset *= -1.0;
    execute();
    m_offset *= -1.0;
}

QString KoControlPointMoveCommand::name() const
{
    return i18n( "Move control point" );
}

KoPointPropertyCommand::KoPointPropertyCommand( KoPathShape *shape, KoPathPoint *point, KoPathPoint::KoPointProperties property )
: KoPathBaseCommand( shape )
, m_point( point )
, m_newProperties( property )
{
    m_oldProperties = point->properties();
    m_controlPoint1 = point->controlPoint1();
    m_controlPoint2 = point->controlPoint2();
}

void KoPointPropertyCommand::execute()
{
    QRectF oldControlRect = m_shape->outline().controlPointRect();

    if( m_newProperties & KoPathPoint::IsSymmetric )
    {
        m_newProperties &= ~KoPathPoint::IsSmooth;
        m_point->setProperties( m_newProperties );

        // First calculate the direction vector of both control points starting from the point and their
        // distance to the point. Then calculate the average distance and move points so that
        // they have the same (average) distance from the point but keeping their direction.
        QPointF directionC1 = m_point->controlPoint1() - m_point->point();
        qreal dirLengthC1 = sqrt( directionC1.x()*directionC1.x() + directionC1.y()*directionC1.y() );
        QPointF directionC2 = m_point->controlPoint2() - m_point->point();
        qreal dirLengthC2 = sqrt( directionC2.x()*directionC2.x() + directionC2.y()*directionC2.y() );
        qreal averageLength = 0.5 * (dirLengthC1 + dirLengthC2);
        m_point->setControlPoint1( m_point->point() + averageLength / dirLengthC1 * directionC1 );
        m_point->setControlPoint2( m_point->point() + averageLength / dirLengthC2 * directionC2 );
        repaint( oldControlRect );
    }
    else if( m_newProperties & KoPathPoint::IsSmooth )
    {
        m_newProperties &= ~KoPathPoint::IsSymmetric;
        m_point->setProperties( m_newProperties );

        // First calculate the direction vector of both control points starting from the point and their
        // distance to the point. Then calculate the normalized direction vector. Then for each control
        // point calculate the bisecting line between its nromalized direction vector and the negated
        // normalied direction vector of the other points. Then use the result as the new direction
        // vector for the control point and their old distance to the point.
        QPointF directionC1 = m_point->controlPoint1() - m_point->point();
        qreal dirLengthC1 = sqrt( directionC1.x()*directionC1.x() + directionC1.y()*directionC1.y() );
        directionC1 /= dirLengthC1;
        QPointF directionC2 = m_point->controlPoint2() - m_point->point();
        qreal dirLengthC2 = sqrt( directionC2.x()*directionC2.x() + directionC2.y()*directionC2.y() );
        directionC2 /= dirLengthC2;
        m_point->setControlPoint1( m_point->point() + 0.5 * dirLengthC1 * (directionC1 - directionC2) );
        m_point->setControlPoint2( m_point->point() + 0.5 * dirLengthC2 * (directionC2 - directionC1) );
        repaint( oldControlRect );
    }
    else
    {
        m_newProperties &= ~KoPathPoint::IsSymmetric;
        m_newProperties &= ~KoPathPoint::IsSmooth;
        m_point->setProperties( m_newProperties );
    }
}

void KoPointPropertyCommand::unexecute()
{
    QRectF oldControlRect = m_shape->outline().controlPointRect();

    m_point->setProperties( m_oldProperties );
    m_point->setControlPoint1( m_controlPoint1 );
    m_point->setControlPoint2( m_controlPoint2 );

    repaint( oldControlRect );
}

QString KoPointPropertyCommand::name() const
{
    return i18n( "Set point properties" );
}

KoPointRemoveCommand::KoPointRemoveCommand( const KoPathShapePointMap &pointMap )
: m_pointMap( pointMap )
{
}

void KoPointRemoveCommand::execute()
{
    KoPathShapePointMap::iterator it( m_pointMap.begin() );
    m_data.clear();
    for ( ; it != m_pointMap.end(); ++it )
    {
        it.key()->repaint();

        foreach( KoPathPoint *p, it.value() )
        {
            QPair<KoSubpath*, int> pointdata = it.key()->removePoint( p );
            m_data.push_back( KoPointRemoveData( p, pointdata.first, pointdata.second ) );
        }

        QPointF offset = it.key()->normalize();

        QMatrix matrix;
        matrix.translate( -offset.x(), -offset.y() );
        foreach( KoPathPoint *p, it.value() )
        {
            p->map( matrix );
        }
        // repaint new bounding rect
        // TODO is this really needed
        it.key()->repaint();
    }
}

void KoPointRemoveCommand::unexecute()
{
    // insert the points in inverse order
    KoPathShape * pathShape = 0;
    for( int i = m_data.size()-1; i >= 0; --i )
    {
        KoPointRemoveData &data = m_data[i];
        if ( pathShape && pathShape != data.m_point->parent() )
        {
            pathShape->normalize();
            pathShape->repaint();
        }
        pathShape = data.m_point->parent();
        pathShape->insertPoint( data.m_point, data.m_subpath, data.m_position );
    }
    if ( pathShape )
    {
        pathShape->normalize();
        pathShape->repaint();
    }
}

QString KoPointRemoveCommand::name() const
{
    return i18n( "Remove point" );
}

KoSegmentSplitCommand::KoSegmentSplitCommand( KoPathShape *shape, const KoPathSegment &segment, double splitPosition )
: KoPathBaseCommand( shape )
, m_deletePoint( false )
{
    if( segment.first && segment.second )
    {
        m_segments << segment;
        m_oldNeighbors << qMakePair( *segment.first, *segment.second );
        m_newNeighbors << qMakePair( *segment.first, *segment.second );
        m_splitPoints << 0;
        m_splitPointPos << qMakePair( (KoSubpath*)0, 0 );
        m_splitPos << splitPosition;
    }
}

KoSegmentSplitCommand::KoSegmentSplitCommand( KoPathShape *shape, const QList<KoPathSegment> &segments, const QList<double> &splitPositions )
: KoPathBaseCommand( shape )
, m_deletePoint( false )
{
    Q_ASSERT(segments.size() == splitPositions.size());
    // check all the segments
    for( int i = 0; i < segments.size(); ++i )
    {
        const KoPathSegment &segment = segments[i];
        if( segment.first && segment.second )
        {
            m_segments << segment;
            m_oldNeighbors << qMakePair( *segment.first, *segment.second );
            m_newNeighbors << qMakePair( *segment.first, *segment.second );
            m_splitPoints << 0;
            m_splitPointPos << qMakePair( (KoSubpath*)0, 0 );
            m_splitPos << splitPositions[i];
        }
    }
}

KoSegmentSplitCommand::KoSegmentSplitCommand( KoPathShape *shape, const QList<KoPathSegment> &segments, double splitPosition )
: KoPathBaseCommand( shape )
, m_deletePoint( false )
{
    // check all the segments
    for( int i = 0; i < segments.size(); ++i )
    {
        const KoPathSegment &segment = segments[i];
        if( segment.first && segment.second )
        {
            m_segments << segment;
            m_oldNeighbors << qMakePair( *segment.first, *segment.second );
            m_newNeighbors << qMakePair( *segment.first, *segment.second );
            m_splitPoints << 0;
            m_splitPointPos << qMakePair( (KoSubpath*)0, 0 );
            m_splitPos << splitPosition;
        }
    }
}

KoSegmentSplitCommand::~KoSegmentSplitCommand()
{
    if( m_deletePoint )
    {
        foreach( KoPathPoint* p, m_splitPoints )
            delete p;
    }
}

void KoSegmentSplitCommand::execute()
{
    QRectF oldControlRect = m_shape->outline().controlPointRect();

    m_deletePoint = false;

    for( int i = 0; i < m_segments.size(); ++i )
    {
        KoPathSegment &segment = m_segments[i];
        KoPathPoint *splitPoint = m_splitPoints[i];
        if( ! splitPoint )
        {
            m_splitPoints[i] = m_shape->splitAt( segment, m_splitPos[i] );
            m_newNeighbors[i] = qMakePair( *segment.first, *segment.second );
        }
        else
        {
            m_shape->insertPoint( splitPoint, m_splitPointPos[i].first, m_splitPointPos[i].second );
            *segment.first = m_newNeighbors[i].first;
            *segment.second = m_newNeighbors[i].second;
        }
    }
    repaint( oldControlRect );
}

void KoSegmentSplitCommand::unexecute()
{
    QRectF oldControlRect = m_shape->outline().controlPointRect();

    m_deletePoint = true;

    for( int i = m_segments.size()-1; i >= 0; --i )
    {
        KoPathSegment &segment = m_segments[i];
        m_splitPointPos[i] = m_shape->removePoint( m_splitPoints[i] );
        *segment.first = m_oldNeighbors[i].first;
        *segment.second = m_oldNeighbors[i].second;
    }
    repaint( oldControlRect );
}

QString KoSegmentSplitCommand::name() const
{
    return i18n( "Split segment" );
}

KoPointJoinCommand::KoPointJoinCommand( KoPathShape *shape, KoPathPoint *point1, KoPathPoint *point2 )
: KoPathBaseCommand( shape )
, m_point1( point1 )
, m_point2( point2 )
, m_joined( false )
{
}

void KoPointJoinCommand::execute()
{
    m_joined = m_shape->joinBetween( m_point1, m_point2 );
    m_shape->repaint();
}

void KoPointJoinCommand::unexecute()
{
    if( m_joined )
    {
        m_shape->breakAt( KoPathSegment( m_point1, m_point2 ) );
        m_shape->repaint();
    }
}

QString KoPointJoinCommand::name() const
{
    return i18n( "Join points" );
}

KoSubpathBreakCommand::KoSubpathBreakCommand( KoPathShape *shape, KoPathPoint *breakPoint )
: KoPathBaseCommand( shape )
, m_breakPoint( breakPoint )
, m_segment( 0, 0 )
, m_breakSegment( false )
, m_broken( false )
, m_newPoint( 0 )
, m_pointData1( 0, QPointF(0,0) )
, m_pointData2( 0, QPointF(0,0) )
{
    if( breakPoint )
        m_pointData1 = *breakPoint;
    KoPathPoint *nextPoint = m_shape->nextPoint( m_breakPoint );
    if( nextPoint )
        m_pointData2 = *nextPoint;
}

KoSubpathBreakCommand::KoSubpathBreakCommand( KoPathShape *shape, const KoPathSegment &segment )
: KoPathBaseCommand( shape )
, m_breakPoint( 0 )
, m_segment( segment )
, m_breakSegment( true )
, m_broken( false )
, m_newPoint( 0 )
, m_pointData1( 0, QPointF(0,0) )
, m_pointData2( 0, QPointF(0,0) )
{
    if( m_segment.first )
        m_pointData1 = *m_segment.first;
    if( m_segment.second )
        m_pointData2 = *m_segment.second;
}

KoSubpathBreakCommand::~KoSubpathBreakCommand()
{
}

void KoSubpathBreakCommand::execute()
{
    if( m_breakSegment )
    {
        if( m_segment.first && m_segment.second )
        {
            m_broken = m_shape->breakAt( m_segment );
            m_shape->repaint();
        }
    }
    else
    {
        if( m_breakPoint )
        {
            m_broken = m_shape->breakAt( m_breakPoint, m_newPoint );
            m_shape->repaint();
        }
    }
}

void KoSubpathBreakCommand::unexecute()
{
    if( ! m_broken )
        return;

    if( m_breakSegment )
    {
        m_shape->joinBetween( m_segment.first, m_segment.second );
        *m_segment.first = m_pointData1;
        *m_segment.second = m_pointData2;
    }
    else
    {
        KoPathPoint *nextPoint = m_shape->nextPoint( m_newPoint );
        m_shape->removePoint( m_newPoint );
        delete m_newPoint;
        m_newPoint = 0;
        if( m_shape->joinBetween( m_breakPoint, nextPoint ) )
        {
            *m_breakPoint = m_pointData1;
            *nextPoint = m_pointData2;
        }
    }
    m_shape->repaint();
}

QString KoSubpathBreakCommand::name() const
{
    return i18n( "Break subpath" );
}

KoSegmentTypeCommand::KoSegmentTypeCommand( KoPathShape *shape, const KoPathSegment &segment, bool changeToLine )
: KoPathBaseCommand( shape )
, m_changeToLine( changeToLine )
{
    if( segment.first && segment.second )
        m_segments.append( segment );
}

KoSegmentTypeCommand::KoSegmentTypeCommand( KoPathShape *shape, const QList<KoPathSegment> &segments, bool changeToLine )
: KoPathBaseCommand( shape )
, m_changeToLine( changeToLine )
{
    foreach( KoPathSegment segment, segments )
    {
        if( segment.first && segment.second )
            m_segments.append( segment );
    }
}

void KoSegmentTypeCommand::execute()
{
    QRectF oldControlRect = m_shape->outline().controlPointRect();

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

    QPointF offset = m_shape->normalize();
    QMatrix matrix;
    matrix.translate( -offset.x(), -offset.y() );
    QMap<KoPathPoint*, KoPathPoint>::iterator it = m_oldPointData.begin();
    for(; it != m_oldPointData.end(); ++it )
        it.value().map( matrix );

    repaint( oldControlRect.translated( -offset ) );
}

void KoSegmentTypeCommand::unexecute()
{
    QRectF oldControlRect = m_shape->outline().controlPointRect();

    KoPathPoint defaultPoint( 0, QPointF(0,0) );
    foreach( KoPathSegment s, m_segments )
    {
        *s.first = m_oldPointData.value( s.first, defaultPoint );
        *s.second = m_oldPointData.value( s.second, defaultPoint );
    }

    repaint( oldControlRect );
}

QString KoSegmentTypeCommand::name() const
{
    return i18n( "Change segment type" );
}

KoPathCombineCommand::KoPathCombineCommand( KoShapeControllerBase *controller, const QList<KoPathShape*> &paths )
: m_controller( controller )
, m_paths( paths )
, m_combinedPath( 0 )
, m_isCombined( false )
{
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

void KoPathCombineCommand::execute()
{
    if( ! m_paths.size() )
        return;

    if( ! m_combinedPath )
    {
        m_combinedPath = new KoPathShape();
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

void KoPathCombineCommand::unexecute()
{
    if( ! m_paths.size() )
        return;

    m_isCombined = false;

    if( m_controller )
    {
        m_controller->removeShape( m_combinedPath );
        foreach( KoPathShape* p, m_paths )
            m_controller->addShape( p );
    }
}

QString KoPathCombineCommand::name() const
{
    return i18n( "Combine paths" );
}

KoParameterChangeCommand::KoParameterChangeCommand( KoParameterShape *shape, int handleId, const QPointF &startPoint, const QPointF &endPoint )
: m_shape( shape )
, m_handleId( handleId )    
, m_startPoint( startPoint )    
, m_endPoint( endPoint )    
{
}

KoParameterChangeCommand::~KoParameterChangeCommand()
{
}

/// execute the command
void KoParameterChangeCommand::execute()
{
    m_shape->moveHandle( m_handleId, m_endPoint );
    m_shape->repaint();
}

/// revert the actions done in execute
void KoParameterChangeCommand::unexecute()
{
    m_shape->moveHandle( m_handleId, m_startPoint );
    m_shape->repaint();
}

/// return the name of this command
QString KoParameterChangeCommand::name() const
{
    return i18n( "Change parameter" );
}

KoParameterToPathCommand::KoParameterToPathCommand( KoParameterShape *shape )
: m_shape( shape )    
{
}

KoParameterToPathCommand::~KoParameterToPathCommand()
{
}

void KoParameterToPathCommand::execute()
{
    m_shape->setModified( true );
}

void KoParameterToPathCommand::unexecute()
{
    m_shape->setModified( false );
}

QString KoParameterToPathCommand::name() const
{
    return i18n( "Modify path" );
}

KoPathSeparateCommand::KoPathSeparateCommand( KoShapeControllerBase *controller, const QList<KoPathShape*> &paths )
: m_controller( controller )
, m_paths( paths )
, m_isSeparated( false )
{
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

void KoPathSeparateCommand::execute()
{
    if( ! m_separatedPaths.size() )
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

void KoPathSeparateCommand::unexecute()
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

QString KoPathSeparateCommand::name() const
{
    return i18n( "Separate paths" );
}
