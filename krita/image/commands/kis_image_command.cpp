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
#include <klocale.h>

#include "kis_image.h"
#include "kis_layer.h"


KisImageCommand::KisImageCommand(const QString& name, KisImageWSP image)
        : KUndo2Command(name)
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
    m_needsFullRefresh = false;

    if(!isLayer(removedNode)) {
        m_node = removedNode->parent();
    }
    else {
        m_node = removedNode;
        while((m_node = m_node->nextSibling()) && !isLayer(m_node));

        if(!m_node) {
            m_node = removedNode;
            while((m_node = m_node->prevSibling()) && !isLayer(m_node));
        }

        if(!m_node) {
            m_node = removedNode->parent();
            m_needsFullRefresh = true;
        }
    }
}

void KisImageCommand::UpdateTarget::update() {
    if (m_needsFullRefresh) {
        m_image->refreshGraph(m_node);
        m_node->setDirty(m_updateRect);
    }
    else {
        m_node->setDirty(m_updateRect);
    }
}
