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
#include <KoAbstractGradient.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings.h"
#include "kis_pattern.h"
#include "kis_canvas_resource_provider.h"
#include "filter/kis_filter_configuration.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "recorder/kis_recorded_paint_action.h"
#include "kis_default_bounds.h"

struct KisResourcesSnapshot::Private {
    Private() : currentPattern(0), currentGradient(0),
                currentGenerator(0), compositeOp(0)
    {
    }

    KisImageWSP image;
    KisDefaultBoundsSP bounds;
    KisPostExecutionUndoAdapter *undoAdapter;
    KoColor currentFgColor;
    KoColor currentBgColor;
    KisPattern *currentPattern;
    KoAbstractGradient *currentGradient;
    KisPaintOpPresetSP currentPaintOpPreset;
    KisNodeSP currentNode;
    qreal currentExposure;
    KisFilterConfiguration *currentGenerator;

    QPointF axisCenter;
    bool mirrorMaskHorizontal;
    bool mirrorMaskVertical;

    quint8 opacity;
    QString compositeOpId;
    const KoCompositeOp *compositeOp;

    KisPainter::StrokeStyle strokeStyle;
    KisPainter::FillStyle fillStyle;
};

KisResourcesSnapshot::KisResourcesSnapshot(KisImageWSP image, KisPostExecutionUndoAdapter *undoAdapter, KoCanvasResourceManager *resourceManager)
    : m_d(new Private())
{
    m_d->image = image;
    m_d->bounds = new KisDefaultBounds(image);
    m_d->undoAdapter = undoAdapter;
    m_d->currentFgColor = resourceManager->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
    m_d->currentBgColor = resourceManager->resource(KoCanvasResourceManager::BackgroundColor).value<KoColor>();
    m_d->currentPattern = static_cast<KisPattern*>(resourceManager->resource(KisCanvasResourceProvider::CurrentPattern).value<void*>());
    m_d->currentGradient = static_cast<KoAbstractGradient*>(resourceManager->resource(KisCanvasResourceProvider::CurrentGradient).value<void*>());
    m_d->currentPaintOpPreset = resourceManager->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
    m_d->currentExposure = resourceManager->resource(KisCanvasResourceProvider::HdrExposure).toDouble();
    m_d->currentGenerator = static_cast<KisFilterConfiguration*>(resourceManager->resource(KisCanvasResourceProvider::CurrentGeneratorConfiguration).value<void*>());

    m_d->axisCenter = resourceManager->resource(KisCanvasResourceProvider::MirrorAxisCenter).toPointF();
    if (m_d->axisCenter.isNull()){
        QRect bounds = m_d->bounds->bounds();
        m_d->axisCenter = QPointF(0.5 * bounds.width(), 0.5 * bounds.height());
    }

    m_d->mirrorMaskHorizontal = resourceManager->resource(KisCanvasResourceProvider::MirrorHorizontal).toBool();
    m_d->mirrorMaskVertical = resourceManager->resource(KisCanvasResourceProvider::MirrorVertical).toBool();


    qreal normOpacity = resourceManager->resource(KisCanvasResourceProvider::Opacity).toDouble();
    m_d->opacity = quint8(normOpacity * OPACITY_OPAQUE_U8);

    m_d->compositeOpId = resourceManager->resource(KisCanvasResourceProvider::CurrentCompositeOp).toString();
    setCurrentNode(resourceManager->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>());

    /**
     * Fill and Stroke styles are not a part of the resource manager
     * so the tools should set them manually
     * TODO: port stroke and fill styles to be a part
     *       of the resource manager
     */
    m_d->strokeStyle = KisPainter::StrokeStyleBrush;
    m_d->fillStyle = KisPainter::FillStyleNone;
}

KisResourcesSnapshot::~KisResourcesSnapshot()
{
    delete m_d;
}

void KisResourcesSnapshot::setupPainter(KisPainter* painter)
{
    painter->setBounds(m_d->bounds->bounds());
    painter->setPaintColor(m_d->currentFgColor);
    painter->setBackgroundColor(m_d->currentBgColor);
    painter->setGenerator(m_d->currentGenerator);
    painter->setPattern(m_d->currentPattern);
    painter->setGradient(m_d->currentGradient);
    painter->setPaintOpPreset(m_d->currentPaintOpPreset, m_d->image);

    KisPaintLayer *paintLayer;
    if ((paintLayer = dynamic_cast<KisPaintLayer*>(m_d->currentNode.data()))) {
        painter->setChannelFlags(paintLayer->channelLockFlags());
    }

    painter->setOpacity(m_d->opacity);
    painter->setCompositeOp(m_d->compositeOp);
    painter->setMirrorInformation(m_d->axisCenter, m_d->mirrorMaskHorizontal, m_d->mirrorMaskVertical);

    painter->setStrokeStyle(m_d->strokeStyle);
    painter->setFillStyle(m_d->fillStyle);
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

bool KisResourcesSnapshot::needsAirbrushing() const
{
    return m_d->currentPaintOpPreset->settings()->isAirbrushing();
}

int KisResourcesSnapshot::airbrushingRate() const
{
    return m_d->currentPaintOpPreset->settings()->rate();
}

quint8 KisResourcesSnapshot::opacity() const
{
    return m_d->opacity;
}

const KoCompositeOp* KisResourcesSnapshot::compositeOp() const
{
    return m_d->compositeOp;
}
