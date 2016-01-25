/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_resources_snapshot.h"

#include <KoColor.h>
#include <resources/KoAbstractGradient.h>
#include <KoCompositeOpRegistry.h>
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_settings.h>
#include <brushengine/kis_paintop_registry.h>
#include <kis_threaded_text_rendering_workaround.h>
#include <resources/KoPattern.h>
#include "kis_canvas_resource_provider.h"
#include "filter/kis_filter_configuration.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "recorder/kis_recorded_paint_action.h"
#include "kis_selection.h"
#include "kis_selection_mask.h"


struct KisResourcesSnapshot::Private {
    Private()
        : currentPattern(0)
        , currentGradient(0)
        , currentGenerator(0)
        , compositeOp(0)
    {
    }

    KisImageWSP image;
    KisDefaultBoundsBaseSP bounds;
    KisPostExecutionUndoAdapter *undoAdapter;
    KoColor currentFgColor;
    KoColor currentBgColor;
    KoPattern *currentPattern;
    KoAbstractGradient *currentGradient;
    KisPaintOpPresetSP currentPaintOpPreset;
    KisNodeSP currentNode;
    qreal currentExposure;
    KisFilterConfiguration *currentGenerator;

    QPointF axesCenter;
    bool mirrorMaskHorizontal;
    bool mirrorMaskVertical;

    quint8 opacity;
    QString compositeOpId;
    const KoCompositeOp *compositeOp;

    KisPainter::StrokeStyle strokeStyle;
    KisPainter::FillStyle fillStyle;

    bool globalAlphaLock;
    qreal effectiveZoom;
    bool presetAllowsLod;
};

KisResourcesSnapshot::KisResourcesSnapshot(KisImageWSP image, KisNodeSP currentNode, KisPostExecutionUndoAdapter *undoAdapter, KoCanvasResourceManager *resourceManager, KisDefaultBoundsBaseSP bounds)
    : m_d(new Private())
{
    m_d->image = image;
    if (!bounds) {
        bounds = new KisDefaultBounds(m_d->image);
    }
    m_d->bounds = bounds;
    m_d->undoAdapter = undoAdapter;

    m_d->currentFgColor = resourceManager->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
    m_d->currentBgColor = resourceManager->resource(KoCanvasResourceManager::BackgroundColor).value<KoColor>();
    m_d->currentPattern = resourceManager->resource(KisCanvasResourceProvider::CurrentPattern).value<KoPattern*>();
    m_d->currentGradient = resourceManager->resource(KisCanvasResourceProvider::CurrentGradient).value<KoAbstractGradient*>();
    m_d->currentPaintOpPreset = resourceManager->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
    KisPaintOpRegistry::instance()->preinitializePaintOpIfNeeded(m_d->currentPaintOpPreset);
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

    m_d->currentExposure = resourceManager->resource(KisCanvasResourceProvider::HdrExposure).toDouble();
    m_d->currentGenerator = resourceManager->resource(KisCanvasResourceProvider::CurrentGeneratorConfiguration).value<KisFilterConfiguration*>();

    m_d->axesCenter = resourceManager->resource(KisCanvasResourceProvider::MirrorAxesCenter).toPointF();
    if (m_d->axesCenter.isNull()){
        QRect bounds = m_d->bounds->bounds();
        m_d->axesCenter = QPointF(0.5 * bounds.width(), 0.5 * bounds.height());
    }

    m_d->mirrorMaskHorizontal = resourceManager->resource(KisCanvasResourceProvider::MirrorHorizontal).toBool();
    m_d->mirrorMaskVertical = resourceManager->resource(KisCanvasResourceProvider::MirrorVertical).toBool();


    qreal normOpacity = resourceManager->resource(KisCanvasResourceProvider::Opacity).toDouble();
    m_d->opacity = quint8(normOpacity * OPACITY_OPAQUE_U8);

    m_d->compositeOpId = resourceManager->resource(KisCanvasResourceProvider::CurrentCompositeOp).toString();
    setCurrentNode(currentNode);

    /**
     * Fill and Stroke styles are not a part of the resource manager
     * so the tools should set them manually
     * TODO: port stroke and fill styles to be a part
     *       of the resource manager
     */
    m_d->strokeStyle = KisPainter::StrokeStyleBrush;
    m_d->fillStyle = KisPainter::FillStyleNone;

    m_d->globalAlphaLock = resourceManager->resource(KisCanvasResourceProvider::GlobalAlphaLock).toBool();
    m_d->effectiveZoom = resourceManager->resource(KisCanvasResourceProvider::EffectiveZoom).toDouble();
    m_d->presetAllowsLod = resourceManager->resource(KisCanvasResourceProvider::PresetAllowsLod).toBool();
}

KisResourcesSnapshot::~KisResourcesSnapshot()
{
    delete m_d;
}

void KisResourcesSnapshot::setupPainter(KisPainter* painter)
{
    painter->setPaintColor(m_d->currentFgColor);
    painter->setBackgroundColor(m_d->currentBgColor);
    painter->setGenerator(m_d->currentGenerator);
    painter->setPattern(m_d->currentPattern);
    painter->setGradient(m_d->currentGradient);

    QBitArray lockflags = channelLockFlags();
    if (lockflags.size() > 0) {
        painter->setChannelFlags(lockflags);
    }

    painter->setOpacity(m_d->opacity);
    painter->setCompositeOp(m_d->compositeOp);
    painter->setMirrorInformation(m_d->axesCenter, m_d->mirrorMaskHorizontal, m_d->mirrorMaskVertical);

    painter->setStrokeStyle(m_d->strokeStyle);
    painter->setFillStyle(m_d->fillStyle);

    /**
     * The paintOp should be initialized the last, because it may
     * ask the painter for some options while initialization
     */
    painter->setPaintOpPreset(m_d->currentPaintOpPreset, m_d->currentNode, m_d->image);
}

void KisResourcesSnapshot::setupPaintAction(KisRecordedPaintAction *action)
{
    action->setPaintOpPreset(m_d->currentPaintOpPreset);
    action->setPaintIncremental(!needsIndirectPainting());

    action->setPaintColor(m_d->currentFgColor);
    action->setBackgroundColor(m_d->currentBgColor);
    action->setGenerator(m_d->currentGenerator);
    action->setGradient(m_d->currentGradient);
    action->setPattern(m_d->currentPattern);

    action->setOpacity(m_d->opacity / qreal(OPACITY_OPAQUE_U8));
    action->setCompositeOp(m_d->compositeOp->id());

    action->setStrokeStyle(m_d->strokeStyle);
    action->setFillStyle(m_d->fillStyle);
}

KisPostExecutionUndoAdapter* KisResourcesSnapshot::postExecutionUndoAdapter() const
{
    return m_d->undoAdapter;
}

void KisResourcesSnapshot::setCurrentNode(KisNodeSP node)
{
    m_d->currentNode = node;

    KisPaintDeviceSP device;
    if(m_d->currentNode && (device = m_d->currentNode->paintDevice())) {
        m_d->compositeOp = device->colorSpace()->compositeOp(m_d->compositeOpId);
        if(!m_d->compositeOp) {
            m_d->compositeOp = device->colorSpace()->compositeOp(COMPOSITE_OVER);
        }
    }
}

void KisResourcesSnapshot::setStrokeStyle(KisPainter::StrokeStyle strokeStyle)
{
    m_d->strokeStyle = strokeStyle;
}

void KisResourcesSnapshot::setFillStyle(KisPainter::FillStyle fillStyle)
{
    m_d->fillStyle = fillStyle;
}

KisNodeSP KisResourcesSnapshot::currentNode() const
{
    return m_d->currentNode;
}

KisImageWSP KisResourcesSnapshot::image() const
{
    return m_d->image;
}

bool KisResourcesSnapshot::needsIndirectPainting() const
{
    return !m_d->currentPaintOpPreset->settings()->paintIncremental();
}

QString KisResourcesSnapshot::indirectPaintingCompositeOp() const
{
    return m_d->currentPaintOpPreset->settings()->indirectPaintingCompositeOp();
}

KisSelectionSP KisResourcesSnapshot::activeSelection() const
{
    /**
     * It is possible to have/use the snapshot without the image. Such
     * usecase is present for example in the scratchpad.
     */
    KisSelectionSP selection = m_d->image ? m_d->image->globalSelection() : 0;

    KisLayerSP layer = dynamic_cast<KisLayer*>(m_d->currentNode.data());
    KisSelectionMaskSP mask;
    if((layer = dynamic_cast<KisLayer*>(m_d->currentNode.data()))) {
        selection = layer->selection();
    } else if ((mask = dynamic_cast<KisSelectionMask*>(m_d->currentNode.data())) &&
               mask->selection() == selection) {

        selection = 0;
    }

    return selection;
}

bool KisResourcesSnapshot::needsAirbrushing() const
{
    return m_d->currentPaintOpPreset->settings()->isAirbrushing();
}

int KisResourcesSnapshot::airbrushingRate() const
{
    return m_d->currentPaintOpPreset->settings()->rate();
}

void KisResourcesSnapshot::setOpacity(qreal opacity)
{
    m_d->opacity = opacity * OPACITY_OPAQUE_U8;
}

quint8 KisResourcesSnapshot::opacity() const
{
    return m_d->opacity;
}

const KoCompositeOp* KisResourcesSnapshot::compositeOp() const
{
    return m_d->compositeOp;
}

KoPattern* KisResourcesSnapshot::currentPattern() const
{
    return m_d->currentPattern;
}

KoColor KisResourcesSnapshot::currentFgColor() const
{
    return m_d->currentFgColor;
}

KoColor KisResourcesSnapshot::currentBgColor() const
{
    return m_d->currentBgColor;
}

KisPaintOpPresetSP KisResourcesSnapshot::currentPaintOpPreset() const
{
    return m_d->currentPaintOpPreset;
}


QBitArray KisResourcesSnapshot::channelLockFlags() const
{
    QBitArray channelFlags;
    KisPaintLayer *paintLayer;
    if ((paintLayer = dynamic_cast<KisPaintLayer*>(m_d->currentNode.data()))) {

        channelFlags = paintLayer->channelLockFlags();
        if (m_d->globalAlphaLock) {
            if (channelFlags.isEmpty()) {
                channelFlags = paintLayer->colorSpace()->channelFlags(true, true);
            }

            channelFlags &= paintLayer->colorSpace()->channelFlags(true, false);
        }
    }
    return channelFlags;
}

qreal KisResourcesSnapshot::effectiveZoom() const
{
    return m_d->effectiveZoom;
}

bool KisResourcesSnapshot::presetAllowsLod() const
{
    return m_d->presetAllowsLod;
}
