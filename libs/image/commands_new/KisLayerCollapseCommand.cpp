/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisLayerCollapseCommand.h"

#include <kis_node.h>


KisLayerCollapseCommand::KisLayerCollapseCommand(KisNodeSP node, bool oldValue, bool newValue, KUndo2Command *parent)
    : KUndo2Command(newValue ? kundo2_i18n("Collapse node %1", node->name()) : kundo2_i18n("Expand node %1", node->name()), parent)
    , m_node(node)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
{
}

KisLayerCollapseCommand::KisLayerCollapseCommand(KisNodeSP node, bool newValue, KUndo2Command *parent)
    : KisLayerCollapseCommand(node, node->collapsed(), newValue, parent)
{
}

void KisLayerCollapseCommand::redo()
{
    m_node->setCollapsed(m_newValue);
}

void KisLayerCollapseCommand::undo()
{
    m_node->setCollapsed(m_oldValue);
}

bool KisLayerCollapseCommand::mergeWith(const KUndo2Command *other)
{
    const KisLayerCollapseCommand *cmd = dynamic_cast<const KisLayerCollapseCommand*>(other);

    if (cmd && cmd->m_node == m_node) {
        // verify the commands are actually chained
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_newValue == cmd->m_oldValue, false);

        m_newValue = cmd->m_newValue;
        return true;
    }

    return false;
}
