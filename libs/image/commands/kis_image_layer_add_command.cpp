/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_commands.h"
#include "kis_image.h"

#include <klocalizedstring.h>


KisImageLayerAddCommand::KisImageLayerAddCommand(KisImageWSP image,
                                                 KisNodeSP layer,
                                                 KisNodeSP parent,
                                                 KisNodeSP aboveThis,
                                                 bool doRedoUpdates,
                                                 bool doUndoUpdates)
        : KisImageCommand(kundo2_i18n("Add Layer"), image),
          m_index(-1),
          m_doRedoUpdates(doRedoUpdates),
          m_doUndoUpdates(doUndoUpdates),
          m_additionFlags(KisNodeAdditionFlag::None)
{
    m_layer = layer;
    m_parent = parent;
    m_aboveThis = aboveThis;
}

KisImageLayerAddCommand::KisImageLayerAddCommand(KisImageWSP image,
                                                 KisNodeSP layer,
                                                 KisNodeSP parent,
                                                 quint32 index,
                                                 bool doRedoUpdates,
                                                 bool doUndoUpdates)
        : KisImageCommand(kundo2_i18n("Add Layer"), image),
          m_index(index),
          m_doRedoUpdates(doRedoUpdates),
          m_doUndoUpdates(doUndoUpdates),
          m_additionFlags(KisNodeAdditionFlag::None)
{
    m_layer = layer;
    m_parent = parent;
    m_aboveThis = 0;
}

KisImageLayerAddCommand::KisImageLayerAddCommand(KisImageWSP image,
                                                 KisNodeSP layer,
                                                 KisNodeSP parent,
                                                 KisNodeSP aboveThis,
                                                 Flags flags)
        : KisImageLayerAddCommand(image, layer, parent, aboveThis,
                                  flags.testFlag(DoRedoUpdates),
                                  flags.testFlag(DoUndoUpdates))
{
    m_additionFlags.setFlag(KisNodeAdditionFlag::DontActivateNode,
                            flags.testFlag(DontActivateOnAddition));
}

KisImageLayerAddCommand::KisImageLayerAddCommand(KisImageWSP image, KisNodeSP layer, KisNodeSP parent, quint32 index, Flags flags)
    : KisImageLayerAddCommand(image, layer, parent, index,
                              flags.testFlag(DoRedoUpdates),
                              flags.testFlag(DoUndoUpdates))
{
    m_additionFlags.setFlag(KisNodeAdditionFlag::DontActivateNode,
                            flags.testFlag(DontActivateOnAddition));
}

void KisImageLayerAddCommand::redo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    if (m_aboveThis || m_index == quint32(-1)) {
        image->addNode(m_layer, m_parent, m_aboveThis, m_additionFlags);
    } else {
        image->addNode(m_layer, m_parent, m_index, m_additionFlags);
    }

    if (m_doRedoUpdates) {
        m_layer->setDirty(image->bounds());
    }
}

void KisImageLayerAddCommand::undo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    if (m_doUndoUpdates) {
        UpdateTarget target(image, m_layer, image->bounds());
        image->removeNode(m_layer);
        target.update();
    } else {
        image->removeNode(m_layer);
    }
}
