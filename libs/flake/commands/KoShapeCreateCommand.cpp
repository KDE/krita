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
#include "KoShapeControllerBase.h"

#include <klocale.h>

class KoShapeCreateCommand::Private {
public:
    Private(KoShapeControllerBase *c, KoShape *s)
        : controller(c),
        shape(s),
        shapeParent( shape->parent() ),
        deleteShape(true)
    {
    }
    ~Private() {
        if( shape && deleteShape )
            delete shape;
    }

    KoShapeControllerBase *controller;
    KoShape *shape;
    KoShapeContainer *shapeParent;
    bool deleteShape;
};

KoShapeCreateCommand::KoShapeCreateCommand( KoShapeControllerBase *controller, KoShape *shape, QUndoCommand *parent )
    : QUndoCommand( parent ),
    d(new Private(controller, shape))
{
    setText( i18n( "Create shape" ) );
}

KoShapeCreateCommand::~KoShapeCreateCommand() {
    delete d;
}

void KoShapeCreateCommand::redo () {
    QUndoCommand::redo();
    Q_ASSERT(d->shape);
    Q_ASSERT(d->controller);
    if( d->shapeParent )
        d->shapeParent->addChild( d->shape );
    // the parent has to be there when it is added to the KoShapeControllerBase
    recurse(d->shape, Add);
    d->deleteShape = false;
}

void KoShapeCreateCommand::undo () {
    QUndoCommand::undo();
    Q_ASSERT(d->shape);
    Q_ASSERT(d->controller);
    // the parent has to be there when it is removed from the KoShapeControllerBase
    recurse(d->shape, Remove);
    if( d->shapeParent )
        d->shapeParent->removeChild( d->shape );
    d->deleteShape = true;
}

void KoShapeCreateCommand::recurse(KoShape *shape, const AddRemove ar) {
    if(ar == Remove)
        d->controller->removeShape( d->shape );
    else
        d->controller->addShape( d->shape );

    KoShapeContainer *container = dynamic_cast<KoShapeContainer*> (shape);
    if(container) {
        foreach(KoShape *child, container->iterator())
            recurse(child, ar);
    }
}
