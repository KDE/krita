/* This file is part of the KDE project

   Copyright (c) 2010 Cyril Oblikov <munknex@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "TreeShapeMoveCommand.h"
#include "TreeShape.h"
#include "KoShapeContainer.h"

#include <klocale.h>

TreeShapeMoveCommand::TreeShapeMoveCommand(const QList<KoShape*> &shapes,
                                           TreeShape *newParent,
                                           KoShape *nextShape,
                                           QPointF diff,
                                           KUndo2Command *parent)
        : KUndo2Command(parent),
        m_newParent(newParent),
        m_nextShape(nextShape),
        m_diff(diff)
{
    foreach(KoShape *shape, shapes) {
        TreeShape *tree = dynamic_cast<TreeShape*>(shape);
        TreeShape *par = dynamic_cast<TreeShape*>(shape->parent());
        m_trees.append(tree);
        m_oldNextShapes.append(tree->nextShape());
        m_parents.append(par);
        m_connectors.append(par ? par->connector(shape) : 0);
    }
    setText(i18nc("(qtundo-format)", "Attach tree"));
}

TreeShapeMoveCommand::~TreeShapeMoveCommand()
{
}

void TreeShapeMoveCommand::redo()
{
    KUndo2Command::redo();
    Q_ASSERT(!m_trees.isEmpty());
    for (int i = 0; i < m_trees.count(); i++) {
        TreeShape *tree = m_trees[i];
        tree->setNextShape(m_nextShape);
        tree->setParent(0);
        if (m_newParent) {
            // TODO: if tree was without parent we should create a connector
            // for it. Otherwise it will cause a crash.
            m_newParent->addChild(tree, m_connectors[i]);
        } else {
            tree->setPosition(tree->position()+m_diff);
        }
    }
}

void TreeShapeMoveCommand::undo()
{
    KUndo2Command::undo();
    for (int i = 0; i < m_trees.count(); i++) {
        TreeShape *tree = m_trees[i];
        tree->setNextShape(m_oldNextShapes[i]);
        tree->setParent(0);
        if (m_parents[i]) {
            m_parents[i]->addChild(tree, m_connectors[i]);
        } else {
            tree->setPosition(tree->position()-m_diff);
        }
    }
}