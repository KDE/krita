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

struct KisImageLayerRemoveCommand::Private {
    KisNodeSP node;
    KisNodeSP prevParent;
    KisNodeSP prevAbove;

    QList<KisCloneLayerSP> clonesList;
    QList<KisNodeSP> reincarnatedNodes;
};

KisImageLayerRemoveCommand::KisImageLayerRemoveCommand(KisImageWSP image, KisNodeSP node)
    : KisImageCommand(i18nc("(qtundo-format)", "Remove Layer"), image),
      m_d(new Private())
{
    m_d->node = node;
    m_d->prevParent = node->parent();
    m_d->prevAbove = node->prevSibling();
}

KisImageLayerRemoveCommand::~KisImageLayerRemoveCommand()
{
    delete m_d;
}

void KisImageLayerRemoveCommand::redo()
{
    processClones(m_d->node);
    UpdateTarget target(m_image, m_d->node, m_image->bounds());
    m_image->removeNode(m_d->node);
    target.update();
}

void KisImageLayerRemoveCommand::undo()
{
    m_image->addNode(m_d->node, m_d->prevParent, m_d->prevAbove);
    restoreClones();
    m_d->node->setDirty(m_image->bounds());
}

void KisImageLayerRemoveCommand::restoreClones()
{
    while(!m_d->reincarnatedNodes.isEmpty()) {
        KisCloneLayerSP clone = m_d->clonesList.takeLast();
        KisNodeSP newNode = m_d->reincarnatedNodes.takeLast();

        m_image->addNode(clone, newNode->parent(), newNode);
        moveChildren(newNode, clone);
        m_image->removeNode(newNode);
    }

    Q_ASSERT(m_d->clonesList.isEmpty());
    Q_ASSERT(m_d->reincarnatedNodes.isEmpty());
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

        m_d->clonesList.append(clone);
        m_d->reincarnatedNodes.append(newNode);
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
