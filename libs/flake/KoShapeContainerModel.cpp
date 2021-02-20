/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeContainerModel.h"

#include "KoShapeContainer.h"

#include "kis_assert.h"

KoShapeContainerModel::KoShapeContainerModel()
{
}

KoShapeContainerModel::~KoShapeContainerModel()
{
}

void KoShapeContainerModel::deleteOwnedShapes()
{
    QList<KoShape*> ownedShapes = this->shapes();

    Q_FOREACH (KoShape *shape, ownedShapes) {
        shape->setParent(0);
        delete shape;
    }

    KIS_SAFE_ASSERT_RECOVER_NOOP(!this->count());
}

void KoShapeContainerModel::proposeMove(KoShape *child, QPointF &move)
{
    Q_UNUSED(child);
    Q_UNUSED(move);
}

void KoShapeContainerModel::childChanged(KoShape *child, KoShape::ChangeType type)
{
    Q_UNUSED(type);
    KoShapeContainer * parent = child->parent();
    Q_ASSERT(parent);
    // propagate the change up the hierarchy
    KoShapeContainer * grandparent = parent->parent();
    if (grandparent) {
        grandparent->model()->childChanged(parent, KoShape::ChildChanged);
    }
}

void KoShapeContainerModel::shapeHasBeenAddedToHierarchy(KoShape *shape, KoShapeContainer *addedToSubtree)
{
    KoShapeContainer *parent = addedToSubtree->parent();
    if (parent) {
        parent->model()->shapeHasBeenAddedToHierarchy(shape, parent);
    }
}

void KoShapeContainerModel::shapeToBeRemovedFromHierarchy(KoShape *shape, KoShapeContainer *removedFromSubtree)
{
    KoShapeContainer *parent = removedFromSubtree->parent();
    if (parent) {
        parent->model()->shapeToBeRemovedFromHierarchy(shape, parent);
    }
}

KoShapeContainerModel::KoShapeContainerModel(const KoShapeContainerModel &rhs)
{
    Q_UNUSED(rhs);
}
