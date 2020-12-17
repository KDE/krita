/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_convert_color_space_processing_visitor.h"

#include "kis_external_layer_iface.h"

#include "kis_paint_device.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_transform_mask.h"
#include "lazybrush/kis_colorize_mask.h"

#include <KoColorConversionTransformation.h>
#include "kis_projection_leaf.h"
#include "kis_paint_layer.h"
#include "kis_time_span.h"
#include <commands_new/KisChangeChannelFlagsCommand.h>
#include <commands_new/KisChangeChannelLockFlagsCommand.h>


KisConvertColorSpaceProcessingVisitor::KisConvertColorSpaceProcessingVisitor(const KoColorSpace *srcColorSpace,
                                                                             const KoColorSpace *dstColorSpace,
                                                                             KoColorConversionTransformation::Intent renderingIntent,
                                                                             KoColorConversionTransformation::ConversionFlags conversionFlags)
    : m_srcColorSpace(srcColorSpace)
    , m_dstColorSpace(dstColorSpace)
    , m_renderingIntent(renderingIntent)
    , m_conversionFlags(conversionFlags)

{
}

void KisConvertColorSpaceProcessingVisitor::visitExternalLayer(KisExternalLayer *layer, KisUndoAdapter *undoAdapter)
{
    undoAdapter->addCommand(layer->convertTo(m_dstColorSpace, m_renderingIntent, m_conversionFlags));
}

void KisConvertColorSpaceProcessingVisitor::visitNodeWithPaintDevice(KisNode *node, KisUndoAdapter *undoAdapter)
{
    if (!node->projectionLeaf()->isLayer()) return;
    if (*m_dstColorSpace == *node->colorSpace()) return;

    bool alphaLock = false;
    bool alphaDisabled = false;

    KisLayer *layer = dynamic_cast<KisLayer*>(node);

    KisPaintLayer *paintLayer = 0;

    KUndo2Command *parentConversionCommand = new KUndo2Command();

    if (m_srcColorSpace->colorModelId() != m_dstColorSpace->colorModelId()) {
        alphaDisabled = layer->alphaChannelDisabled();
        new KisChangeChannelFlagsCommand(QBitArray(), layer, parentConversionCommand);
        if ((paintLayer = dynamic_cast<KisPaintLayer*>(layer))) {
            alphaLock = paintLayer->alphaLocked();
            new KisChangeChannelLockFlagsCommand(QBitArray(), paintLayer, parentConversionCommand);
        }
    }


    if (layer->original()) {
        layer->original()->convertTo(m_dstColorSpace, m_renderingIntent, m_conversionFlags, parentConversionCommand);
    }

    if (layer->paintDevice()) {
        layer->paintDevice()->convertTo(m_dstColorSpace, m_renderingIntent, m_conversionFlags, parentConversionCommand);
    }

    if (layer->projection()) {
        layer->projection()->convertTo(m_dstColorSpace, m_renderingIntent, m_conversionFlags, parentConversionCommand);
    }

    if (layer && alphaDisabled) {
        new KisChangeChannelFlagsCommand(m_dstColorSpace->channelFlags(true, false),
                                         layer, parentConversionCommand);
    }

    if (paintLayer && alphaLock) {
        new KisChangeChannelLockFlagsCommand(m_dstColorSpace->channelFlags(true, false),
                                             paintLayer, parentConversionCommand);
    }

    undoAdapter->addCommand(parentConversionCommand);
    layer->invalidateFrames(KisTimeSpan::infinite(0), layer->extent());
}

void KisConvertColorSpaceProcessingVisitor::visit(KisTransformMask *node, KisUndoAdapter *undoAdapter)
{
    node->threadSafeForceStaticImageUpdate();
    KisSimpleProcessingVisitor::visit(node, undoAdapter);
}

void KisConvertColorSpaceProcessingVisitor::visitColorizeMask(KisColorizeMask *node, KisUndoAdapter *undoAdapter)
{
    undoAdapter->addCommand(node->setColorSpace(m_dstColorSpace, m_renderingIntent, m_conversionFlags));
    node->invalidateFrames(KisTimeSpan::infinite(0), node->extent());
}
