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

#ifndef TREECHANGECONNECTIONCOMMAND_H
#define TREECHANGECONNECTIONCOMMAND_H

#include "TreeShape.h"
#include <kundo2command.h>

/// The undo / redo command for configuring a connections between root and children
class TreeChangeConnectionCommand : public KUndo2Command
{
public:
    /**
     * Configures a tree shape
     * @param tree the tree shape to configure
     * @param structure the tree structure
     * @param followParent if tree will follow parent's structure
     * @param parent the optional parent command
     */
    TreeChangeConnectionCommand(TreeShape *tree, KoConnectionShape::Type type, KUndo2Command *parent=0);
    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();
private:
    TreeShape *m_tree;

    KoConnectionShape::Type m_oldType, m_newType;
};

#endif // TREECHANGECONNECTIONCOMMAND_H

