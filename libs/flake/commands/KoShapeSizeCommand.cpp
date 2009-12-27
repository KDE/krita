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

#include "KoShapeSizeCommand.h"

#include <klocale.h>

class KoShapeSizeCommand::Private
{
public:
    QList<KoShape*> shapes;
    QList<QSizeF> previousSizes, newSizes;
};

KoShapeSizeCommand::KoShapeSizeCommand(const QList<KoShape*> &shapes, const QList<QSizeF> &previousSizes, const QList<QSizeF> &newSizes, QUndoCommand *parent)
        : QUndoCommand(parent),
        d(new Private())
{
    d->previousSizes = previousSizes;
    d->newSizes = newSizes;
    d->shapes = shapes;
    Q_ASSERT(d->shapes.count() == d->previousSizes.count());
    Q_ASSERT(d->shapes.count() == d->newSizes.count());

    setText(i18n("Resize shapes"));
}

KoShapeSizeCommand::~KoShapeSizeCommand()
{
    delete d;
}

void KoShapeSizeCommand::redo()
{
    QUndoCommand::redo();
    int i = 0;
    foreach(KoShape *shape, d->shapes) {
        shape->update();
        shape->setSize(d->newSizes[i++]);
        shape->update();
    }
}

void KoShapeSizeCommand::undo()
{
    QUndoCommand::undo();
    int i = 0;
    foreach(KoShape *shape, d->shapes) {
        shape->update();
        shape->setSize(d->previousSizes[i++]);
        shape->update();
    }
}
