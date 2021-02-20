/*
 *  SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISNODERENAMECOMMAND_H
#define KISNODERENAMECOMMAND_H

#include "kis_node_command.h"
#include "commands_new/KisAsynchronouslyMergeableCommandInterface.h"

/// The command for setting the node's name
class KRITAIMAGE_EXPORT KisNodeRenameCommand : public KisNodeCommand, public KisAsynchronouslyMergeableCommandInterface
{
public:
    KisNodeRenameCommand(KisNodeSP node,
                         const QString &oldName,
                         const QString &newName);

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *command) override;
    bool canMergeWith(const KUndo2Command *command) const override;

private:

    QString m_oldName;
    QString m_newName;
};

#endif // KISNODERENAMECOMMAND_H
