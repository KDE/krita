/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHPOINTREMOVECOMMAND_H
#define KOPATHPOINTREMOVECOMMAND_H

#include <kundo2command.h>
#include <QList>
#include "KoPathPointData.h"
#include "kritaflake_export.h"

class KoShapeController;
class KoPathPointRemoveCommandPrivate;

/// The undo / redo command for removing path points.
class KRITAFLAKE_EXPORT KoPathPointRemoveCommand : public KUndo2Command
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
    static KUndo2Command *createCommand(const QList<KoPathPointData> &pointDataList, KoShapeController *shapeController, KUndo2Command *parent = 0);

    /**
     * @brief Command to remove a points from path shapes
     *
     * Don't use this directly use createCommand instead.
     *
     * @param pointDataList List of point data to remove.
     * @param parent the parent command used for macro commands
     */
    explicit KoPathPointRemoveCommand(const QList<KoPathPointData> &pointDataList, KUndo2Command *parent = 0);
    ~KoPathPointRemoveCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    KoPathPointRemoveCommandPrivate *d;
};

#endif // KOPATHPOINTREMOVECOMMAND_H
