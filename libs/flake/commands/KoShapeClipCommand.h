/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSHAPECLIPCOMMAND_H
#define KOSHAPECLIPCOMMAND_H

#include "kritaflake_export.h"
#include <kundo2command.h>
#include <QList>

class KoShape;
class KoPathShape;
class KoShapeControllerBase;

/// The undo / redo command for setting the shape clip path
class KRITAFLAKE_EXPORT KoShapeClipCommand : public KUndo2Command
{
public:
    /**
     * Command to set a new shape clipping path for multiple shape.
     * @param controller the controller to used for deleting.
     * @param shapes a set of all the shapes that should get the clipping path.
     * @param clipPathShapes the path shapes to be used a clipping path
     * @param parent the parent command used for macro commands
     */
    KoShapeClipCommand(KoShapeControllerBase *controller, const QList<KoShape*> &shapes, const QList<KoPathShape*> &clipPathShapes, KUndo2Command *parent = 0);

    /**
     * Command to set a new shape clipping path for a single shape
     * @param controller the controller to used for deleting.
     * @param shape a single shape that should get the new shadow.
     * @param clipPathShapes the path shapes to be used a clipping path
     * @param parent the parent command used for macro commands
     */
    KoShapeClipCommand(KoShapeControllerBase *controller, KoShape *shape, const QList<KoPathShape*> &clipPathShapes, KUndo2Command *parent = 0);

    /// Destroys the command
    ~KoShapeClipCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    class Private;
    Private * const d;
};

#endif // KOSHAPECLIPCOMMAND_H
