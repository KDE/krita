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
#include "kis_transaction.h"
#include "kis_image.h"
#include "kis_undo_adapter.h"
#include "commands/kis_layer_props_command.h"

KisColorSpaceConvertVisitor::KisColorSpaceConvertVisitor(KisImageWSP image,
        const KoColorSpace *dstColorSpace,
        KoColorConversionTransformation::Intent renderingIntent)
        : KisNodeVisitor()
        , m_image(image)
        , m_dstColorSpace(dstColorSpace)
        , m_renderingIntent(renderingIntent)
{
}

KisColorSpaceConvertVisitor::~KisColorSpaceConvertVisitor()
{
}

bool KisColorSpaceConvertVisitor::visit(KisGroupLayer * layer)
{
    convertPaintDevice(layer);
    KisLayerSP child = dynamic_cast<KisLayer*>(layer->firstChild().data());
    while (child) {
        child->accept(*this);
        child = dynamic_cast<KisLayer*>(child->nextSibling().data());
    }

    layer->resetCache();

    return true;
}

bool KisColorSpaceConvertVisitor::visit(KisPaintLayer *layer)
{
    return convertPaintDevice(layer);
}

bool KisColorSpaceConvertVisitor::visit(KisGeneratorLayer *layer)
{
    return convertPaintDevice(layer);
}

bool KisColorSpaceConvertVisitor::visit(KisAdjustmentLayer * layer)
{
    // XXX: Make undoable!
    if (layer->filter()->name() == "perchannel") {
        // Per-channel filters need to be reset because of different number
        // of channels. This makes undo very tricky, but so be it.
        // XXX: Make this more generic for after 1.6, when we'll have many
        // channel-specific filters.
        KisFilterSP f = KisFilterRegistry::instance()->value("perchannel");
        layer->setFilter(f->defaultConfiguration(0));
    }

    KisLayerPropsCommand* propsCommand = new KisLayerPropsCommand(layer,
            layer->opacity(), layer->opacity(),
            layer->compositeOpId(), layer->compositeOpId(),
            layer->name(), layer->name(),
            layer->channelFlags(), m_emptyChannelFlags);
    m_image->undoAdapter()->addCommand(propsCommand);

    layer->resetCache();
    return true;
}

bool KisColorSpaceConvertVisitor::visit(KisExternalLayer *layer)
{
    Q_UNUSED(layer)
    return true;
}

bool KisColorSpaceConvertVisitor::convertPaintDevice(KisLayer* layer)
{

    KisLayerPropsCommand* propsCommand = new KisLayerPropsCommand(layer,
            layer->opacity(), layer->opacity(),
            layer->compositeOpId(), layer->compositeOpId(),
            layer->name(), layer->name(),
            layer->channelFlags(), m_emptyChannelFlags);

    if (m_image && m_image->undoAdapter()) {
        m_image->undoAdapter()->addCommand(propsCommand);
    }

    if (layer->original()) {
        QUndoCommand* cmd = layer->original()->convertTo(m_dstColorSpace, m_renderingIntent);
        if (m_image->undoAdapter()) {
            if (cmd) {
                m_image->undoAdapter()->addCommand(cmd);
            }
        }
        else {
            delete cmd;
        }
    }

    if (layer->paintDevice()) {
        QUndoCommand* cmd = layer->paintDevice()->convertTo(m_dstColorSpace, m_renderingIntent);
        if (m_image->undoAdapter()) {
            if (cmd) {
                m_image->undoAdapter()->addCommand(cmd);
            }
        }
        else {
            delete cmd;
        }    }

    if (layer->projection()) {
        QUndoCommand* cmd = layer->projection()->convertTo(m_dstColorSpace, m_renderingIntent);
        if (m_image->undoAdapter()) {
            if (cmd) {
                m_image->undoAdapter()->addCommand(cmd);
            }
        }
        else {
            delete cmd;
        }    }

    layer->setDirty();

    return true;

}
