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

#include "KoPathSegmentTypeCommand.h"
#include <klocale.h>

KoPathSegmentTypeCommand::KoPathSegmentTypeCommand( const QList<KoPathPointData> & pointDataList, SegmentType segmentType,
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

KoPathSegmentTypeCommand::~KoPathSegmentTypeCommand()
{
}

void KoPathSegmentTypeCommand::redo()
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

void KoPathSegmentTypeCommand::undo()
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

