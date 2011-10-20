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

#ifndef KIS_CLONE_INFO_H
#define KIS_CLONE_INFO_H

#include <QUuid>
#include <QString>
#include "kis_node.h"

class KRITAIMAGE_EXPORT KisCloneInfo
{

public:
    KisCloneInfo();
    KisCloneInfo(const QUuid& uuid);
    KisCloneInfo(const QString& name);
    KisCloneInfo(KisNodeSP node);
    
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

#endif // KIS_CLONE_INFO_H
