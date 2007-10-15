/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeMoveCommand.h"

#include <klocale.h>

class KoShapeMoveCommand::Private {
public:
    QList<KoShape*> shapes;
    QList<QPointF> previousPositions, newPositions;
};

KoShapeMoveCommand::KoShapeMoveCommand(const QList<KoShape*> &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions, QUndoCommand *parent)
: QUndoCommand( parent ),
    d(new Private())
{
    d->shapes = shapes;
    d->previousPositions = previousPositions;
    d->newPositions = newPositions;
    Q_ASSERT(d->shapes.count() == d->previousPositions.count());
    Q_ASSERT(d->shapes.count() == d->newPositions.count());

    setText( i18n( "Move shapes" ) );
}

KoShapeMoveCommand::~KoShapeMoveCommand() {
    delete d;
}

void KoShapeMoveCommand::redo() {
    QUndoCommand::redo();
    for(int i=0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->setPosition( d->newPositions.at(i) );
        d->shapes.at(i)->update();
    }
}

void KoShapeMoveCommand::undo() {
    QUndoCommand::undo();
    for(int i=0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->setPosition( d->previousPositions.at(i) );
        d->shapes.at(i)->update();
    }
}

/// update newPositions list with new postions.
void KoShapeMoveCommand::setNewPositions(QList<QPointF> newPositions) {
    d->newPositions = newPositions;
}
