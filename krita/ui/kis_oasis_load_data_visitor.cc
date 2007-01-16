/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include <kis_oasis_load_data_visitor.h>

#include <KoOasisStore.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

bool KisOasisLoadDataVisitor::visit(KisPaintLayer *layer)
{
    QString filename = m_layerFilenames[layer];
    kDebug(41008) << "Loading file : " << filename << endl;
    if (m_oasisStore->store()->open(filename) ) {
        KoStoreDevice io ( m_oasisStore->store() );
        if ( !io.open( QIODevice::ReadOnly ) )
        {
            kDebug(41008) << "Couldn't open for reading: " << filename << endl;
            return false;
        }
        QImage img;
        if ( ! img.load( &io, "PNG" ) )
        {
            kDebug(41008) << "Loading PNG failed: " << filename << endl;
            m_oasisStore->store()->close();
            io.close();
            return false;
        }
        layer->paintDevice()->convertFromQImage(img, "");
        img.save("testouille.png", "PNG", 0);
        io.close();
        m_oasisStore->store()->close();
        kDebug(41008) << "Loading was successful" << endl;
        return true;
    }
    kDebug(41008) << "Loading was unsuccessful" << endl;
    return false;
}

bool KisOasisLoadDataVisitor::visit(KisGroupLayer *layer)
{
    kDebug(41008) << "Visiting a group layer" << endl;
    KisLayerSP child = layer->firstChild();
    while(child)
    {
        child->accept(*this);
        child = child->nextSibling();
    }
    return true;
}


bool KisOasisLoadDataVisitor::visit(KisAdjustmentLayer *layer)
{
    Q_UNUSED(layer);
    return true;
}
