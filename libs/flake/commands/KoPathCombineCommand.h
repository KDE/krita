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
