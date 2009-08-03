/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOPATHPOINTMERGECOMMAND_H
#define KOPATHPOINTMERGECOMMAND_H

#include <QUndoCommand>

#include "flake_export.h"

class KoPathPointData;

/// The undo / redo command for merging two subpath end points
class FLAKE_EXPORT KoPathPointMergeCommand : public QUndoCommand
{
public:
    /**
     * Command to merge two subpath end points.
     *
     * The points have to be from the same path shape.
     *
     * @param pointData1 the data of the first point to merge
     * @param pointData2 the data of the second point to merge
     * @param parent the parent command used for macro commands
     */
    KoPathPointMergeCommand(const KoPathPointData &pointData1, const KoPathPointData &pointData2, QUndoCommand *parent = 0);
    ~KoPathPointMergeCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:

    class Private;
    Private * const d;
};

#endif // KOPATHPOINTMERGECOMMAND_H
