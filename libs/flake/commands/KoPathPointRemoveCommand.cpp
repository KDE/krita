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

#include "KoPathPointRemoveCommand.h"
#include "KoSubpathRemoveCommand.h"
#include "KoShapeControllerBase.h"
#include "KoShapeController.h"
#include <klocale.h>

QUndoCommand * KoPathPointRemoveCommand::createCommand(
    const QList<KoPathPointData> & pointDataList,
    KoShapeController * shapeController,
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
        new KoPathPointRemoveCommand( pointsToDelete, cmd );
    }
    foreach ( const KoPathPointData & pd, subpathToDelete )
    {
        new KoSubpathRemoveCommand( pd.m_pathShape, pd.m_pointIndex.first, cmd );
    }
    if ( shapesToDelete.size() > 0 )
    {
        shapeController->removeShapes( shapesToDelete, cmd );
    }

    return cmd;
}

KoPathPointRemoveCommand::KoPathPointRemoveCommand(
    const QList<KoPathPointData> & pointDataList,
    QUndoCommand *parent )
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

KoPathPointRemoveCommand::~KoPathPointRemoveCommand()
{
    if ( m_deletePoints )
    {
        qDeleteAll( m_points );
    }
}

void KoPathPointRemoveCommand::redo()
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

void KoPathPointRemoveCommand::undo()
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
