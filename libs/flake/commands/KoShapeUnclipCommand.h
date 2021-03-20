/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
