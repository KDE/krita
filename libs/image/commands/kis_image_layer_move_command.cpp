/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_commands.h"
#include <QString>
#include <QBitArray>

#include <klocalizedstring.h>

#include "KoColor.h"
#include "KoColorProfile.h"


#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_undo_adapter.h"


KisImageLayerMoveCommand::KisImageLayerMoveCommand(KisImageWSP image, KisNodeSP layer, KisNodeSP newParent, KisNodeSP newAbove, bool doUpdates)
        : KisImageCommand(kundo2_i18n("Move Layer"), image)
{
    m_layer = layer;
    m_newParent = newParent;
    m_newAbove = newAbove;
    m_prevParent = layer->parent();
    m_prevAbove = layer->prevSibling();
    m_index = -1;
    m_useIndex = false;
    m_doUpdates = doUpdates;
}

KisImageLayerMoveCommand::KisImageLayerMoveCommand(KisImageWSP image, KisNodeSP node, KisNodeSP newParent, quint32 index)
        : KisImageCommand(kundo2_i18n("Move Layer"), image)
{
    m_layer = node;
    m_newParent = newParent;
    m_newAbove = 0;
    m_prevParent = node->parent();
    m_prevAbove = node->prevSibling();
    m_index = index;
    m_useIndex = true;
    m_doUpdates = true;
}

void KisImageLayerMoveCommand::redo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    if (m_useIndex) {
        image->moveNode(m_layer, m_newParent, m_index);
    } else {
        image->moveNode(m_layer, m_newParent, m_newAbove);
    }

    if (m_doUpdates) {
        image->refreshGraphAsync(m_prevParent);
        if (m_newParent != m_prevParent) {
            m_layer->setDirty(image->bounds());
        }
    }
}

void KisImageLayerMoveCommand::undo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    image->moveNode(m_layer, m_prevParent, m_prevAbove);

    if (m_doUpdates) {
        image->refreshGraphAsync(m_newParent);
        if (m_newParent != m_prevParent) {
            m_layer->setDirty(image->bounds());
        }
    }
}
