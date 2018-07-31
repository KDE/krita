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

#ifndef KOSHAPEUNCLIPCOMMAND_H
#define KOSHAPEUNCLIPCOMMAND_H

#include "kritaflake_export.h"
#include <kundo2command.h>
#include <QList>

class KoShape;
class KoShapeControllerBase;

/// The undo / redo command for removing the shape clip path
class KRITAFLAKE_EXPORT KoShapeUnclipCommand : public KUndo2Command
{
public:
    /**
     * Command to remove clip path from multiple shapes.
     * @param controller the controller to used for adding the clip shapes.
     * @param shapes a set of all the shapes to remove the clip path from.
     * @param parent the parent command used for macro commands
     */
    KoShapeUnclipCommand(KoShapeControllerBase *controller, const QList<KoShape*> &shapes, KUndo2Command *parent = 0);

    /**
     * Command to remove clip path from a single shape.
     * @param controller the controller to used for adding the clip shapes.
     * @param shape a single shape to remove the clip path from.
     * @param parent the parent command used for macro commands
     */
    KoShapeUnclipCommand(KoShapeControllerBase *controller, KoShape *shape, KUndo2Command *parent = 0);

    /// Destroys the command
    ~KoShapeUnclipCommand() override;
    
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    class Private;
    Private * const d;
};

#endif // KOSHAPEUNCLIPCOMMAND_H
