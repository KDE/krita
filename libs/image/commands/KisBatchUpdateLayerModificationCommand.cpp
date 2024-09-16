/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisBatchUpdateLayerModificationCommand.h"

#include <kis_image.h>
#include <kis_node.h>

KisBatchUpdateLayerModificationCommand::KisBatchUpdateLayerModificationCommand(KisImageWSP image,
                                                                               RecipeSP recipe,
                                                                               KisCommandUtils::FlipFlopCommand::State state,
                                                                               KUndo2Command *parent)
    : KisCommandUtils::FlipFlopCommand(state)
    , m_image(image)
    , m_recipe(recipe)
{
}

void KisBatchUpdateLayerModificationCommand::redo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }

    // TODO: reuse compression from the KisUpdateCommandEx and KisBatchNodeUpdate
    //       to avoid double updates

    if (getState() == INITIALIZING) {
        KIS_SAFE_ASSERT_RECOVER(m_recipe->removedNodesUpdateTargets.empty()) {
            m_recipe->removedNodesUpdateTargets.clear();
        }

        for (auto it = m_recipe->nodesToRemove.begin();
             it != m_recipe->nodesToRemove.end();
             ++it) {

            if (!it->doRedoUpdates) continue;
            m_recipe->removedNodesUpdateTargets.emplace_back(image, it->node, image->bounds());
        }
    } else {
        for (auto it = m_recipe->removedNodesUpdateTargets.begin();
             it != m_recipe->removedNodesUpdateTargets.end();
             ++it) {

            it->update();
        }
        m_recipe->removedNodesUpdateTargets.clear();

        for (auto it = m_recipe->nodesToAdd.begin();
             it != m_recipe->nodesToAdd.end();
             ++it) {

            if (!it->doRedoUpdates) continue;
            it->node->setDirty(image->bounds());
        }
    }
    KisCommandUtils::FlipFlopCommand::redo();
}

void KisBatchUpdateLayerModificationCommand::undo()
{
    KisCommandUtils::FlipFlopCommand::undo();

    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }

    if (getState() == FINALIZING) {
        KIS_SAFE_ASSERT_RECOVER(m_recipe->addedNodesUpdateTargets.empty()) {
            m_recipe->addedNodesUpdateTargets.clear();
        }

        for (auto it = m_recipe->nodesToAdd.begin();
             it != m_recipe->nodesToAdd.end();
             ++it) {

            if (!it->doUndoUpdates) continue;
            m_recipe->addedNodesUpdateTargets.emplace_back(image, it->node, image->bounds());
        }
    } else {
        for (auto it = m_recipe->addedNodesUpdateTargets.begin();
             it != m_recipe->addedNodesUpdateTargets.end();
             ++it) {

            it->update();
        }
        m_recipe->addedNodesUpdateTargets.clear();

        for (auto it = m_recipe->nodesToRemove.begin();
             it != m_recipe->nodesToRemove.end();
             ++it) {

            if (!it->doUndoUpdates) continue;
            image->refreshGraphAsync(it->node, image->bounds());
        }
    }
}
