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
        oldFills.clear();
        newFills.clear();
    }

    void addOldFill(QPointer<KoShapeBackground>  oldFill)
    {
        oldFills.append(oldFill);
    }

    void addNewFill(QPointer<KoShapeBackground>  newFill)
    {
        newFills.append(newFill);
    }

    QList<KoShape*> shapes;    ///< the shapes to set background for
    QList<QPointer<KoShapeBackground> > oldFills;
    QList<QPointer<KoShapeBackground> > newFills;
};

KoShapeBackgroundCommand::KoShapeBackgroundCommand(const QList<KoShape*> &shapes, QPointer<KoShapeBackground>  fill,
        KUndo2Command *parent)
        : KUndo2Command(parent)
        , d(new Private())
{
    d->shapes = shapes;
    foreach(KoShape *shape, d->shapes) {
        d->addOldFill(shape->background());
        d->addNewFill(fill);
    }

    setText(i18nc("(qtundo-format)", "Set background"));
}

KoShapeBackgroundCommand::KoShapeBackgroundCommand(KoShape * shape, QPointer<KoShapeBackground>  fill, KUndo2Command *parent)
        : KUndo2Command(parent)
        , d(new Private())
{
    d->shapes.append(shape);
    d->addOldFill(shape->background());
    d->addNewFill(fill);

    setText(i18nc("(qtundo-format)", "Set background"));
}

KoShapeBackgroundCommand::KoShapeBackgroundCommand(const QList<KoShape*> &shapes, const QList<QPointer<KoShapeBackground> > &fills, KUndo2Command *parent)
        : KUndo2Command(parent)
        , d(new Private())
{
    d->shapes = shapes;
    foreach(KoShape *shape, d->shapes) {
        d->addOldFill(shape->background());
    }
    foreach (QPointer<KoShapeBackground>  fill, fills) {
        d->addNewFill(fill);
    }

    setText(i18nc("(qtundo-format)", "Set background"));
}

void KoShapeBackgroundCommand::redo()
{
    KUndo2Command::redo();
    QList<QPointer<KoShapeBackground> >::iterator brushIt = d->newFills.begin();
    foreach(KoShape *shape, d->shapes) {
        shape->setBackground(*brushIt);
        shape->update();
        ++brushIt;
    }
}

void KoShapeBackgroundCommand::undo()
{
    KUndo2Command::undo();
    QList<QPointer<KoShapeBackground> >::iterator brushIt = d->oldFills.begin();
    foreach(KoShape *shape, d->shapes) {
        shape->setBackground(*brushIt);
        shape->update();
        ++brushIt;
    }
}

KoShapeBackgroundCommand::~KoShapeBackgroundCommand()
{
    delete d;
}
