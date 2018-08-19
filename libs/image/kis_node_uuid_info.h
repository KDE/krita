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

#ifndef kis_node_uuid_info_H
#define kis_node_uuid_info_H

#include <QUuid>
#include <QString>
#include "kritaimage_export.h"
#include "kis_node.h"

class KRITAIMAGE_EXPORT KisNodeUuidInfo
{

public:
    KisNodeUuidInfo();
    KisNodeUuidInfo(const QUuid& uuid);
    KisNodeUuidInfo(const QString& name);
    KisNodeUuidInfo(KisNodeSP node);
    
public:
    QUuid uuid()
    {
        return m_uuid;
    }
    
    QString name()
    {
        return m_name;
    }
    
public:
    KisNodeSP findNode(KisNodeSP rootNode);
    
private:
    bool check(KisNodeSP node);
    
private:
    QUuid m_uuid;
    QString m_name;
};

#endif // kis_node_uuid_info_H
