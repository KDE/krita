/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPECREATECOMMAND_H
#define KOSHAPECREATECOMMAND_H

#include "kritaflake_export.h"
#include <kundo2command.h>

class KoShape;
class KoShapeContainer;
class KoShapeControllerBase;

/// The undo / redo command for creating shapes
class KRITAFLAKE_EXPORT KoShapeCreateCommand : public KUndo2Command
{
public:
    /**
     * Command used on creation of new shapes
     * @param controller the controller used to add/remove the shape from
     * @param shape the shape that's just been created.
     * @param parent the parent command used for macro commands
     */
    KoShapeCreateCommand(KoShapeControllerBase *controller, KoShape *shape,
                         KoShapeContainer *parentShape = 0,
                         KUndo2Command *parent = 0);

   /**
    * Command used on creation of new shapes
    * @param controller the controller used to add/remove the shape from
    * @param shapes the shapes that have just been created.
    * @param parent the parent command used for macro commands
    */
    KoShapeCreateCommand(KoShapeControllerBase *controller, const QList<KoShape*> shape,
                         KoShapeContainer *parentShape = 0,
                         KUndo2Command *parent = 0);

    ~KoShapeCreateCommand() override;
    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

protected:
    KoShapeCreateCommand(KoShapeControllerBase *controller, const QList<KoShape *> shapes,
                         KoShapeContainer *parentShape, KUndo2Command *parent,
                         const KUndo2MagicString &undoString);

private:
    class Private;
    Private * const d;
};

#endif
