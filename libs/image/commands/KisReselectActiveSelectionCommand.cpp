/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisReselectActiveSelectionCommand.h"

#include "kis_image.h"
#include "kis_node.h"
#include "kis_layer.h"
#include "kis_selection_mask.h"
#include <KoProperties.h>


KisReselectActiveSelectionCommand::KisReselectActiveSelectionCommand(KisNodeSP activeNode, KisImageWSP image, KUndo2Command *parent)
    : KisReselectGlobalSelectionCommand(image, parent),
      m_activeNode(activeNode)
{
}

void KisReselectActiveSelectionCommand::redo()
{
    bool shouldReselectFGlobalSelection = true;

    if (m_activeNode) {
        KisSelectionMaskSP mask = dynamic_cast<KisSelectionMask*>(m_activeNode.data());

        if (!mask) {

            KisLayerSP layer;
            KisNodeSP node = m_activeNode;
            while (node && !(layer = dynamic_cast<KisLayer*>(node.data()))) {
                node = node->parent();
            }

            if (layer && !layer->selectionMask()) {
                KoProperties properties;
                properties.setProperty("active", false);
                properties.setProperty("visible", true);
                QList<KisNodeSP> masks = layer->childNodes(QStringList("KisSelectionMask"), properties);

                if (!masks.isEmpty()) {
                    mask = dynamic_cast<KisSelectionMask*>(masks.first().data());
                }
            } else if (layer && layer->selectionMask()) {
                shouldReselectFGlobalSelection = false;
            }
        }

        if (mask) {
            mask->setActive(true);
            shouldReselectFGlobalSelection = false;
            m_reselectedMask = mask;
        }
    }

    if (shouldReselectFGlobalSelection) {
        KisReselectGlobalSelectionCommand::redo();
    }
}

void KisReselectActiveSelectionCommand::undo()
{
    if (m_reselectedMask) {
        m_reselectedMask->setActive(false);
        m_reselectedMask.clear();
    } else {
        KisReselectGlobalSelectionCommand::undo();
    }
}
