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

#ifndef KOPATHBREAKATPOINTCOMMAND_H
#define KOPATHBREAKATPOINTCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include "KoPathPoint.h"
#include "KoPathPointData.h"

/// Command to break a subpath at points.
class KoPathBreakAtPointCommand : public QUndoCommand
{
public:
    /**
     * Command to break a subpath at points.
     *
     * The paths are broken at the given points. New points will be inserted after
     * the given points and then the paths will be split after the given points.
     *
     * @param pointDataList List of point data where the path should be split.
     * @param parent the parent command used for macro commands
     */
    explicit KoPathBreakAtPointCommand(const QList<KoPathPointData> &pointDataList, QUndoCommand *parent = 0);
    ~KoPathBreakAtPointCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    QList<KoPathPointData> m_pointDataList;
    QList<KoPathPoint*> m_points;
    // used for storing where to open the subpath. In case it not used for the open
    // status use .second to the store offset caused by a open of a subpath.
    QList<KoPathPointIndex> m_closedIndex;
    bool m_deletePoints;
};

#endif // KOPATHBREAKATPOINTCOMMAND_H
