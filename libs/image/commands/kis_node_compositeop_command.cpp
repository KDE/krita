/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <klocalizedstring.h>
#include <KoCompositeOp.h>
#include "kis_node.h"
#include "commands/kis_node_commands.h"
#include "kis_command_ids.h"

KisNodeCompositeOpCommand::KisNodeCompositeOpCommand(KisNodeSP node, const QString& oldCompositeOp,
        const QString& newCompositeOp) :
        KisNodeCommand(kundo2_i18n("Composition Mode Change"), node)
{
    m_oldCompositeOp = oldCompositeOp;
    m_newCompositeOp = newCompositeOp;
}

void KisNodeCompositeOpCommand::setCompositeOpImpl(const QString &compositeOp)
{
    /**
     * The node might have had "Destination Atop" blending
     * that changes extent of the layer
     */
    const QRect oldExtent = m_node->extent();
    m_node->setCompositeOpId(compositeOp);
    m_node->setDirty(oldExtent | m_node->extent());
}

void KisNodeCompositeOpCommand::redo()
{
    setCompositeOpImpl(m_newCompositeOp);
}

void KisNodeCompositeOpCommand::undo()
{
    setCompositeOpImpl(m_oldCompositeOp);
}

int KisNodeCompositeOpCommand::id() const
{
    return KisCommandUtils::ChangeNodeCompositeOpId;
}

bool KisNodeCompositeOpCommand::mergeWith(const KUndo2Command *command)
{
    const KisNodeCompositeOpCommand *other =
        dynamic_cast<const KisNodeCompositeOpCommand*>(command);

    if (other && other->m_node == m_node) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_newCompositeOp == other->m_oldCompositeOp);
        m_newCompositeOp = other->m_newCompositeOp;
        return true;
    }

    return false;
}

bool KisNodeCompositeOpCommand::canMergeWith(const KUndo2Command *command) const
{
    const KisNodeCompositeOpCommand *other =
        dynamic_cast<const KisNodeCompositeOpCommand*>(command);

    return other && other->m_node == m_node;
}
