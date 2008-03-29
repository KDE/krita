/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoPathToolSelection.h"
#include "KoPathTool.h"
#include <KoParameterShape.h>
#include <KoPathPoint.h>
#include <KoPathPointData.h>
#include <KoViewConverter.h>
#include <QtGui/QPainter>

KoPathToolSelection::KoPathToolSelection( KoPathTool * tool )
    : m_tool( tool )
{
}

KoPathToolSelection::~KoPathToolSelection() 
{
}

void KoPathToolSelection::paint( QPainter &painter, const KoViewConverter &converter )
{
    KoPathShapePointMap::iterator it( m_shapePointMap.begin() );
    for ( ; it != m_shapePointMap.end(); ++it )
    {
        painter.save();

        painter.setMatrix( it.key()->absoluteTransformation(&converter) * painter.matrix() );
        KoShape::applyConversion( painter, converter );

        QRectF handle = converter.viewToDocument( m_tool->handleRect( QPoint(0,0) ) );

        foreach( KoPathPoint *p, it.value() )
            p->paint( painter, handle.size(), KoPathPoint::All );

        painter.restore();
    }
}

void KoPathToolSelection::add( KoPathPoint * point, bool clear )
{
    bool allreadyIn = false;
    if ( clear )
    {
        if ( size() == 1 && m_selectedPoints.contains( point ) )
        {
            allreadyIn = true;
        }
        else
        {
            this->clear();
        }
    }
    else
    {
        allreadyIn = m_selectedPoints.contains( point );
    }

    if ( !allreadyIn )
    {
        m_selectedPoints.insert( point );
        KoPathShape * pathShape = point->parent();
        KoPathShapePointMap::iterator it( m_shapePointMap.find( pathShape ) );
        if ( it == m_shapePointMap.end() )
        {
            it = m_shapePointMap.insert( pathShape, QSet<KoPathPoint *>() );
        }
        it.value().insert( point );
        m_tool->repaint( point->boundingRect() );
        emit selectionChanged();
    }
}

void KoPathToolSelection::remove( KoPathPoint * point )
{
    if ( m_selectedPoints.remove( point ) )
    {
        KoPathShape * pathShape = point->parent();
        m_shapePointMap[pathShape].remove( point );
        if ( m_shapePointMap[pathShape].size() == 0 )
        {
            m_shapePointMap.remove( pathShape );
        }
        emit selectionChanged();
    }
    m_tool->repaint( point->boundingRect() );
}

void KoPathToolSelection::clear()
{
    repaint();
    m_selectedPoints.clear();
    m_shapePointMap.clear();
    emit selectionChanged();
}

void KoPathToolSelection::selectPoints( const QRectF &rect, bool clearSelection )
{
    if( clearSelection )
    {
        clear();
    }

    blockSignals( true );
    foreach(KoPathShape* shape, m_selectedShapes)
    {
        KoParameterShape *parameterShape = dynamic_cast<KoParameterShape*>( shape );
        if(parameterShape && parameterShape->isParametricShape() )
            continue;
        foreach( KoPathPoint* point, shape->pointsAt( shape->documentToShape( rect ) ))
            add( point, false );
    }
    blockSignals(false);
    emit selectionChanged();
}

int KoPathToolSelection::objectCount() const 
{
    return m_shapePointMap.size();
}

int KoPathToolSelection::size() const
{
    return m_selectedPoints.size();
}

bool KoPathToolSelection::contains( KoPathPoint * point )
{
    return m_selectedPoints.contains( point );
}

const QSet<KoPathPoint *> & KoPathToolSelection::selectedPoints() const
{
    return m_selectedPoints;
}

QList<KoPathPointData> KoPathToolSelection::selectedPointsData() const
{
    QList<KoPathPointData> pointData;
    foreach( KoPathPoint* p, m_selectedPoints )
    {
        KoPathShape * pathShape = p->parent();
        pointData.append( KoPathPointData( pathShape, pathShape->pathPointIndex( p ) ) );
    }
    return pointData;
}

QList<KoPathPointData> KoPathToolSelection::selectedSegmentsData() const
{
    QList<KoPathPointData> pointData;

    QList<KoPathPointData> pd( selectedPointsData() );
    qSort( pd );

    KoPathPointData last( 0, KoPathPointIndex( -1, -1 ) );
    KoPathPointData lastSubpathStart( 0, KoPathPointIndex( -1, -1 ) );

    QList<KoPathPointData>::const_iterator it( pd.begin() );
    for ( ; it != pd.end(); ++it )
    {
        if ( it->m_pointIndex.second == 0 )
            lastSubpathStart = *it;

        if ( last.m_pathShape == it->m_pathShape
             && last.m_pointIndex.first == it->m_pointIndex.first
             && last.m_pointIndex.second + 1 == it->m_pointIndex.second  )
        {
            pointData.append( last );
        }

        if ( lastSubpathStart.m_pathShape == it->m_pathShape
             && it->m_pathShape->pointByIndex( it->m_pointIndex )->properties() & KoPathPoint::CloseSubpath
             && (it->m_pathShape->pointByIndex( it->m_pointIndex )->properties() & KoPathPoint::StartSubpath) == 0 )
        {
            pointData.append( *it );
        }

        last = *it;
    }

    return pointData;
}

const KoPathShapePointMap & KoPathToolSelection::selectedPointMap() const
{
    return m_shapePointMap;
}

QList<KoPathShape*> KoPathToolSelection::selectedShapes() const
{
    return m_selectedShapes;
}

void KoPathToolSelection::setSelectedShapes( const QList<KoPathShape*> shapes )
{
    m_selectedShapes = shapes;
}

void KoPathToolSelection::repaint()
{
    foreach ( KoPathPoint *p, m_selectedPoints )
    {
        m_tool->repaint( p->boundingRect(false) );
    }
}

void KoPathToolSelection::update()
{
    KoPathShapePointMap::iterator it( m_shapePointMap.begin() );
    while ( it != m_shapePointMap.end() )
    {
        if ( ! m_selectedShapes.contains( it.key() ) )
        {
            it = m_shapePointMap.erase( it );
        }
        else
        {
            QSet<KoPathPoint *>::iterator pointIt( it.value().begin() );
            while ( pointIt != it.value().end() )
            {
                if ( ( *pointIt )->parent()->pathPointIndex( *pointIt ) == KoPathPointIndex( -1, -1 ) )
                {
                    pointIt = it.value().erase( pointIt );
                }
                else
                {
                    ++pointIt;
                }
            }
            ++it;
        }
    }
}

bool KoPathToolSelection::hasSelection()
{
    return m_selectedPoints.isEmpty();
}

#include "KoPathToolSelection.moc"
