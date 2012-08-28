/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOSHAPERUNAROUNDCOMMAND_H
#define KOSHAPERUNAROUNDCOMMAND_H

#include "flake_export.h"
#include <kundo2command.h>

#include <KoShape.h>

/// API docs go here
class FLAKE_EXPORT KoShapeRunAroundCommand : public KUndo2Command
{
public:
    KoShapeRunAroundCommand(KoShape *shape, KoShape::TextRunAroundSide side, int runThrough, qreal distanceLeft, qreal distanceTop, qreal distanceRight, qreal distanceBottom, qreal threshold, KoShape::TextRunAroundContour contour, KUndo2Command *parent = 0);
    virtual ~KoShapeRunAroundCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    class Private;
    Private * const d;
};

#endif /* KOSHAPERUNAROUNDCOMMAND_H */
