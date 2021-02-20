/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_layer_remove_command.h"

#include <klocalizedstring.h>
#include "kis_image.h"
#include "kis_image_layer_remove_command_impl.h"


KisImageLayerRemoveCommand::KisImageLayerRemoveCommand(KisImageWSP image,
                                                       KisNodeSP node,
                                                       bool doRedoUpdates,
                                                       bool doUndoUpdates)
    : KisImageCommand(kundo2_i18n("Remove Layer"), image),
      m_node(node),
      m_doRedoUpdates(doRedoUpdates),
      m_doUndoUpdates(doUndoUpdates)
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
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    UpdateTarget target(image, m_node, image->bounds());
    KisImageCommand::redo();

    if (m_doRedoUpdates) {
        target.update();
    }
}

void KisImageLayerRemoveCommand::undo()
{
    KisImageCommand::undo();

    if (m_doUndoUpdates) {
        /**
         * We are removing the group recursively, so the updates should
         * come recursively as well
         */
        KisImageSP image = m_image.toStrongRef();
        if (!image) {
            return;
        }
        image->refreshGraphAsync(m_node, image->bounds());
    }
}
