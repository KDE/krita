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

#include "KoShapeConnectionChangeCommand.h"

class Q_DECL_HIDDEN KoShapeConnectionChangeCommand::Private
{
public:
    Private()
        : connection(0), newConnectedShape(0), oldConnectedShape(0)
        , newConnectionPointId(-1), oldConnectionPointId(-1)
    {
    }

    KoConnectionShape *connection;
    KoConnectionShape::HandleId connectionHandle;
    KoShape *newConnectedShape;
    KoShape *oldConnectedShape;
    int newConnectionPointId;
    int oldConnectionPointId;

};

KoShapeConnectionChangeCommand::KoShapeConnectionChangeCommand(
        KoConnectionShape *connection, KoConnectionShape::HandleId connectionHandle,
        KoShape *oldConnectedShape, int oldConnectionPointId,
        KoShape *newConnectedShape, int newConnectionPointId, KUndo2Command *parent)
    : KUndo2Command(parent), d(new Private)
{
    d->connection = connection;
    d->connectionHandle = connectionHandle;
    d->oldConnectedShape = oldConnectedShape;
    d->oldConnectionPointId = oldConnectionPointId;
    d->newConnectedShape = newConnectedShape;
    d->newConnectionPointId = newConnectionPointId;
}

KoShapeConnectionChangeCommand::~KoShapeConnectionChangeCommand()
{
    delete d;
}

void KoShapeConnectionChangeCommand::redo()
{
    if(d->connection) {
        if(d->connectionHandle == KoConnectionShape::StartHandle) {
            d->connection->connectFirst(d->newConnectedShape, d->newConnectionPointId);
        }
        else {
            d->connection->connectSecond(d->newConnectedShape, d->newConnectionPointId);
        }
    }

    KUndo2Command::redo();
}

void KoShapeConnectionChangeCommand::undo()
{
    KUndo2Command::undo();

    if(d->connection) {
        if(d->connectionHandle == KoConnectionShape::StartHandle) {
            d->connection->connectFirst(d->oldConnectedShape, d->oldConnectionPointId);
        }
        else {
            d->connection->connectSecond(d->oldConnectedShape, d->oldConnectionPointId);
        }
    }
}
