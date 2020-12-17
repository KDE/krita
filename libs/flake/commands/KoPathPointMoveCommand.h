/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2009 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHPOINTMOVECOMMAND_H
#define KOPATHPOINTMOVECOMMAND_H

#include "kritaflake_export.h"

#include <kundo2command.h>

#include "KoPathPointData.h"

class KoPathPointMoveCommandPrivate;
class QPointF;

/// The undo / redo command for path point moving.
class KRITAFLAKE_EXPORT KoPathPointMoveCommand : public KUndo2Command
{
public:
    /**
     * Command to move path points.
     * @param pointData the path points to move
     * @param offset the offset by which the point is moved in document coordinates
     * @param parent the parent command used for macro commands
     */
    KoPathPointMoveCommand(const QList<KoPathPointData> &pointData, const QPointF &offset, KUndo2Command *parent = 0);

    /**
    * Command to move path points.
    * @param pointData the path points to move
    * @param offsets the offsets by which the points are moved in document coordinates
    * @param parent the parent command used for macro commands
    */
    KoPathPointMoveCommand(const QList<KoPathPointData> &pointData, const QList<QPointF> &offsets, KUndo2Command *parent = 0);

    ~KoPathPointMoveCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *command) override;

private:
    KoPathPointMoveCommandPrivate * const d;
};

#endif // KOPATHPOINTMOVECOMMAND_H
