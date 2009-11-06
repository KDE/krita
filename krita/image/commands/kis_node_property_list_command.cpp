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

#include <klocale.h>
#include "kis_node.h"
#include "commands/kis_node_property_list_command.h"


KisNodePropertyListCommand::KisNodePropertyListCommand(KisNodeSP node, KoDocumentSectionModel::PropertyList newPropertyList)
    : KisNodeCommand(i18n("Property Changes"), node),
      m_newPropertyList(newPropertyList),
      m_oldPropertyList(node->sectionModelProperties())
/**
 * TODO instead of "Property Changes" check which property
 * has been changed and display either lock/unlock, visible/hidden
 * or "Property Changes" (this require new strings)
 */
{
}

void KisNodePropertyListCommand::redo()
{
    m_node->setSectionModelProperties(m_newPropertyList);
    m_node->setDirty(); // TODO check if visibility was changed or not
}

void KisNodePropertyListCommand::undo()
{
    m_node->setSectionModelProperties(m_oldPropertyList);
    m_node->setDirty(); // TODO check if visibility was changed or not
}
