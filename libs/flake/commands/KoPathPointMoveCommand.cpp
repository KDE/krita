/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoPathPointMoveCommand.h"
#include "KoPathPoint.h"
#include <klocale.h>

KoPathPointMoveCommand::KoPathPointMoveCommand(const KoPathShapePointMap &pointMap, const QPointF &offset, QUndoCommand *parent)
        : QUndoCommand(parent)
        , m_offset(offset)
        , m_undoCalled(true)
{
    setText(i18n("Move points"));

    // only store path point indeces not pointer to path points
    KoPathShapePointMap::const_iterator it(pointMap.begin());
    for (; it != pointMap.end(); ++it) {
        KoPathShape * path = it.key();
        foreach( KoPathPoint * p, it.value() )
            m_points[path].insert( path->pathPointIndex( p ) );
    }
}

void KoPathPointMoveCommand::redo()
{
    QUndoCommand::redo();
    if (! m_undoCalled)
        return;
    
    applyOffset( m_offset );
    
    m_undoCalled = false;
}

void KoPathPointMoveCommand::undo()
{
    QUndoCommand::undo();
    if (m_undoCalled)
        return;
    
    applyOffset( -1.0 * m_offset );

    m_undoCalled = true;
}

void KoPathPointMoveCommand::applyOffset( const QPointF &offset )
{
    QMap<KoPathShape*, QSet<KoPathPointIndex> >::iterator it(m_points.begin());
    for (; it != m_points.end(); ++it) {
        KoPathShape * path = it.key();
        // transform offset from document to shape coordinate system
        QPointF shapeOffset = path->documentToShape(offset) - path->documentToShape(QPointF());
        QMatrix matrix;
        matrix.translate(shapeOffset.x(), shapeOffset.y());
        
        // repaint old bounding rect
        path->update();
        foreach(const KoPathPointIndex index, it.value()) {
            KoPathPoint * p = path->pointByIndex(index);
            if ( p )
                p->map(matrix, true);
        }
        path->normalize();
        // repaint new bounding rect
        path->update();
    }
}
