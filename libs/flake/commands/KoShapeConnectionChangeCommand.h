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

#ifndef KOSHAPECONNECTIONCHANGECOMMAND_H
#define KOSHAPECONNECTIONCHANGECOMMAND_H

#include "flake_export.h"
#include <kundo2command.h>
#include "KoConnectionShape.h"

/// A command to add a connection between two shapes
class FLAKE_EXPORT KoShapeConnectionChangeCommand : public KUndo2Command
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
    virtual ~KoShapeConnectionChangeCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    class Private;
    Private * const d;
};

#endif // KOSHAPECONNECTIONCHANGECOMMAND_H
