/* This file is part of the KDE project
 * Copyright (C) 2006,2008-2009 Jan Hambrecht <jaham@gmx.net>
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

KoPathPointMoveCommand::KoPathPointMoveCommand(const QList<KoPathPointData> &pointData, const QPointF &offset, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_undoCalled(true)
{
    setText(i18n("Move points"));

    foreach( const KoPathPointData &data, pointData ) {
        if (!m_points.contains(data)) {
            m_points[data] = offset;
            m_paths.insert(data.pathShape);
        }
    }
}

KoPathPointMoveCommand::KoPathPointMoveCommand(const QList<KoPathPointData> &pointData, const QList<QPointF> &offsets, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_undoCalled(true)
{
    Q_ASSERT(pointData.count() == offsets.count());

    setText(i18n("Move points"));

    uint dataCount = pointData.count();
    for (uint i = 0; i < dataCount; ++i) {
        const KoPathPointData & data = pointData[i];
        if (!m_points.contains(data)) {
            m_points[data] = offsets[i];
            m_paths.insert(data.pathShape);
        }
    }
}

void KoPathPointMoveCommand::redo()
{
    QUndoCommand::redo();
    if (! m_undoCalled)
        return;

    applyOffset( 1.0 );

    m_undoCalled = false;
}

void KoPathPointMoveCommand::undo()
{
    QUndoCommand::undo();
    if (m_undoCalled)
        return;

    applyOffset( -1.0 );

    m_undoCalled = true;
}

void KoPathPointMoveCommand::applyOffset( qreal factor )
{
    foreach(KoPathShape * path, m_paths) {
        // repaint old bounding rect
        path->update();
    }

    QMap<KoPathPointData, QPointF>::iterator it(m_points.begin());
    for (; it != m_points.end(); ++it) {
        KoPathShape * path = it.key().pathShape;
        // transform offset from document to shape coordinate system
        QPointF shapeOffset = path->documentToShape(factor*it.value()) - path->documentToShape(QPointF());
        QMatrix matrix;
        matrix.translate(shapeOffset.x(), shapeOffset.y());

        KoPathPoint * p = path->pointByIndex(it.key().pointIndex);
        if ( p )
            p->map(matrix, true);
    }

    foreach(KoPathShape * path, m_paths) {
        path->normalize();
        // repaint new bounding rect
        path->update();
    }
}
