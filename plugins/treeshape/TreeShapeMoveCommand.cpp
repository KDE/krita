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
                                           QUndoCommand *parent)
        : QUndoCommand(parent),
        m_newParent(newParent),
        m_nextShape(nextShape)
{
    foreach(KoShape *shape, shapes) {
        TreeShape *tree = dynamic_cast<TreeShape*>(shape);
        TreeShape *par = dynamic_cast<TreeShape*>(shape->parent());
        m_trees.append(tree);
        m_oldNextShapes.append(tree->nextShape());
        m_parents.append(par);
        m_connectors.append(par->connector(shape));
    }
    setText(i18n("Attach tree"));
}

TreeShapeMoveCommand::~TreeShapeMoveCommand()
{
}

void TreeShapeMoveCommand::redo()
{
    QUndoCommand::redo();
    Q_ASSERT(!m_trees.isEmpty());
    for (int i = 0; i < m_trees.count(); i++) {
        m_trees[i]->setNextShape(m_nextShape);
        m_newParent->addChild(m_trees[i], m_connectors[i]);
    }
}

void TreeShapeMoveCommand::undo()
{
    QUndoCommand::undo();
    for (int i = 0; i < m_trees.count(); i++) {
        m_trees[i]->setNextShape(m_oldNextShapes[i]);
        m_parents[i]->addChild(m_trees[i], m_connectors[i]);
    }
}