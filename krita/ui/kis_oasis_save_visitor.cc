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

#include <kis_oasis_save_visitor.h>

#include <QImage>

#include <KoOasisStore.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>

#include "kis_adjustment_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"

KisOasisSaveVisitor::KisOasisSaveVisitor(KoOasisStore* os) : m_oasisStore(os), m_bodyWriter( m_oasisStore->bodyWriter())
{}

void KisOasisSaveVisitor::saveLayerInfo(KisLayer* layer)
{
    m_bodyWriter->addAttribute("name", layer->name());
    m_bodyWriter->addAttribute("x", layer->x());
    m_bodyWriter->addAttribute("y", layer->y());
}

bool KisOasisSaveVisitor::visit(KisPaintLayer *layer)
{
    QString filename = "data/" + layer->name() + ".png";
    m_bodyWriter->startElement("image:layer");
    saveLayerInfo(layer);
    m_bodyWriter->addAttribute("src", filename);
    m_bodyWriter->endElement();
    return true;
}

bool KisOasisSaveVisitor::visit(KisGroupLayer *layer)
{
    m_bodyWriter->startElement("image:layer");
    saveLayerInfo(layer);
    
    KisLayerSP child = layer->firstChild();

    while(child)
    {
        child->accept(*this);
        child = child->nextSibling();
    }

    m_bodyWriter->endElement();
    return true;
}
bool KisOasisSaveVisitor::visit(KisPartLayer *layer)
{
    Q_UNUSED(layer);
    kDebug() << "not supported by OpenRaster" << endl;
    return false;
}
bool KisOasisSaveVisitor::visit(KisAdjustmentLayer *layer)
{
    saveLayerInfo(layer);
    return true;
}
