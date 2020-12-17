/*
 *  Clone info stores information about clone layer's target
 *  SPDX-FileCopyrightText: 2011 Torio Mlshi <mlshi@lavabit.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
