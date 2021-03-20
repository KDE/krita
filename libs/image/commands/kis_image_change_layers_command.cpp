/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_commands.h"
#include "kis_image.h"
#include "kis_group_layer.h"

#include <klocalizedstring.h>


KisImageChangeLayersCommand::KisImageChangeLayersCommand(KisImageWSP image, KisNodeSP oldRootLayer, KisNodeSP newRootLayer)
    : KisImageCommand(kundo2_noi18n("change-layer-command"), image)
{
    m_oldRootLayer = oldRootLayer;
    m_newRootLayer = newRootLayer;
}

void KisImageChangeLayersCommand::redo()
{
    KisImageSP image = m_image.toStrongRef();
    if (image) {
        image->setRootLayer(static_cast<KisGroupLayer*>(m_newRootLayer.data()));
        image->refreshGraphAsync();
        image->notifyLayersChanged();
    }
}

void KisImageChangeLayersCommand::undo()
{
    KisImageSP image = m_image.toStrongRef();
    if (image) {
        image->setRootLayer(static_cast<KisGroupLayer*>(m_oldRootLayer.data()));
        image->refreshGraphAsync();
        image->notifyLayersChanged();
    }
}
