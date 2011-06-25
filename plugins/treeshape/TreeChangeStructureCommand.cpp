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

#include "TreeChangeStructureCommand.h"
#include <klocale.h>

TreeChangeStructureCommand::TreeChangeStructureCommand(TreeShape *tree, TreeShape::TreeType structure, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_tree(tree)
    , m_newStructure(structure)
{
    Q_ASSERT(m_tree);

    setText(i18nc("(qtundo-format)", "Change tree"));

    m_oldStructure = m_tree->structure();
    m_oldFollowParent = false;
}

void TreeChangeStructureCommand::redo()
{
    KUndo2Command::redo();

    m_tree->update();

    if (m_oldStructure != m_newStructure)
        m_tree->setStructure(m_newStructure);

    m_tree->update();
}

void TreeChangeStructureCommand::undo()
{
    KUndo2Command::undo();

    m_tree->update();

    if (m_oldStructure != m_newStructure)
        m_tree->setStructure(m_oldStructure);

    m_tree->update();
}
