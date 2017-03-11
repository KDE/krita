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

#include <KoShape.h>
#include <klocalizedstring.h>
#include "kis_command_ids.h"

class Q_DECL_HIDDEN KoShapeMoveCommand::Private
{
public:
    QList<KoShape*> shapes;
    QList<QPointF> previousPositions, newPositions;
    KoFlake::AnchorPosition anchor;
};

KoShapeMoveCommand::KoShapeMoveCommand(const QList<KoShape*> &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions, KoFlake::AnchorPosition anchor, KUndo2Command *parent)
        : KUndo2Command(parent),
        d(new Private())
{
    d->shapes = shapes;
    d->previousPositions = previousPositions;
    d->newPositions = newPositions;
    d->anchor = anchor;
    Q_ASSERT(d->shapes.count() == d->previousPositions.count());
    Q_ASSERT(d->shapes.count() == d->newPositions.count());

    setText(kundo2_i18n("Move shapes"));
}

KoShapeMoveCommand::~KoShapeMoveCommand()
{
    delete d;
}

void KoShapeMoveCommand::redo()
{
    KUndo2Command::redo();
    for (int i = 0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->setAbsolutePosition(d->newPositions.at(i), d->anchor);
        d->shapes.at(i)->update();
    }
}

void KoShapeMoveCommand::undo()
{
    KUndo2Command::undo();
    for (int i = 0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->setAbsolutePosition(d->previousPositions.at(i), d->anchor);
        d->shapes.at(i)->update();
    }
}

int KoShapeMoveCommand::id() const
{
    return KisCommandUtils::MoveShapeId;
}

bool KoShapeMoveCommand::mergeWith(const KUndo2Command *command)
{
    const KoShapeMoveCommand *other = dynamic_cast<const KoShapeMoveCommand*>(command);

    if (other->d->shapes != d->shapes ||
        other->d->anchor != d->anchor) {

        return false;
    }

    d->newPositions = other->d->newPositions;
    return true;
}
