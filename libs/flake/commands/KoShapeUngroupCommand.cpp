/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeUngroupCommand.h"
#include "KoShapeContainer.h"
#include "KoShapeReorderCommand.h"

#include <klocalizedstring.h>
#include "kis_assert.h"


struct KoShapeUngroupCommand::Private
{
    Private(KoShapeContainer *_container,
            const QList<KoShape *> &_shapes,
            const QList<KoShape*> &_topLevelShapes)
        : container(_container),
          shapes(_shapes),
          topLevelShapes(_topLevelShapes)
    {
        std::stable_sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
        std::sort(topLevelShapes.begin(), topLevelShapes.end(), KoShape::compareShapeZIndex);
    }


    KoShapeContainer *container;
    QList<KoShape*> shapes;
    QList<KoShape*> topLevelShapes;
    QScopedPointer<KUndo2Command> shapesReorderCommand;

};

KoShapeUngroupCommand::KoShapeUngroupCommand(KoShapeContainer *container, const QList<KoShape *> &shapes,
                                             const QList<KoShape*> &topLevelShapes, KUndo2Command *parent)
    : KUndo2Command(parent),
      m_d(new Private(container, shapes, topLevelShapes))
{
    setText(kundo2_i18n("Ungroup shapes"));
}

KoShapeUngroupCommand::~KoShapeUngroupCommand()
{
}

void KoShapeUngroupCommand::redo()
{
    using IndexedShape = KoShapeReorderCommand::IndexedShape;

    KoShapeContainer *newParent = m_d->container->parent();

    QList<IndexedShape> indexedSiblings;
    QList<KoShape*> perspectiveSiblings;

    if (newParent) {
        perspectiveSiblings = newParent->shapes();
        std::sort(perspectiveSiblings.begin(), perspectiveSiblings.end(), KoShape::compareShapeZIndex);
    } else {
        perspectiveSiblings = m_d->topLevelShapes;
    }

    Q_FOREACH (KoShape *shape, perspectiveSiblings) {
        indexedSiblings.append(shape);
    }

    // find the place where the ungrouped shapes should be inserted
    // (right on the top of their current container)
    auto insertIt = std::upper_bound(indexedSiblings.begin(),
                                     indexedSiblings.end(),
                                     IndexedShape(m_d->container));

    std::copy(m_d->shapes.begin(), m_d->shapes.end(),
              std::inserter(indexedSiblings, insertIt));

    indexedSiblings = KoShapeReorderCommand::homogenizeZIndexesLazy(indexedSiblings);

    const QTransform ungroupTransform = m_d->container->absoluteTransformation();
    for (auto it = m_d->shapes.begin(); it != m_d->shapes.end(); ++it) {
        KoShape *shape = *it;
        KIS_SAFE_ASSERT_RECOVER(shape->parent() == m_d->container) { continue; }

        shape->setParent(newParent);
        shape->applyAbsoluteTransformation(ungroupTransform);
    }

    if (!indexedSiblings.isEmpty()) {
        m_d->shapesReorderCommand.reset(new KoShapeReorderCommand(indexedSiblings));
        m_d->shapesReorderCommand->redo();
    }
}

void KoShapeUngroupCommand::undo()
{
    const QTransform groupTransform = m_d->container->absoluteTransformation().inverted();
    for (auto it = m_d->shapes.begin(); it != m_d->shapes.end(); ++it) {
        KoShape *shape = *it;

        shape->setParent(m_d->container);
        shape->applyAbsoluteTransformation(groupTransform);
    }

    if (m_d->shapesReorderCommand) {
        m_d->shapesReorderCommand->undo();
        m_d->shapesReorderCommand.reset();
    }
}
