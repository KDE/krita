/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeBackgroundCommand.h"
#include "KoShape.h"
#include "KoShapeBackground.h"

#include <klocale.h>

class KoShapeBackgroundCommand::Private
{
public:
    Private() {
    }
    ~Private() {
        foreach(KoShapeBackground* fill, oldFills) {
            if (fill && !fill->deref())
                delete fill;
        }
        foreach(KoShapeBackground* fill, newFills) {
            if (fill && !fill->deref())
                delete fill;
        }
    }

    void addOldFill(KoShapeBackground * oldFill)
    {
        if (oldFill)
            oldFill->ref();
        oldFills.append(oldFill);
    }

    void addNewFill(KoShapeBackground * newFill)
    {
        if (newFill)
            newFill->ref();
        newFills.append(newFill);
    }

    QList<KoShape*> shapes;    ///< the shapes to set background for
    QList<KoShapeBackground*> oldFills;
    QList<KoShapeBackground*> newFills;
};

KoShapeBackgroundCommand::KoShapeBackgroundCommand(const QList<KoShape*> &shapes, KoShapeBackground * fill,
        QUndoCommand *parent)
        : QUndoCommand(parent)
        , d(new Private())
{
    d->shapes = shapes;
    foreach(KoShape *shape, d->shapes) {
        d->addOldFill(shape->background());
        d->addNewFill(fill);
    }

    setText(i18n("Set background"));
}

KoShapeBackgroundCommand::KoShapeBackgroundCommand(KoShape * shape, KoShapeBackground * fill, QUndoCommand *parent)
        : QUndoCommand(parent)
        , d(new Private())
{
    d->shapes.append(shape);
    d->addOldFill(shape->background());
    d->addNewFill(fill);

    setText(i18n("Set background"));
}

KoShapeBackgroundCommand::KoShapeBackgroundCommand(const QList<KoShape*> &shapes, const QList<KoShapeBackground*> &fills, QUndoCommand *parent)
        : QUndoCommand(parent)
        , d(new Private())
{
    d->shapes = shapes;
    foreach(KoShape *shape, d->shapes) {
        d->addOldFill(shape->background());
    }
    foreach (KoShapeBackground * fill, fills) {
        d->addNewFill(fill);
    }

    setText(i18n("Set background"));
}

void KoShapeBackgroundCommand::redo()
{
    QUndoCommand::redo();
    QList<KoShapeBackground*>::iterator brushIt = d->newFills.begin();
    foreach(KoShape *shape, d->shapes) {
        shape->setBackground(*brushIt);
        shape->update();
        brushIt++;
    }
}

void KoShapeBackgroundCommand::undo()
{
    QUndoCommand::undo();
    QList<KoShapeBackground*>::iterator brushIt = d->oldFills.begin();
    foreach(KoShape *shape, d->shapes) {
        shape->setBackground(*brushIt);
        shape->update();
        brushIt++;
    }
}

KoShapeBackgroundCommand::~KoShapeBackgroundCommand()
{
    delete d;
}
