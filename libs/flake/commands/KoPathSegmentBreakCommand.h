/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHSEGMENTBREAKCOMMAND_H
#define KOPATHSEGMENTBREAKCOMMAND_H

#include <kundo2command.h>
#include "KoPathPointData.h"

/// The undo / redo command for breaking a subpath by removing the segment
class KoPathSegmentBreakCommand : public KUndo2Command
{
public:
    /**
     * Command to break a subpath by removing the segment
     *
     * The segment following the given point will be removed.
     *
     * @param pointData describing the point
     * @param parent the parent command used for macro commands
     */
    explicit KoPathSegmentBreakCommand(const KoPathPointData &pointData, KUndo2Command *parent = 0);
    ~KoPathSegmentBreakCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;
private:
    KoPathPointData m_pointData;
    KoPathPointIndex m_startIndex;
    bool m_broken;
};

#endif // KOPATHSEGMENTBREAKCOMMAND_H
