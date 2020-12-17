/*
 *  SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisNodeRenameCommand.h"

#include <klocalizedstring.h>
#include "kis_node.h"
#include "commands/kis_node_commands.h"
#include "kis_command_ids.h"

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

int KisNodeRenameCommand::id() const
{
    return KisCommandUtils::ChangeNodeNameId;
}

bool KisNodeRenameCommand::mergeWith(const KUndo2Command *command)
{
    const KisNodeRenameCommand *other =
        dynamic_cast<const KisNodeRenameCommand*>(command);

    if (other && other->m_node == m_node) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_newName == other->m_oldName);
        m_newName = other->m_newName;
        return true;
    }

    return false;
}

bool KisNodeRenameCommand::canMergeWith(const KUndo2Command *command) const
{
    const KisNodeRenameCommand *other =
        dynamic_cast<const KisNodeRenameCommand*>(command);

    return other && other->m_node == m_node;
}
