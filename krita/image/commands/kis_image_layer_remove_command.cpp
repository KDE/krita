/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_image_layer_remove_command.h"

#include <klocale.h>
#include "kis_image.h"
#include "kis_image_layer_remove_command_impl.h"


KisImageLayerRemoveCommand::KisImageLayerRemoveCommand(KisImageWSP image, KisNodeSP node)
    : KisImageCommand(i18nc("(qtundo-format)", "Remove Layer"), image),
      m_node(node)
{
    addSubtree(image, node);
}

KisImageLayerRemoveCommand::~KisImageLayerRemoveCommand()
{
}

void KisImageLayerRemoveCommand::addSubtree(KisImageWSP image, KisNodeSP node)
{
    // Simple tail-recursion to remove nodes in bottom-up way
    //
    // Alert: the nodes must be traversed in last-to-first order,
    //        because each KisImageLayerRemoveCommandImpl stores a
    //        pointer to the previous node of the stack

    KisNodeSP child = node->lastChild();
    while (child) {
        addSubtree(image, child);
        child = child->prevSibling();
    }

    new KisImageLayerRemoveCommandImpl(image, node, this);
}

void KisImageLayerRemoveCommand::redo()
{
    UpdateTarget target(m_image, m_node, m_image->bounds());
    KisImageCommand::redo();
    target.update();
}

void KisImageLayerRemoveCommand::undo()
{
    KisImageCommand::undo();
    m_node->setDirty(m_image->bounds());
}
