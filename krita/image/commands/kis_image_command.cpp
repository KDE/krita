/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_image_commands.h"
#include <klocalizedstring.h>

#include "kis_image.h"
#include "kis_layer.h"



KisImageCommand::KisImageCommand(const KUndo2MagicString& name, KisImageWSP image, KUndo2Command *parent)
    : KUndo2Command(name, parent)
    , m_image(image)
{
}

KisImageCommand::~KisImageCommand()
{
}

static inline bool isLayer(KisNodeSP node) {
    return qobject_cast<KisLayer*>(node.data());
}

KisImageCommand::UpdateTarget::UpdateTarget(KisImageWSP image,
                                            KisNodeSP removedNode,
                                            const QRect &updateRect)
    : m_image(image), m_updateRect(updateRect)
{
    /**
     * We are saving an index, but not shared pointer, because the
     * target node may suddenly reincarnate into another type of a
     * layer during the removal process
     */
    m_removedNodeParent = removedNode->parent();
    m_removedNodeIndex = m_removedNodeParent ? m_removedNodeParent->index(removedNode) : -1;
}

void KisImageCommand::UpdateTarget::update() {
    if (!m_removedNodeParent) return;
    KIS_ASSERT_RECOVER_RETURN(m_removedNodeIndex >= 0);

    KisNodeSP node;
    int index = m_removedNodeIndex;

    while ((node = m_removedNodeParent->at(index)) && !isLayer(node)) {
        index++;
    }

    if (!node) {
        index = qMax(0, m_removedNodeIndex - 1);

        while ((node = m_removedNodeParent->at(index)) && !isLayer(node)) {
            index--;
        }
    }

    if (node) {
        node->setDirty(m_updateRect);
    } else {
        m_image->refreshGraphAsync(m_removedNodeParent);
        m_removedNodeParent->setDirty(m_updateRect);
    }
}
