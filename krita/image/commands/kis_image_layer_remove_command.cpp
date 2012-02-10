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
#include "kis_image.h"

#include <klocale.h>
#include "kis_layer.h"
#include "kis_clone_layer.h"
#include "kis_paint_layer.h"

KisImageLayerRemoveCommand::KisImageLayerRemoveCommand(KisImageWSP image, KisNodeSP node)
        : KisImageCommand(i18nc("(qtundo-format)", "Remove Layer"), image)
{
    m_node = node;
    m_prevParent = node->parent();
    m_prevAbove = node->prevSibling();
}

void KisImageLayerRemoveCommand::redo()
{
    processClones(m_node);
    UpdateTarget target(m_image, m_node, m_image->bounds());
    m_image->removeNode(m_node);
    target.update();
}

void KisImageLayerRemoveCommand::undo()
{
    m_image->addNode(m_node, m_prevParent, m_prevAbove);
    restoreClones();
    m_node->setDirty(m_image->bounds());
}

void KisImageLayerRemoveCommand::restoreClones()
{
    while(!m_reincarnatedNodes.isEmpty()) {
        KisCloneLayerSP clone = m_clonesList.takeLast();
        KisNodeSP newNode = m_reincarnatedNodes.takeLast();

        m_image->addNode(clone, newNode->parent(), newNode);
        moveChildren(newNode, clone);
        m_image->removeNode(newNode);
    }

    Q_ASSERT(m_clonesList.isEmpty());
    Q_ASSERT(m_reincarnatedNodes.isEmpty());
}

void KisImageLayerRemoveCommand::processClones(KisNodeSP node)
{
    KisLayerSP layer = dynamic_cast<KisLayer*>(node.data());
    if(!layer || !layer->hasClones()) return;

    foreach(KisCloneLayerWSP _clone, layer->registeredClones()) {
        KisCloneLayerSP clone = _clone;
        Q_ASSERT(clone);

        KisNodeSP newNode = clone->reincarnateAsPaintLayer();

        m_image->addNode(newNode, clone->parent(), clone);
        moveChildren(clone, newNode);
        m_image->removeNode(clone);

        m_clonesList.append(clone);
        m_reincarnatedNodes.append(newNode);
    }
}

void KisImageLayerRemoveCommand::moveChildren(KisNodeSP src, KisNodeSP dst)
{
    KisNodeSP child = src->firstChild();
    while(child) {
        m_image->moveNode(child, dst, dst->lastChild());
        child = child->nextSibling();
    }
}
