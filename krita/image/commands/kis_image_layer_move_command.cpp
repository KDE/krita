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

#include <klocale.h>

#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"


#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_undo_adapter.h"


KisImageLayerMoveCommand::KisImageLayerMoveCommand(KisImageWSP image, KisNodeSP layer, KisNodeSP newParent, KisNodeSP newAbove)
        : KisImageCommand(i18n("Move Layer"), image)
{
    m_layer = layer;
    m_newParent = newParent;
    m_newAbove = newAbove;
    m_prevParent = layer->parent();
    m_prevAbove = layer->prevSibling();
    m_index = -1;
}

KisImageLayerMoveCommand::KisImageLayerMoveCommand(KisImageWSP image, KisNodeSP node, KisNodeSP newParent, quint32 index)
        : KisImageCommand(i18n("Move Layer"), image)
{
    m_layer = node;
    m_newParent = newParent;
    m_newAbove = 0;
    m_prevParent = node->parent();
    m_prevAbove = node->prevSibling();
    m_index = index;
}

void KisImageLayerMoveCommand::redo()
{
    if (m_newAbove || m_index == quint32(-1)) {
        m_image->moveNode(m_layer, m_newParent, m_newAbove);
    } else {
        m_image->moveNode(m_layer, m_newParent, m_index);
    }
    m_layer->setDirty();
}

void KisImageLayerMoveCommand::undo()
{
    m_image->moveNode(m_layer, m_prevParent, m_prevAbove);
    m_layer->setDirty();
}
