/*
 *  Clone info stores information about clone layer's target
 *  Copyright (C) 2011 Torio Mlshi <mlshi@lavabit.com>
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "kis_node_uuid_info.h"

#include "kis_debug.h"

KisNodeUuidInfo::KisNodeUuidInfo()
{
}

KisNodeUuidInfo::KisNodeUuidInfo(const QUuid& uuid)
{
    m_uuid = uuid;
}

KisNodeUuidInfo::KisNodeUuidInfo(const QString& name)
{
    m_name = name;
}

KisNodeUuidInfo::KisNodeUuidInfo(KisNodeSP node)
{
    m_uuid = node->uuid();
    m_name = node->name();
}

KisNodeSP KisNodeUuidInfo::findNode(KisNodeSP rootNode)
{
    if (check(rootNode))
        return rootNode;
    
    KisNodeSP child = rootNode->firstChild();
    KisNodeSP node = 0;
    while (child && !node)
    {
        node = findNode(child);
        child = child->nextSibling();
    }
    return node;
}

bool KisNodeUuidInfo::check(KisNodeSP node)
{
    if (m_uuid == node->uuid())                     // every node has valid uuid
        return true;
    if (m_uuid.isNull() && m_name == node->name())  // but some may have empty names
        return true;
    return false;
}
