/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeTransparencyCommand.h"
#include "KoShape.h"
#include "KoShapeBackground.h"

#include <klocale.h>

class KoShapeTransparencyCommand::Private
{
public:
    Private() {
    }
    ~Private() {
    }

    QList<KoShape*> shapes;    ///< the shapes to set background for
    QList<qreal> oldTransparencies; ///< the old transparencies
    QList<qreal> newTransparencies; ///< the new transparencies
};

KoShapeTransparencyCommand::KoShapeTransparencyCommand(const QList<KoShape*> &shapes, qreal transparency, QUndoCommand *parent)
    : QUndoCommand(parent)
    , d(new Private())
{
    d->shapes = shapes;
    foreach(KoShape *shape, d->shapes) {
        d->oldTransparencies.append(shape->transparency());
        d->newTransparencies.append(transparency);
    }

    setText(i18n("Set opacity"));
}

KoShapeTransparencyCommand::KoShapeTransparencyCommand(KoShape * shape, qreal transparency, QUndoCommand *parent)
    : QUndoCommand(parent)
    , d(new Private())
{
    d->shapes.append(shape);
    d->oldTransparencies.append(shape->transparency());
    d->newTransparencies.append(transparency);

    setText(i18n("Set opacity"));
}

KoShapeTransparencyCommand::KoShapeTransparencyCommand(const QList<KoShape*> &shapes, const QList<qreal> &transparencies, QUndoCommand *parent)
    : QUndoCommand(parent)
    , d(new Private())
{
    d->shapes = shapes;
    foreach(KoShape *shape, d->shapes) {
        d->oldTransparencies.append(shape->transparency());
    }
    d->newTransparencies = transparencies;

    setText(i18n("Set opacity"));
}

KoShapeTransparencyCommand::~KoShapeTransparencyCommand()
{
    delete d;
}

void KoShapeTransparencyCommand::redo()
{
    QUndoCommand::redo();
    QList<qreal>::iterator transparencyIt = d->newTransparencies.begin();
    foreach(KoShape *shape, d->shapes) {
        shape->setTransparency(*transparencyIt);
        shape->update();
        transparencyIt++;
    }
}

void KoShapeTransparencyCommand::undo()
{
    QUndoCommand::undo();
    QList<qreal>::iterator transparencyIt = d->oldTransparencies.begin();
    foreach(KoShape *shape, d->shapes) {
        shape->setTransparency(*transparencyIt);
        shape->update();
        transparencyIt++;
    }
}
