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

#ifndef KOPATHPOINTREMOVECOMMAND_H
#define KOPATHPOINTREMOVECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include "KoPathPoint.h"
#include "KoPathPointData.h"
#include "flake_export.h"

class KoShapeController;
class KoPathPointRemoveCommandPrivate;

/// The undo / redo command for removing path points.
class FLAKE_TEST_EXPORT KoPathPointRemoveCommand : public QUndoCommand
{
public:
    /**
     * @brief Create command for removing points from path shapes
     *
     * This will create the command for removing points from path shapes. If all
     * points from a path shape are deleted it will delete the path shape. If all
     * points from a subpath are deleted it will delete the subpath.
     *
     * @param pointDataList List of point data to remove
     * @param shapeController shape controller in charge
     * @param parent the parent command used for macro commands
     */
    static QUndoCommand *createCommand(const QList<KoPathPointData> &pointDataList, KoShapeController *shapeController, QUndoCommand *parent = 0);

    /**
     * @brief Command to remove a points from path shapes
     *
     * Don't use this directly use createCommand instead.
     *
     * @param pointDataList List of point data to remove.
     * @param parent the parent command used for macro commands
     */
    explicit KoPathPointRemoveCommand(const QList<KoPathPointData> &pointDataList, QUndoCommand *parent = 0);
    ~KoPathPointRemoveCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    KoPathPointRemoveCommandPrivate *d;
};

#endif // KOPATHPOINTREMOVECOMMAND_H
