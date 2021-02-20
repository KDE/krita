/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_command_ids.h"

#include "KoShapeTransformCommand.h"
#include "KoShape.h"

#include <QList>
#include <QTransform>

#include <FlakeDebug.h>

class Q_DECL_HIDDEN KoShapeTransformCommand::Private
{
public:
    Private(const QList<KoShape*> &list) : shapes(list) { }
    QList<KoShape*> shapes;
    QList<QTransform> oldState;
    QList<QTransform> newState;
};

KoShapeTransformCommand::KoShapeTransformCommand(const QList<KoShape*> &shapes, const QList<QTransform> &oldState, const QList<QTransform> &newState, KUndo2Command * parent)
        : KUndo2Command(parent),
        d(new Private(shapes))
{
    Q_ASSERT(shapes.count() == oldState.count());
    Q_ASSERT(shapes.count() == newState.count());
    d->oldState = oldState;
    d->newState = newState;
}

KoShapeTransformCommand::~KoShapeTransformCommand()
{
    delete d;
}

void KoShapeTransformCommand::redo()
{
    KUndo2Command::redo();

    const int shapeCount = d->shapes.count();
    for (int i = 0; i < shapeCount; ++i) {
        KoShape * shape = d->shapes[i];
        const QRectF oldDirtyRect = shape->boundingRect();
        shape->setTransformation(d->newState[i]);
        shape->updateAbsolute(oldDirtyRect | shape->boundingRect());
    }
}

void KoShapeTransformCommand::undo()
{
    KUndo2Command::undo();

    const int shapeCount = d->shapes.count();
    for (int i = 0; i < shapeCount; ++i) {
        KoShape * shape = d->shapes[i];
        const QRectF oldDirtyRect = shape->boundingRect();
        shape->setTransformation(d->oldState[i]);
        shape->updateAbsolute(oldDirtyRect | shape->boundingRect());
    }
}

int KoShapeTransformCommand::id() const
{
    return KisCommandUtils::TransformShapeId;
}

bool KoShapeTransformCommand::mergeWith(const KUndo2Command *command)
{
    const KoShapeTransformCommand *other = dynamic_cast<const KoShapeTransformCommand*>(command);

    if (!other ||
        other->d->shapes != d->shapes ||
        other->text() != text()) {

        return false;
    }

    d->newState = other->d->newState;
    return true;
}
