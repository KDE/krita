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

#ifndef KOPATHPOINTINSERTCOMMAND_H
#define KOPATHPOINTINSERTCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QPointF>
#include "KoPathPoint.h"
#include "KoPathPointData.h"
#include "flake_export.h"

class KoPathPointInsertCommandPrivate;

/// The undo / redo command for inserting path points
class FLAKE_EXPORT KoPathPointInsertCommand : public QUndoCommand
{
public:
    /**
     * Command to insert path points.
     *
     * This splits the segments at the given position by inserting new points.
     * The De Casteljau algorithm is used for calculating the position of the new
     * points.
     *
     * @param pointDataList describing the segments to split
     * @param insertPosition the position to insert at [0..1]
     * @param parent the parent command used for macro commands
     */
    KoPathPointInsertCommand(const QList<KoPathPointData> &pointDataList, qreal insertPosition, QUndoCommand *parent = 0);
    virtual ~KoPathPointInsertCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

    /// Returns list of inserted points
    QList<KoPathPoint*> insertedPoints() const;

private:
    KoPathPointInsertCommandPrivate *d;
};

#endif // KOPATHPOINTINSERTCOMMAND_H
