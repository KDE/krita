/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        KisImageSP image = m_image.toStrongRef();
        if (image) {
            image->refreshGraphAsync(m_removedNodeParent);
            m_removedNodeParent->setDirty(m_updateRect);
        }
    }
}
