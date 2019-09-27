/*
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_undo_adapter.h"
#include "kis_adjustment_layer.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "lazybrush/kis_colorize_mask.h"
#include "kis_external_layer_iface.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter.h"
#include "kis_generator.h"
#include "kis_generator_registry.h"
#include "generator/kis_generator_layer.h"
#include "kis_time_range.h"
#include <kundo2command.h>

KisColorSpaceConvertVisitor::KisColorSpaceConvertVisitor(KisImageWSP image,
                                                         const KoColorSpace *srcColorSpace,
                                                         const KoColorSpace *dstColorSpace,
                                                         KoColorConversionTransformation::Intent renderingIntent,
                                                         KoColorConversionTransformation::ConversionFlags conversionFlags)
    : KisNodeVisitor()
    , m_image(image)
    , m_srcColorSpace(srcColorSpace)
    , m_dstColorSpace(dstColorSpace)
    , m_renderingIntent(renderingIntent)
    , m_conversionFlags(conversionFlags)
{
}

KisColorSpaceConvertVisitor::~KisColorSpaceConvertVisitor()
{
}

bool KisColorSpaceConvertVisitor::visit(KisGroupLayer * layer)
{
    convertPaintDevice(layer);
    KisLayerSP child = qobject_cast<KisLayer*>(layer->firstChild().data());
    while (child) {
        child->accept(*this);
        child = qobject_cast<KisLayer*>(child->nextSibling().data());
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
    layer->resetCache();
    return true;
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
        layer->setFilter(f->defaultConfiguration());
    }

    layer->resetCache();
    return true;
}

bool KisColorSpaceConvertVisitor::convertPaintDevice(KisLayer* layer)
{

    if (*m_dstColorSpace == *layer->colorSpace()) return true;

    bool alphaLock = false;

    if (m_srcColorSpace->colorModelId() != m_dstColorSpace->colorModelId()) {
        layer->setChannelFlags(m_emptyChannelFlags);
        KisPaintLayer *paintLayer = 0;
        if ((paintLayer = dynamic_cast<KisPaintLayer*>(layer))) {
            alphaLock = paintLayer->alphaLocked();
            paintLayer->setChannelLockFlags(QBitArray());
        }
    }

    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return false;
    }

    KUndo2Command *parentConversionCommand = new KUndo2Command();

    if (layer->original()) {
        layer->original()->convertTo(m_dstColorSpace, m_renderingIntent, m_conversionFlags, parentConversionCommand);
    }

    if (layer->paintDevice()) {
        layer->paintDevice()->convertTo(m_dstColorSpace, m_renderingIntent, m_conversionFlags, parentConversionCommand);
    }

    if (layer->projection()) {
        layer->projection()->convertTo(m_dstColorSpace, m_renderingIntent, m_conversionFlags, parentConversionCommand);
    }

    image->undoAdapter()->addCommand(parentConversionCommand);

    KisPaintLayer *paintLayer = 0;
    if ((paintLayer = dynamic_cast<KisPaintLayer*>(layer))) {
        paintLayer->setAlphaLocked(alphaLock);
    }
    layer->setDirty();
    layer->invalidateFrames(KisFrameSet::infiniteFrom(0), layer->extent());

    return true;

}

bool KisColorSpaceConvertVisitor::visit(KisColorizeMask *mask)
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return false;
    }
    KUndo2Command* cmd = mask->setColorSpace(m_dstColorSpace, m_renderingIntent, m_conversionFlags);
    if (cmd) {
        image->undoAdapter()->addCommand(cmd);
    }
    return true;
}
