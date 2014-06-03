/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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

#ifndef KOSHAPESTROKECOMMAND_H
#define KOSHAPESTROKECOMMAND_H

#include "flake_export.h"

#include <kundo2command.h>
#include <QList>

class KoShape;
class KoShapeStrokeModel;

/// The undo / redo command for setting the shape stroke
class FLAKE_EXPORT KoShapeStrokeCommand : public KUndo2Command
{
public:
    /**
     * Command to set a new shape stroke.
     * @param shapes a set of all the shapes that should get the new stroke.
     * @param stroke the new stroke, the same for all given shapes
     * @param parent the parent command used for macro commands
     */
    KoShapeStrokeCommand(const QList<KoShape*> &shapes, KoShapeStrokeModel *stroke, KUndo2Command *parent = 0);

    /**
     * Command to set new shape strokes.
     * @param shapes a set of all the shapes that should get a new stroke.
     * @param strokes the new strokes, one for each shape
     * @param parent the parent command used for macro commands
     */
    KoShapeStrokeCommand(const QList<KoShape*> &shapes, const QList<KoShapeStrokeModel*> &strokes, KUndo2Command *parent = 0);

    /**
     * Command to set a new shape stroke.
     * @param shape a single shape that should get the new stroke.
     * @param stroke the new stroke
     * @param parent the parent command used for macro commands
     */
    KoShapeStrokeCommand(KoShape* shape, KoShapeStrokeModel *stroke, KUndo2Command *parent = 0);

    virtual ~KoShapeStrokeCommand();
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    class Private;
    Private * const d;
};

#endif
