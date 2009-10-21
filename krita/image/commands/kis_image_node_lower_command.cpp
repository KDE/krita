/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_image_node_lower_command.h"
#include <QString>

#include <klocale.h>

#include "kis_image.h"
#include "kis_undo_adapter.h"

KisImageNodeLowerCommand::KisImageNodeLowerCommand(KisImageWSP image, KisNodeSP node)
        : KisImageCommand(i18n("Lower"), image), m_node(node)
{
}

void KisImageNodeLowerCommand::redo()
{
    m_image->lowerNode(m_node);
    m_node->setDirty();
}

void KisImageNodeLowerCommand::undo()
{
    m_image->raiseNode(m_node);
    m_node->setDirty();
}
