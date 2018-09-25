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

#include <klocalizedstring.h>

#include "kis_assert.h"
#include <KoShapeLayer.h>
#include <KoShapeReorderCommand.h>

#include <vector>
#include <memory>

class Q_DECL_HIDDEN KoShapeCreateCommand::Private
{
public:
    Private(KoShapeControllerBase *_document, const QList<KoShape*> &_shapes, KoShapeContainer *_parentShape)
            : shapesDocument(_document),
            shapes(_shapes),
            explicitParentShape(_parentShape),
            deleteShapes(true)
    {
    }

    ~Private() {
        if (deleteShapes) {
            qDeleteAll(shapes);
        }
    }

    KoShapeControllerBase *shapesDocument;
    QList<KoShape*> shapes;
    KoShapeContainer *explicitParentShape;
    bool deleteShapes;

    std::vector<std::unique_ptr<KUndo2Command>> reorderingCommands;
};

KoShapeCreateCommand::KoShapeCreateCommand(KoShapeControllerBase *controller, KoShape *shape, KoShapeContainer *parentShape, KUndo2Command *parent)
    : KoShapeCreateCommand(controller, QList<KoShape *>() << shape, parentShape, parent)
{
}

KoShapeCreateCommand::KoShapeCreateCommand(KoShapeControllerBase *controller, const QList<KoShape *> shapes, KoShapeContainer *parentShape, KUndo2Command *parent)
        : KoShapeCreateCommand(controller, shapes, parentShape, parent, kundo2_i18np("Create shape", "Create %1 shapes", shapes.size()))
{
}

KoShapeCreateCommand::KoShapeCreateCommand(KoShapeControllerBase *controller, const QList<KoShape *> shapes, KoShapeContainer *parentShape, KUndo2Command *parent, const KUndo2MagicString &undoString)
        : KUndo2Command(undoString, parent)
        , d(new Private(controller, shapes, parentShape))
{
}

KoShapeCreateCommand::~KoShapeCreateCommand()
{
    delete d;
}

void KoShapeCreateCommand::redo()
{
    KUndo2Command::redo();
    KIS_ASSERT(d->shapesDocument);

    d->deleteShapes = false;
    d->reorderingCommands.clear();

    Q_FOREACH(KoShape *shape, d->shapes) {
        if (d->explicitParentShape) {
            shape->setParent(d->explicitParentShape);
        }

        d->shapesDocument->addShape(shape);

        KoShapeContainer *shapeParent = shape->parent();

        KIS_SAFE_ASSERT_RECOVER_NOOP(shape->parent() ||
                                     dynamic_cast<KoShapeLayer*>(shape));

        if (shapeParent) {
            KUndo2Command *cmd = KoShapeReorderCommand::mergeInShape(shapeParent->shapes(), shape);

            if (cmd) {
                cmd->redo();
                d->reorderingCommands.push_back(
                    std::unique_ptr<KUndo2Command>(cmd));
            }
        }
    }
}

void KoShapeCreateCommand::undo()
{
    KUndo2Command::undo();
    KIS_ASSERT(d->shapesDocument);

    while (!d->reorderingCommands.empty()) {
        std::unique_ptr<KUndo2Command> cmd = std::move(d->reorderingCommands.back());
        cmd->undo();
        d->reorderingCommands.pop_back();
    }

    Q_FOREACH(KoShape *shape, d->shapes) {
        d->shapesDocument->removeShape(shape);
    }

    d->deleteShapes = true;
}
