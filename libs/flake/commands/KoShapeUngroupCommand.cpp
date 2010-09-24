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

#include "KoShapeUngroupCommand.h"
#include "KoShapeGroupCommand_p.h"
#include "KoShapeContainer.h"

#include <klocale.h>

KoShapeUngroupCommand::KoShapeUngroupCommand(KoShapeContainer *container, const QList<KoShape *> &shapes,
        const QList<KoShape*> &topLevelShapes, QUndoCommand *parent)
    : KoShapeGroupCommand(*(new KoShapeGroupCommandPrivate(container, shapes)), parent)
{
    QList<KoShape*> orderdShapes(shapes);
    qSort(orderdShapes.begin(), orderdShapes.end(), KoShape::compareShapeZIndex);
    d->shapes = orderdShapes;

    QList<KoShape*> ancestors = d->container->parent()? d->container->parent()->shapes(): topLevelShapes;
    if (ancestors.count()) {
        qSort(ancestors.begin(), ancestors.end(), KoShape::compareShapeZIndex);
        QList<KoShape*>::const_iterator it(qFind(ancestors, d->container));

        Q_ASSERT(it != ancestors.constEnd());
        for (; it != ancestors.constEnd(); ++it) {
            d->oldAncestorsZIndex.append(QPair<KoShape*, int>(*it, (*it)->zIndex()));
        }
    }

    int zIndex = d->container->zIndex();
    foreach(KoShape *shape, d->shapes) {
        d->clipped.append(d->container->isClipped(shape));
        d->oldParents.append(d->container->parent());
        d->oldClipped.append(d->container->isClipped(shape));
        d->oldInheritTransform.append(shape->parent() && shape->parent()->inheritsTransform(shape));
        d->inheritTransform.append(false);
        // TODO this might also need to change the children of the parent but that is very problematic if the parent is 0
        d->oldZIndex.append(zIndex++);
    }

    setText(i18n("Ungroup shapes"));
}

void KoShapeUngroupCommand::redo()
{
    KoShapeGroupCommand::undo();
    if (d->oldAncestorsZIndex.count()) {
        int zIndex = d->container->zIndex() + d->oldZIndex.count() - 1;
        for (QList<QPair<KoShape*, int> >::const_iterator it(d->oldAncestorsZIndex.constBegin()); it != d->oldAncestorsZIndex.constEnd(); ++it) {
            it->first->setZIndex(zIndex++);
        }
    }
}

void KoShapeUngroupCommand::undo()
{
    KoShapeGroupCommand::redo();
    if (d->oldAncestorsZIndex.count()) {
        for (QList<QPair<KoShape*, int> >::const_iterator it(d->oldAncestorsZIndex.constBegin()); it != d->oldAncestorsZIndex.constEnd(); ++it) {
            it->first->setZIndex(it->second);
        }
    }
}
