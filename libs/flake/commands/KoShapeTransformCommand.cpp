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

#include "KoShapeTransformCommand.h"
#include "KoShape.h"

#include <QList>
#include <QTransform>

#include <KDebug>

class KoShapeTransformCommand::Private
{
public:
    Private(const QList<KoShape*> &list) : shapes(list) { }
    QList<KoShape*> shapes;
    QList<QTransform> oldState;
    QList<QTransform> newState;
};

KoShapeTransformCommand::KoShapeTransformCommand(const QList<KoShape*> &shapes, const QList<QTransform> &oldState, const QList<QTransform> &newState, QUndoCommand * parent)
        : QUndoCommand(parent),
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
    QUndoCommand::redo();

    const int shapeCount = d->shapes.count();
    for (int i = 0; i < shapeCount; ++i) {
        KoShape * shape = d->shapes[i];
        shape->update();
        shape->setTransformation(d->newState[i]);
        shape->update();
    }
}

void KoShapeTransformCommand::undo()
{
    QUndoCommand::undo();

    const int shapeCount = d->shapes.count();
    for (int i = 0; i < shapeCount; ++i) {
        KoShape * shape = d->shapes[i];
        shape->update();
        shape->setTransformation(d->oldState[i]);
        shape->update();
    }
}
