/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPECONNECTIONCHANGECOMMAND_H
#define KOSHAPECONNECTIONCHANGECOMMAND_H

#include "kritaflake_export.h"
#include <kundo2command.h>
#include "KoConnectionShape.h"

/// A command to add a connection between two shapes
class KRITAFLAKE_EXPORT KoShapeConnectionChangeCommand : public KUndo2Command
{
public:
    /**
     * Creates command to connect aconnection shape to a shape
     * @param connection the connection shape to connect to the shape
     * @param connectionHandle the handle of the connection to connect to
     * @param oldConnectedShape the old shape we were connected to
     * @param newConnectedShape the new shape to connect to
     * @param connectionPointId the id of the connection point to connect to
     * @param parent the parent undo command
     */
    KoShapeConnectionChangeCommand(KoConnectionShape *connection, KoConnectionShape::HandleId connectionHandle,
                                   KoShape *oldConnectedShape, int oldConnectionPointId,
                                   KoShape *newConnectedShape, int newConnectionPointId, KUndo2Command *parent = 0);

    /// Destroys the command
    ~KoShapeConnectionChangeCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

private:
    class Private;
    Private * const d;
};

#endif // KOSHAPECONNECTIONCHANGECOMMAND_H
