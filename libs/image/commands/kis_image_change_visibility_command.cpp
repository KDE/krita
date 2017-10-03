/*
 *  Copyright (c) 2017 Nikita Smirnov <pakrentos@gmail.com>
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
#include "kis_node.h"

#include <klocalizedstring.h>


KisImageChangeVisibilityCommand::KisImageChangeVisibilityCommand(bool visibility, KisNodeSP node)
    : KUndo2Command(kundo2_noi18n("change-visibility-command"), 0)
{
    m_node = node;
    m_visible = visibility;
}

void KisImageChangeVisibilityCommand::redo()
{
    m_node->setVisible(m_visible);
}

void KisImageChangeVisibilityCommand::undo()
{
    m_node->setVisible(!m_visible);
}
