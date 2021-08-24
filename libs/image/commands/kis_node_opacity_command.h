/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_NODE_OPACITY_COMMAND_H
#define KIS_NODE_OPACITY_COMMAND_H

#include "kis_node_command.h"
#include "commands_new/KisAsynchronouslyMergeableCommandInterface.h"
#include "boost/optional.hpp"

/// The command for setting the node opacity
class KRITAIMAGE_EXPORT KisNodeOpacityCommand : public KisNodeCommand, public KisAsynchronouslyMergeableCommandInterface
{

public:
    /**
     * Constructor
     * @param node The node the command will be working on.
     * @param oldOpacity the old node opacity
     * @param newOpacity the new node opacity
     */
    KisNodeOpacityCommand(KisNodeSP node, quint8 newOpacity);

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *command) override;
    bool canMergeWith(const KUndo2Command *command) const override;
    bool canAnnihilateWith(const KUndo2Command *command) const override;

private:
    boost::optional<quint8> m_oldOpacity;
    quint8 m_newOpacity;
};

#endif /* KIS_NODE_OPACITY_COMMAND_H */
