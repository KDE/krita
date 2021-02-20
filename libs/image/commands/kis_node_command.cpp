/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "commands/kis_node_command.h"

#include "kis_node.h"

KisNodeCommand::KisNodeCommand(const KUndo2MagicString& name, KisNodeSP node)
    : KUndo2Command(name), m_node(node)
{
}

KisNodeCommand::~KisNodeCommand()
{
}
