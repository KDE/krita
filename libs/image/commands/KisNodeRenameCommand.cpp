/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisNodeRenameCommand.h"

#include <klocalizedstring.h>
#include "kis_node.h"
#include "commands/kis_node_commands.h"


KisNodeRenameCommand::KisNodeRenameCommand(KisNodeSP node, const QString &oldName, const QString &newName)
    : KisNodeCommand(kundo2_i18n("Node Rename"), node)
{
    m_oldName = oldName;
    m_newName = newName;
}

void KisNodeRenameCommand::redo()
{
    m_node->setName(m_newName);
}

void KisNodeRenameCommand::undo()
{
    m_node->setName(m_oldName);
}
