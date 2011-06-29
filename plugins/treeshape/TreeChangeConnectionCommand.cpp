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

#include "TreeChangeConnectionCommand.h"
#include <klocale.h>

TreeChangeConnectionCommand::TreeChangeConnectionCommand(TreeShape *tree, KoConnectionShape::Type type, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_tree(tree)
    , m_newType(type)
{
    Q_ASSERT(m_tree);

    setText(i18nc("(qtundo-format)", "Change tree"));

    m_oldType = m_tree->connectionType();
}

void TreeChangeConnectionCommand::redo()
{
    KUndo2Command::redo();

    m_tree->update();

    if (m_oldType != m_newType)
        m_tree->setConnectionType(m_newType);

    m_tree->update();
}

void TreeChangeConnectionCommand::undo()
{
    KUndo2Command::undo();

    m_tree->update();

    if (m_oldType != m_newType)
        m_tree->setConnectionType(m_oldType);

    m_tree->update();
}
