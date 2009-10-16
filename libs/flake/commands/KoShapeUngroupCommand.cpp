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
#include "KoShapeContainer.h"

#include <klocale.h>

KoShapeUngroupCommand::KoShapeUngroupCommand(KoShapeContainer *container, const QList<KoShape *> &shapes,
                                             const QList<KoShape*> &topLevelShapes, QUndoCommand *parent)
        : KoShapeGroupCommand(parent)
{
    QList<KoShape*> orderdShapes(shapes);
    qSort(orderdShapes.begin(), orderdShapes.end(), KoShape::compareShapeZIndex);
    m_shapes = orderdShapes;
    m_container = container;

    QList<KoShape*> ancestors = m_container->parent()? m_container->parent()->childShapes(): topLevelShapes;
    qSort(ancestors.begin(), ancestors.end(), KoShape::compareShapeZIndex);
    QList<KoShape*>::const_iterator it(qFind(ancestors, m_container));

    Q_ASSERT(it != ancestors.constEnd());
    for ( ; it != ancestors.constEnd(); ++it ) {
        m_oldAncestorsZIndex.append(QPair<KoShape*, int>(*it, (*it)->zIndex()));
    }

    int zIndex = m_container->zIndex();
    foreach(KoShape *shape, m_shapes) {
        m_clipped.append(m_container->childClipped(shape));
        m_oldParents.append(m_container->parent());
        m_oldClipped.append(m_container->childClipped(shape));
        // TODO this might also need to change the childs of the parent but that is very problematic if the parent is 0
        m_oldZIndex.append(zIndex++);
    }

    setText(i18n("Ungroup shapes"));
}

void KoShapeUngroupCommand::redo()
{
    KoShapeGroupCommand::undo();
    int zIndex = m_container->zIndex() + m_oldZIndex.count() - 1;
    for (QList<QPair<KoShape*, int> >::const_iterator it( m_oldAncestorsZIndex.constBegin()); it != m_oldAncestorsZIndex.constEnd(); ++it) {
        it->first->setZIndex(zIndex++);
    }
}

void KoShapeUngroupCommand::undo()
{
    KoShapeGroupCommand::redo();
    for (QList<QPair<KoShape*, int> >::const_iterator it( m_oldAncestorsZIndex.constBegin()); it != m_oldAncestorsZIndex.constEnd(); ++it) {
        it->first->setZIndex(it->second);
    }
}
