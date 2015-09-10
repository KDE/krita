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

#include "KoShapeCreateCommand.h"
#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoShapeBasedDocumentBase.h"

#include <klocalizedstring.h>

class Q_DECL_HIDDEN KoShapeCreateCommand::Private
{
public:
    Private(KoShapeBasedDocumentBase *c, KoShape *s)
            : controller(c),
            shape(s),
            shapeParent(shape->parent()),
            deleteShape(true) {
    }
    ~Private() {
        if (shape && deleteShape)
            delete shape;
    }

    KoShapeBasedDocumentBase *controller;
    KoShape *shape;
    KoShapeContainer *shapeParent;
    bool deleteShape;
};

KoShapeCreateCommand::KoShapeCreateCommand(KoShapeBasedDocumentBase *controller, KoShape *shape, KUndo2Command *parent)
        : KUndo2Command(parent),
        d(new Private(controller, shape))
{
    setText(kundo2_i18n("Create shape"));
}

KoShapeCreateCommand::~KoShapeCreateCommand()
{
    delete d;
}

void KoShapeCreateCommand::redo()
{
    KUndo2Command::redo();
    Q_ASSERT(d->shape);
    Q_ASSERT(d->controller);
    if (d->shapeParent)
        d->shapeParent->addShape(d->shape);
    // the parent has to be there when it is added to the KoShapeBasedDocumentBase
    d->controller->addShape(d->shape);
    d->shapeParent = d->shape->parent(); // update parent if the 'addShape' changed it
    d->deleteShape = false;
}

void KoShapeCreateCommand::undo()
{
    KUndo2Command::undo();
    Q_ASSERT(d->shape);
    Q_ASSERT(d->controller);
    // the parent has to be there when it is removed from the KoShapeBasedDocumentBase
    d->controller->removeShape(d->shape);
    if (d->shapeParent)
        d->shapeParent->removeShape(d->shape);
    d->deleteShape = true;
}
