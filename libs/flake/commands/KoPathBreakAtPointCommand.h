/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHBREAKATPOINTCOMMAND_H
#define KOPATHBREAKATPOINTCOMMAND_H

#include <kundo2command.h>
#include <QList>
#include "KoPathPointData.h"

class KoPathPoint;

/// Command to break a subpath at points.
class KoPathBreakAtPointCommand : public KUndo2Command
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
    explicit KoPathBreakAtPointCommand(const QList<KoPathPointData> &pointDataList, KUndo2Command *parent = 0);
    ~KoPathBreakAtPointCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    QList<KoPathPointData> m_pointDataList;
    QList<KoPathPoint*> m_points;
    // used for storing where to open the subpath. In case it not used for the open
    // status use .second to the store offset caused by a open of a subpath.
    QList<KoPathPointIndex> m_closedIndex;
    bool m_deletePoints;
};

#endif // KOPATHBREAKATPOINTCOMMAND_H
