/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
