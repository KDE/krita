/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISBATCHUPDATELAYERMODIFICATIONCOMMAND_H
#define KISBATCHUPDATELAYERMODIFICATIONCOMMAND_H

#include <vector>

#include <QSharedPointer>

#include <kis_types.h>

#include "kis_command_utils.h"
#include "kis_image_command.h"

class KisBatchUpdateLayerModificationCommand : public KisCommandUtils::FlipFlopCommand
{
public:
    struct NodeTask {
        KisNodeSP node;
        bool doRedoUpdates = true;
        bool doUndoUpdates = true;
    };

    struct Recipe {
        QVector<NodeTask> nodesToAdd;
        QVector<NodeTask> nodesToRemove;

        std::vector<KisImageCommand::UpdateTarget> addedNodesUpdateTargets;
        std::vector<KisImageCommand::UpdateTarget> removedNodesUpdateTargets;
    };

    using RecipeSP = QSharedPointer<Recipe>;

public:
    KisBatchUpdateLayerModificationCommand(KisImageWSP image,
                                           RecipeSP recipe,
                                           KisCommandUtils::FlipFlopCommand::State state,
                                           KUndo2Command *parent = 0);

    void redo() override;
    void undo() override;

private:
    KisImageWSP m_image;
    RecipeSP m_recipe;
    KisCommandUtils::FlipFlopCommand::State m_state;
};

#endif // KISBATCHUPDATELAYERMODIFICATIONCOMMAND_H
