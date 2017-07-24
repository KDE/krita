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

#include "kis_assert.h"
#include <KoShapeLayer.h>
#include <KoShapeReorderCommand.h>

#include <vector>
#include <memory>

class Q_DECL_HIDDEN KoShapeCreateCommand::Private
{
public:
    Private(KoShapeBasedDocumentBase *_document, const QList<KoShape*> &_shapes)
            : shapesDocument(_document),
            shapes(_shapes),
            deleteShapes(true)
    {
        Q_FOREACH(KoShape *shape, shapes) {
            originalShapeParents << shape->parent();
        }
    }

    ~Private() {
        if (deleteShapes) {
            qDeleteAll(shapes);
        }
    }

    KoShapeBasedDocumentBase *shapesDocument;
    QList<KoShape*> shapes;
    QList<KoShapeContainer*> originalShapeParents;
    bool deleteShapes;

    std::vector<std::unique_ptr<KUndo2Command>> reorderingCommands;

    QScopedPointer<KUndo2Command> reorderingCommand;
};

KoShapeCreateCommand::KoShapeCreateCommand(KoShapeBasedDocumentBase *controller, KoShape *shape, KUndo2Command *parent)
    : KoShapeCreateCommand(controller, QList<KoShape *>() << shape, parent)
{
}

KoShapeCreateCommand::KoShapeCreateCommand(KoShapeBasedDocumentBase *controller, const QList<KoShape *> shapes, KUndo2Command *parent)
        : KUndo2Command(kundo2_i18np("Create shape", "Create shapes", shapes.size()), parent),
        d(new Private(controller, shapes))
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
        d->shapesDocument->addShape(shape);

        KoShapeContainer *shapeParent = shape->parent();

        KIS_SAFE_ASSERT_RECOVER_NOOP(shape->parent() ||
                                     dynamic_cast<KoShapeLayer*>(shape));

        if (shapeParent) {
            KUndo2Command *cmd = KoShapeReorderCommand::mergeInShape(shapeParent->shapes(), shape);

            if (d->reorderingCommand) {
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

    KIS_SAFE_ASSERT_RECOVER_RETURN(d->shapes.size() == d->originalShapeParents.size());

    for (int i = 0; i < d->shapes.size(); i++) {
        d->shapesDocument->removeShape(d->shapes[i]);
        d->shapes[i]->setParent(d->originalShapeParents[i]);
    }

    d->deleteShapes = true;
}
