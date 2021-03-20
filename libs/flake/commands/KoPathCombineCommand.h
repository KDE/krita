/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHCOMBINECOMMAND_H
#define KOPATHCOMBINECOMMAND_H

#include <kundo2command.h>
#include <QList>
#include "kritaflake_export.h"

class KoShapeControllerBase;
class KoPathShape;
class KoPathPointData;

/// The undo / redo command for combining two or more paths into one
class KRITAFLAKE_EXPORT KoPathCombineCommand : public KUndo2Command
{
public:
    /**
     * Command for combining a list of paths into one single path.
     * @param controller the controller to used for removing/inserting.
     * @param paths the list of paths to combine
     * @param parent the parent command used for macro commands
     */
    KoPathCombineCommand(KoShapeControllerBase *controller, const QList<KoPathShape*> &paths, KUndo2Command *parent = 0);
    ~KoPathCombineCommand() override;
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

    KoPathShape *combinedPath() const;
    KoPathPointData originalToCombined(KoPathPointData pd) const;

private:
    class Private;
    Private * const d;
};

#endif // KOPATHCOMBINECOMMAND_H
