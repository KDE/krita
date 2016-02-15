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

    if (m_useIndex) {
        m_image->moveNode(m_layer, m_newParent, m_index);
    } else {
        m_image->moveNode(m_layer, m_newParent, m_newAbove);
    }

    if (m_doUpdates) {
        m_image->refreshGraphAsync(m_prevParent);
        if (m_newParent != m_prevParent) {
            m_layer->setDirty(m_image->bounds());
        }
    }
}

void KisImageLayerMoveCommand::undo()
{
    m_image->moveNode(m_layer, m_prevParent, m_prevAbove);

    if (m_doUpdates) {
        m_image->refreshGraphAsync(m_newParent);
        if (m_newParent != m_prevParent) {
            m_layer->setDirty(m_image->bounds());
        }
    }
}
