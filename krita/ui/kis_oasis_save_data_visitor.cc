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

#include <kis_oasis_save_data_visitor.h>

#include <QImage>

#include <KoOasisStore.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>

#include "kis_adjustment_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"

KisOasisSaveDataVisitor::KisOasisSaveDataVisitor(KoOasisStore* os, KoXmlWriter* manifestWriter) : m_oasisStore(os), m_manifestWriter(manifestWriter)
{}

bool KisOasisSaveDataVisitor::visit(KisPaintLayer *layer)
{
    QString filename = "data/" + layer->name() + ".png";
    QImage img = layer->paintDevice()->convertToQImage(0);
    if( m_oasisStore->store()->open(filename))
    {
        KoStoreDevice io ( m_oasisStore->store() );
        if ( !io.open( QIODevice::WriteOnly ) )
        {
            kdDebug() << "Couldn't open for writing: " << filename << endl;
            return false;
        }
        if ( ! img.save( &io, "PNG", 0 ) )
        {
            kdDebug() << "Saving PNG failed: " << filename << endl;
            return false;
        }
        io.close();
        if(m_oasisStore->store()->close())
        {
            m_manifestWriter->addManifestEntry( filename, "" );
        } else {
            kdDebug() << "Closing of data file failed: " << filename << endl;
            return false;
        }
    } else {
        kdDebug() << "Opening of data file failed :" << filename << endl;
        return false;
    }
    kdDebug() << "Successfull saving of layer: " << layer->name() << endl;
    return true;
}

bool KisOasisSaveDataVisitor::visit(KisGroupLayer *layer)
{
    KisLayerSP child = layer->firstChild();
    while(child)
    {
        child->accept(*this);
        child = child->nextSibling();
    }
    return true;
}
bool KisOasisSaveDataVisitor::visit(KisPartLayer *layer)
{
    Q_UNUSED(layer);
    kDebug() << "not supported by OpenRaster" << endl;
    return false;
}
bool KisOasisSaveDataVisitor::visit(KisAdjustmentLayer *layer)
{
    Q_UNUSED(layer);
    return true;
}
