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

#include "KoPathPointTypeCommand.h"
#include <klocale.h>
#include <math.h>

KoPathPointTypeCommand::KoPathPointTypeCommand(
    const QList<KoPathPointData> & pointDataList,
    PointType pointType,
    QUndoCommand *parent )
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
            pointData.m_oldControlPoint1 = it->m_pathShape->shapeToDocument( point->controlPoint1() );
            pointData.m_oldControlPoint2 = it->m_pathShape->shapeToDocument( point->controlPoint2() );
            pointData.m_oldProperties = point->properties();

            m_oldPointData.append( pointData );
            m_shapes.insert( it->m_pathShape );
        }
    }
    setText( i18n( "Set point type" ) );
}

KoPathPointTypeCommand::~KoPathPointTypeCommand()
{
}

void KoPathPointTypeCommand::redo()
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

void KoPathPointTypeCommand::undo()
{
    repaint( false );

    QList<PointData>::iterator it( m_oldPointData.begin() );
    for ( ; it != m_oldPointData.end(); ++it )
    {
        KoPathShape *pathShape = it->m_pointData.m_pathShape;
        KoPathPoint *point = pathShape->pointByIndex( it->m_pointData.m_pointIndex );;

        point->setProperties( it->m_oldProperties );
        point->setControlPoint1( pathShape->documentToShape( it->m_oldControlPoint1 ) );
        point->setControlPoint2( pathShape->documentToShape( it->m_oldControlPoint2 ) );
    }
    repaint( true );
}

