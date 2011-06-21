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

#ifndef TREESHAPEMOVECOMMAND_H
#define TREESHAPEMOVECOMMAND_H

#include "KoSelection.h"

#include "flake_export.h"

#include <kundo2command.h>
#include <QList>
#include <QPointF>

class KoShape;
class TreeShape;
class KoShapeContainer;

/// The undo / redo command for shape moving.
class TreeShapeMoveCommand : public KUndo2Command
{
public:
    TreeShapeMoveCommand(const QList<KoShape*> &shapes,
                         TreeShape *newParent,
                         KoShape *nextShape,
                         QPointF diff,
                         KUndo2Command *parent = 0);
    ~TreeShapeMoveCommand();
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    QList<TreeShape*> m_trees;
    QList<TreeShape*> m_parents;
    TreeShape *m_newParent;
    KoShape *m_nextShape;
    QList<KoShape*> m_oldNextShapes;
    QList<KoShape*> m_connectors;
    QPointF m_diff;
};

#endif
