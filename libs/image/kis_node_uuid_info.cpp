/*
 *  Clone info stores information about clone layer's target
 *  SPDX-FileCopyrightText: 2011 Torio Mlshi <mlshi@lavabit.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
