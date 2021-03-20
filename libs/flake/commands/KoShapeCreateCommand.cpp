/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

#include <kis_undo_stores.h>
#include <KoAddRemoveShapeCommands.h>

class Q_DECL_HIDDEN KoShapeCreateCommand::Private
{
public:
    Private(KoShapeControllerBase *_document, const QList<KoShape*> &_shapes, KoShapeContainer *_parentShape)
            : shapesDocument(_document),
            shapes(_shapes),
            explicitParentShape(_parentShape)
    {
    }

    KoShapeControllerBase *shapesDocument;
    QList<KoShape*> shapes;
    KoShapeContainer *explicitParentShape;

    KisSurrogateUndoStore undoStore;
    bool firstRedo = true;

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
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->explicitParentShape);

    if (d->firstRedo) {
        Q_FOREACH(KoShape *shape, d->shapes) {

            d->undoStore.addCommand(new KoAddShapeCommand(shape, d->explicitParentShape));

            KoShapeContainer *shapeParent = shape->parent();
            KIS_SAFE_ASSERT_RECOVER_NOOP(shape->parent() ||
                                         dynamic_cast<KoShapeLayer*>(shape));

            if (shapeParent) {
                d->undoStore.addCommand(KoShapeReorderCommand::mergeInShape(shapeParent->shapes(), shape));
            }
        }
        d->firstRedo = false;
    } else {
        d->undoStore.redoAll();
    }
}

void KoShapeCreateCommand::undo()
{
    d->undoStore.undoAll();
    KUndo2Command::undo();
}
