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

KoPointMoveCommand::KoPointMoveCommand( KoPathShape *shape, KoPathPoint *point, const QPointF &offset, KoPathPoint::KoPointType pointType )
: KoPathBaseCommand( shape )
, m_offset( offset )
, m_pointType( pointType )
{
    m_points << point;
}

KoPointMoveCommand::KoPointMoveCommand( KoPathShape *shape, const QList<KoPathPoint*> &points, const QPointF &offset )
: KoPathBaseCommand( shape )
, m_points( points )
, m_offset( offset )
, m_pointType( KoPathPoint::Node )
{
}

void KoPointMoveCommand::execute()
{
    QRectF oldControlRect = m_shape->outline().controlPointRect();

    if( m_pointType == KoPathPoint::Node )
    {
        QMatrix matrix;
        matrix.translate( m_offset.x(), m_offset.y() );

        foreach( KoPathPoint *p, m_points )
        {
           p->map( matrix, true );
        }
    }
    else if( m_pointType == KoPathPoint::ControlPoint1 )
    {
        KoPathPoint *p = m_points.first();
        p->setControlPoint1( p->controlPoint1() + m_offset );
        if( p->properties() & KoPathPoint::IsSymmetric )
        {
            // set the other control point so that it lies on the line between the moved
            // control point and the point, with the same distance to the point as the moved point
            p->setControlPoint2( 2.0 * p->point() - p->controlPoint1() );
        }
        else if( p->properties() & KoPathPoint::IsSmooth )
        {
            // move the other control point so that it lies on the line through point and control point
            // keeping its distance to the point
            QPointF direction = p->point() - p->controlPoint1();
            direction /= sqrt( direction.x()*direction.x() + direction.y()*direction.y() );
            QPointF distance = p->point() - p->controlPoint2();
            qreal length = sqrt( distance.x()*distance.x() + distance.y()*distance.y() );
            p->setControlPoint2( p->point() + length * direction );
        }
    }
    else if( m_pointType == KoPathPoint::ControlPoint2 )
    {
        KoPathPoint *p = m_points.first();
        p->setControlPoint2( p->controlPoint2() + m_offset );
        if( p->properties() & KoPathPoint::IsSymmetric )
        {
            // set the other control point so that it lies on the line between the moved
            // control point and the point, with the same distance to the point as the moved point
            p->setControlPoint1( 2.0 * p->point() - p->controlPoint2() );
        }
        else if( p->properties() & KoPathPoint::IsSmooth )
        {
            // move the other control point so that it lies on the line through point and control point
            // keeping its distance to the point
            QPointF direction = p->point() - p->controlPoint2();
            direction /= sqrt( direction.x()*direction.x() + direction.y()*direction.y() );
            QPointF distance = p->point() - p->controlPoint1();
            qreal length = sqrt( distance.x()*distance.x() + distance.y()*distance.y() );
            p->setControlPoint1( p->point() + length * direction );
        }
    }

    repaint( oldControlRect );
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

KoPointRemoveCommand::KoPointRemoveCommand( KoPathShape *shape, KoPathPoint *point )
: KoPathBaseCommand( shape )
{
    m_data << KoPointRemoveData( point );
}

KoPointRemoveCommand::KoPointRemoveCommand( KoPathShape *shape, const QList<KoPathPoint*> &points )
: KoPathBaseCommand( shape )
{
    foreach( KoPathPoint* point, points )
        m_data << KoPointRemoveData( point );
}

void KoPointRemoveCommand::execute()
{
    QRectF oldControlRect = m_shape->outline().controlPointRect();

    for( int i = 0; i < m_data.size(); ++i )
    {
        KoPointRemoveData &data = m_data[i];
        QPair<KoSubpath*, int> pointdata = m_shape->removePoint( data.m_point );
        data.m_subpath = pointdata.first;
        data.m_position = pointdata.second;
    }

    QPointF offset = m_shape->normalize();
    QMatrix matrix;
    matrix.translate( -offset.x(), -offset.y() );
    for( int i = 0; i < m_data.size(); ++i )
        m_data[i].m_point->map( matrix );

    repaint( oldControlRect.translated( -offset ) );
}

void KoPointRemoveCommand::unexecute()
{
    QRectF oldControlRect = m_shape->outline().controlPointRect();
    // insert the points in inverse order
    for( int i = m_data.size()-1; i >= 0; --i )
    {
        KoPointRemoveData &data = m_data[i];
        m_shape->insertPoint( data.m_point, data.m_subpath, data.m_position );
    }
    repaint( oldControlRect );
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
