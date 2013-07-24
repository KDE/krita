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

#include "kis_kranim_frame_saver.h"

KisKranimFrameSaver::KisKranimFrameSaver(KoStore *store, KisNode *node, QString name, QRect index)
    : KisNodeVisitor()
    , m_index(index)
    , m_name(name)
    , m_store(store)
    , m_writer(new KisStorePaintDeviceWriter(store))
{
}

KisKranimFrameSaver::~KisKranimFrameSaver(){
    delete m_writer;
}

bool KisKranimFrameSaver::visit(KisPaintLayer *layer){
    return true;
}
