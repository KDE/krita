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

KisImageLayerRemoveCommand::KisImageLayerRemoveCommand(KisImageWSP image, KisNodeSP layer)
        : KisImageCommand(i18n("Remove Layer"), image)
{
    m_layer = layer;
    m_prevParent = layer->parent();
    m_prevAbove = layer->prevSibling();
}


void KisImageLayerRemoveCommand::redo()
{
    KisNodeSP parentNode = m_layer->parent();
    QRect extent = m_image->bounds();
    m_image->removeNode(m_layer);
    parentNode->setDirty(extent);
}

void KisImageLayerRemoveCommand::undo()
{
    m_image->addNode(m_layer, m_prevParent, m_prevAbove);
    m_layer->setDirty();
}
