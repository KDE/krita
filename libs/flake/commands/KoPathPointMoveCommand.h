/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOPATHPOINTMOVECOMMAND_H
#define KOPATHPOINTMOVECOMMAND_H

#include <QUndoCommand>
#include <QPointF>

#include "KoPathShape.h"
#include "KoPathPointData.h"

/// The undo / redo command for path point moving.
class KoPathPointMoveCommand : public QUndoCommand
{
public:
    /**
     * Command to move path point.
     * @param pointData the path points to move
     * @param offset the offset by which the point is moved in document coordinates
     * @param parent the parent command used for macro commands
     */
    KoPathPointMoveCommand(const QList<KoPathPointData> &pointData, const QPointF &offset, QUndoCommand *parent = 0);

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    void applyOffset( const QPointF &offset );
    
    QPointF m_offset;
    bool m_undoCalled; // this command stores diffs; so calling undo twice will give wrong results. Guard against that.
    QMap<KoPathShape*, QSet<KoPathPointIndex> > m_points;
};

#endif // KOPATHPOINTMOVECOMMAND_H
