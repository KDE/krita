/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_NODE_COMMAND_H_
#define KIS_NODE_COMMAND_H_

#include <kritaimage_export.h>
#include <kundo2command.h>
#include "kis_types.h"

class KisNode;

/// the base command for commands altering a node
class KRITAIMAGE_EXPORT KisNodeCommand : public KUndo2Command
{

public:
    /**
     * Constructor
     * @param name The name that will be shown in the ui
     * @param node The node the command will be working on.
     */
    KisNodeCommand(const KUndo2MagicString& name, KisNodeSP node);
    ~KisNodeCommand() override;

protected:
    KisNodeSP m_node;
};

#endif /* KIS_NODE_COMMAND_H_*/
