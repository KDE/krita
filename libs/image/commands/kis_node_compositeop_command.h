/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_NODE_COMPOSITEOP_COMMAND_H
#define KIS_NODE_COMPOSITEOP_COMMAND_H

#include "kis_node_command.h"
#include "commands_new/KisAsynchronouslyMergeableCommandInterface.h"

/// The command for setting the composite op
class KRITAIMAGE_EXPORT KisNodeCompositeOpCommand : public KisNodeCommand, public KisAsynchronouslyMergeableCommandInterface
{

public:
    /**
     * Constructor
     * @param node The node the command will be working on.
     * @param oldCompositeOp the old node composite op
     * @param newCompositeOp the new node composite op
     */
    KisNodeCompositeOpCommand(KisNodeSP node, const QString& oldCompositeOp, const QString& newCompositeOp);

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *command) override;
    bool canMergeWith(const KUndo2Command *command) const override;

private:
    void setCompositeOpImpl(const QString &compositeOp);

private:
    QString m_oldCompositeOp;
    QString m_newCompositeOp;
};

#endif /* KIS_NODE_COMPOSITEOP_COMMAND_H */
