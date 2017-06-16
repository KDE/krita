/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
