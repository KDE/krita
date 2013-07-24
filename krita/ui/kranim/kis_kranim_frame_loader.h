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

#ifndef KIS_KRANIM_FRAME_LOADER_H
#define KIS_KRANIM_FRAME_LOADER_H

#include <KoStore.h>
#include <kis_node_visitor.h>
#include <kis_types.h>
#include <kis_image.h>

class KisKranimFrameLoader : public KisNodeVisitor
{
public:
    KisKranimFrameLoader(KisImageWSP image, KoStore* store, QRect index);
    virtual ~KisKranimFrameLoader();

public:
    bool visit(KisPaintLayer *layer);

    bool visit(KisNode*){
        return true;
    }

private:
    KisImageWSP m_image;
    KoStore* m_store;
    QRect m_index;
};

#endif // KIS_KRANIM_FRAME_LOADER_H
