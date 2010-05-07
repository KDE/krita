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

#include "KoShapeDeleteCommand.h"
#include "KoShapeContainer.h"
#include "KoShapeControllerBase.h"

#include <klocale.h>

class KoShapeDeleteCommand::Private
{
public:
    Private(KoShapeControllerBase *c)
            : controller(c),
            deleteShapes(false) {
    }

    ~Private() {
        if (! deleteShapes)
            return;

        foreach(KoShape *shape, shapes)
            delete shape;
    }

    KoShapeControllerBase *controller; ///< the shape controller to use for removing/readding
    QList<KoShape*> shapes; ///< the list of shapes to delete
    QList<KoShapeContainer*> oldParents; ///< the old parents of the shapes
    bool deleteShapes;  ///< shows if shapes should be deleted when deleting the command
};

KoShapeDeleteCommand::KoShapeDeleteCommand(KoShapeControllerBase *controller, KoShape *shape, QUndoCommand *parent)
        : QUndoCommand(parent),
        d(new Private(controller))
{
    d->shapes.append(shape);
    d->oldParents.append(shape->parent());

    setText(i18n("Delete shape"));
}

KoShapeDeleteCommand::KoShapeDeleteCommand(KoShapeControllerBase *controller, const QList<KoShape*> &shapes,
        QUndoCommand *parent)
        : QUndoCommand(parent),
        d(new Private(controller))
{
    d->shapes = shapes;
    foreach(KoShape *shape, d->shapes) {
        d->oldParents.append(shape->parent());
    }

    setText(i18n("Delete shapes"));
}

KoShapeDeleteCommand::~KoShapeDeleteCommand()
{
    delete d;
}

void KoShapeDeleteCommand::redo()
{
    QUndoCommand::redo();
    if (! d->controller)
        return;

    for (int i = 0; i < d->shapes.count(); i++) {
        // the parent has to be there when it is removed from the KoShapeControllerBase
        d->controller->removeShape(d->shapes[i]);
        if (d->oldParents.at(i))
            d->oldParents.at(i)->removeShape(d->shapes[i]);
    }
    d->deleteShapes = true;
}

void KoShapeDeleteCommand::undo()
{
    QUndoCommand::undo();
    if (! d->controller)
        return;

    for (int i = 0; i < d->shapes.count(); i++) {
        if (d->oldParents.at(i))
            d->oldParents.at(i)->removeShape(d->shapes[i]);
        // the parent has to be there when it is added to the KoShapeControllerBase
        d->controller->addShape(d->shapes[i]);
    }
    d->deleteShapes = false;
}
