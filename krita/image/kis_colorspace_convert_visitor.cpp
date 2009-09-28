/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#include "kis_colorspace_convert_visitor.h"


KisColorSpaceConvertVisitor::KisColorSpaceConvertVisitor(const KoColorSpace *dstColorSpace,
                                                         KoColorConversionTransformation::Intent renderingIntent) :
        KisNodeVisitor(),
        m_dstColorSpace(dstColorSpace),
        m_renderingIntent(renderingIntent)
{
}

KisColorSpaceConvertVisitor::~KisColorSpaceConvertVisitor()
{
}

bool KisColorSpaceConvertVisitor::visit(KisGroupLayer * layer)
{
    // Clear the projection, we will have to re-render everything.
    // The image is already set to the new colorspace, so this'll work.
    layer->setColorSpace(m_dstColorSpace, m_renderingIntent);
    layer->resetCache();
    layer->setChannelFlags(m_emptyChannelFlags);
    KisLayerSP child = dynamic_cast<KisLayer*>(layer->firstChild().data());
    while (child) {
        child->accept(*this);
        child = dynamic_cast<KisLayer*>(child->nextSibling().data());
    }
    return true;
}

bool KisColorSpaceConvertVisitor::visit(KisPaintLayer *layer)
{
    layer->paintDevice()->convertTo(m_dstColorSpace, m_renderingIntent);
    layer->setChannelFlags(m_emptyChannelFlags);
    Q_ASSERT(!layer->temporaryTarget() );
    return true;
}

bool KisColorSpaceConvertVisitor::visit(KisGeneratorLayer *layer)
{
    layer->paintDevice()->convertTo(m_dstColorSpace, m_renderingIntent);
    layer->setChannelFlags(m_emptyChannelFlags);
    return true;
}



bool KisColorSpaceConvertVisitor::visit(KisAdjustmentLayer * layer)
{
    if (layer->filter()->name() == "perchannel") {
        // Per-channel filters need to be reset because of different number
        // of channels. This makes undo very tricky, but so be it.
        // XXX: Make this more generic for after 1.6, when we'll have many
        // channel-specific filters.
        KisFilterSP f = KisFilterRegistry::instance()->value("perchannel");
        layer->setFilter(f->defaultConfiguration(0));
    }
    layer->setChannelFlags(m_emptyChannelFlags);
    layer->resetCache();
    return true;
}

bool KisColorSpaceConvertVisitor::visit(KisExternalLayer *layer) 
{
    Q_UNUSED(layer)
    return true;
}

