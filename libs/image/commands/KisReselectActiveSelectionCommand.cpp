/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
