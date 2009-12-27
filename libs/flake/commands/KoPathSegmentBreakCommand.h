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

#ifndef KOPATHSEGMENTBREAKCOMMAND_H
#define KOPATHSEGMENTBREAKCOMMAND_H

#include <QUndoCommand>
#include "KoPathPoint.h"
#include "KoPathPointData.h"

/// The undo / redo command for breaking a subpath by removing the segment
class KoPathSegmentBreakCommand : public QUndoCommand
{
public:
    /**
     * Command to break a subpath by removing the segement
     *
     * The segment following the given point will be removed.
     *
     * @param pointData describing the point
     * @param parent the parent command used for macro commands
     */
    explicit KoPathSegmentBreakCommand(const KoPathPointData &pointData, QUndoCommand *parent = 0);
    ~KoPathSegmentBreakCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KoPathPointData m_pointData;
    KoPathPointIndex m_startIndex;
    bool m_broken;
};

#endif // KOPATHSEGMENTBREAKCOMMAND_H
