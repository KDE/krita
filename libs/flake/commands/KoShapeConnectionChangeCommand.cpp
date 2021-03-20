/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
