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

#include "KoShapeShearCommand.h"
#include "KoShape.h"

#include <klocale.h>

class KoShapeShearCommandPrivate
{
public:
    QList<KoShape*> shapes;
    QList<qreal> previousShearXs;
    QList<qreal> previousShearYs;
    QList<qreal> newShearXs;
    QList<qreal> newShearYs;
};

KoShapeShearCommand::KoShapeShearCommand(const QList<KoShape*> &shapes, const QList<qreal> &previousShearXs, const QList<qreal> &previousShearYs, const QList<qreal> &newShearXs, const QList<qreal> &newShearYs, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KoShapeShearCommandPrivate())
{
    d->shapes = shapes;
    d->previousShearXs = previousShearXs;
    d->previousShearYs = previousShearYs;
    d->newShearXs = newShearXs;
    d->newShearYs = newShearYs;

    Q_ASSERT(d->shapes.count() == d->previousShearXs.count());
    Q_ASSERT(d->shapes.count() == d->previousShearYs.count());
    Q_ASSERT(d->shapes.count() == d->newShearXs.count());
    Q_ASSERT(d->shapes.count() == d->newShearYs.count());

    setText(i18n("Shear shapes"));
}

KoShapeShearCommand::~KoShapeShearCommand()
{
    delete d;
}

void KoShapeShearCommand::redo()
{
    QUndoCommand::redo();
    for (int i = 0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->shear(d->newShearXs.at(i), d->newShearYs.at(i));
        d->shapes.at(i)->update();
    }
}

void KoShapeShearCommand::undo()
{
    QUndoCommand::undo();
    for (int i = 0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->shear(d->previousShearXs.at(i), d->previousShearYs.at(i));
        d->shapes.at(i)->update();
    }
}
