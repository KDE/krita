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


KisImageLayerAddCommand::KisImageLayerAddCommand(KisImageWSP image, KisNodeSP layer, KisNodeSP parent, KisNodeSP aboveThis)
        : KisImageCommand(i18nc("(qtundo-format)", "Add Layer"), image), m_index(-1)
{
    m_layer = layer;
    m_parent = parent;
    m_aboveThis = aboveThis;
}

KisImageLayerAddCommand::KisImageLayerAddCommand(KisImageWSP image, KisNodeSP layer, KisNodeSP parent, quint32 index)
        : KisImageCommand(i18nc("(qtundo-format)", "Add Layer"), image), m_index(index)
{
    m_layer = layer;
    m_parent = parent;
    m_aboveThis = 0;
}

void KisImageLayerAddCommand::redo()
{
    if (m_aboveThis || m_index == quint32(-1)) {
        m_image->addNode(m_layer, m_parent, m_aboveThis);
    } else {
        m_image->addNode(m_layer, m_parent, m_index);
    }

    m_layer->setDirty(m_image->bounds());
}

void KisImageLayerAddCommand::undo()
{
    UpdateTarget target(m_image, m_layer, m_image->bounds());
    m_image->removeNode(m_layer);
    target.update();
}
