/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_KRANIM_FRAME_SAVER_H
#define KIS_KRANIM_FRAME_SAVER_H

#include <KoStore.h>
#include <kis_node_visitor.h>
#include <kis_types.h>
#include <kis_store_paintdevice_writer.h>

class KisKranimFrameSaver : public KisNodeVisitor
{
public:
    KisKranimFrameSaver(KoStore* store, KisNode* node, QString name, QRect index);
    virtual ~KisKranimFrameSaver();
    using KisNodeVisitor::visit;

public:

    bool visit(KisPaintLayer *layer);

    bool visit(KisNode*){
        return true;
    }

private:
    KoStore* m_store;
    QString m_name;
    QRect m_index;
    KisStorePaintDeviceWriter* m_writer;
};

#endif // KIS_KRANIM_FRAME_SAVER_H
