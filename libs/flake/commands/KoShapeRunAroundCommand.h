/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPERUNAROUNDCOMMAND_H
#define KOSHAPERUNAROUNDCOMMAND_H

#include "kritaflake_export.h"
#include <kundo2command.h>

#include "KoShape.h"

/// API docs go here
class KRITAFLAKE_EXPORT KoShapeRunAroundCommand : public KUndo2Command
{
public:
    KoShapeRunAroundCommand(KoShape *shape, KoShape::TextRunAroundSide side, int runThrough, qreal distanceLeft, qreal distanceTop, qreal distanceRight, qreal distanceBottom, qreal threshold, KoShape::TextRunAroundContour contour, KUndo2Command *parent = 0);
    ~KoShapeRunAroundCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    class Private;
    Private * const d;
};

#endif /* KOSHAPERUNAROUNDCOMMAND_H */
