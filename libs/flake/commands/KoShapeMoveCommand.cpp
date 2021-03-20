/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
        : KUndo2Command(kundo2_i18n("Move shapes"), parent),
        d(new Private())
{
    d->shapes = shapes;
    d->previousPositions = previousPositions;
    d->newPositions = newPositions;
    d->anchor = anchor;
    Q_ASSERT(d->shapes.count() == d->previousPositions.count());
    Q_ASSERT(d->shapes.count() == d->newPositions.count());
}

KoShapeMoveCommand::KoShapeMoveCommand(const QList<KoShape *> &shapes, const QPointF &offset, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Move shapes"), parent),
      d(new Private())
{
    d->shapes = shapes;
    d->anchor = KoFlake::Center;

    Q_FOREACH (KoShape *shape, d->shapes) {
        const QPointF pos = shape->absolutePosition();

        d->previousPositions << pos;
        d->newPositions << pos + offset;
    }
}

KoShapeMoveCommand::~KoShapeMoveCommand()
{
    delete d;
}

void KoShapeMoveCommand::redo()
{
    KUndo2Command::redo();

    for (int i = 0; i < d->shapes.count(); i++) {
        KoShape *shape = d->shapes.at(i);

        const QRectF oldDirtyRect = shape->boundingRect();
        shape->setAbsolutePosition(d->newPositions.at(i), d->anchor);
        shape->updateAbsolute(oldDirtyRect | shape->boundingRect());
    }
}

void KoShapeMoveCommand::undo()
{
    KUndo2Command::undo();
    for (int i = 0; i < d->shapes.count(); i++) {
        KoShape *shape = d->shapes.at(i);

        const QRectF oldDirtyRect = shape->boundingRect();
        shape->setAbsolutePosition(d->previousPositions.at(i), d->anchor);
        shape->updateAbsolute(oldDirtyRect | shape->boundingRect());
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
