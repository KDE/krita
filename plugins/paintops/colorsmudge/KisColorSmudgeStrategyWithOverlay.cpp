/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisColorSmudgeStrategyWithOverlay.h"

#include <KoCompositeOpRegistry.h>

#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"
#include "kis_selection.h"

#include "KisOverlayPaintDeviceWrapper.h"

KisColorSmudgeStrategyWithOverlay::KisColorSmudgeStrategyWithOverlay(KisPainter *painter, KisImageSP image,
                                                                     bool smearAlpha, bool useDullingMode,
                                                                     bool useOverlayMode)
        : KisColorSmudgeStrategyBase(useDullingMode)
        , m_maskDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->alpha8()))
        , m_smearAlpha(smearAlpha)
        , m_initializationPainter(painter)
{
    m_layerOverlayDevice.reset(new KisOverlayPaintDeviceWrapper(painter->device(), 1, KisOverlayPaintDeviceWrapper::LazyPreciseMode));

    if (useOverlayMode && image) {
        m_imageOverlayDevice.reset(new KisOverlayPaintDeviceWrapper(image->projection(), 1, KisOverlayPaintDeviceWrapper::PreciseMode));
        m_sourceWrapperDevice.reset(new KisColorSmudgeSourceImage(image, *m_imageOverlayDevice));
    } else {
        m_sourceWrapperDevice.reset(new KisColorSmudgeSourcePaintDevice(*m_layerOverlayDevice));
    }
}

KisColorSmudgeStrategyWithOverlay::~KisColorSmudgeStrategyWithOverlay() {

}

void KisColorSmudgeStrategyWithOverlay::initializePainting()
{
    initializePaintingImpl(m_layerOverlayDevice->overlayColorSpace(),
                           m_smearAlpha,
                           m_initializationPainter->compositeOp()->id());

    m_finalPainter.begin(m_layerOverlayDevice->overlay());
    m_finalPainter.setCompositeOp(finalCompositeOp(m_smearAlpha));
    m_finalPainter.setSelection(m_initializationPainter->selection());
    m_finalPainter.setChannelFlags(m_initializationPainter->channelFlags());
    m_finalPainter.copyMirrorInformationFrom(m_initializationPainter);

    if (m_imageOverlayDevice) {
        m_overlayPainter.reset(new KisPainter());
        m_overlayPainter->begin(m_imageOverlayDevice->overlay());
        m_overlayPainter->setCompositeOp(finalCompositeOp(m_smearAlpha));
        m_overlayPainter->setSelection(m_initializationPainter->selection());
        m_overlayPainter->setChannelFlags(m_initializationPainter->channelFlags());
        m_overlayPainter->copyMirrorInformationFrom(m_initializationPainter);
    }
}

QVector<KisPainter *> KisColorSmudgeStrategyWithOverlay::finalPainters()
{
    QVector<KisPainter*> result;
    result << &m_finalPainter;
    if (m_overlayPainter) {
        result << m_overlayPainter.data();
    }
    return result;
}

QVector<QRect> KisColorSmudgeStrategyWithOverlay::paintDab(const QRect &srcRect, const QRect &dstRect,
                                                           const KoColor &currentPaintColor, qreal opacity,
                                                           qreal colorRateValue, qreal smudgeRateValue,
                                                           qreal maxPossibleSmudgeRateValue,
                                                           qreal lightnessStrengthValue, qreal smudgeRadiusValue)
{
    Q_UNUSED(lightnessStrengthValue);

    const QVector<QRect> mirroredRects = m_finalPainter.calculateAllMirroredRects(dstRect);

    QVector<QRect> readRects;
    readRects << mirroredRects;
    readRects << srcRect;

    m_sourceWrapperDevice->readRects(readRects);

    if (m_imageOverlayDevice) {
        /**
         * If we have m_imageOverlayDevice set, then m_sourceWrapperOverlay points to it, not
         * to the layer's overlay. Therefore, we should read from it as well.
         */
        m_layerOverlayDevice->readRects(readRects);
    }

    blendBrush(finalPainters(),
               m_sourceWrapperDevice,
               m_maskDab, m_shouldPreserveMaskDab,
               srcRect, dstRect,
               currentPaintColor,
               opacity,
               smudgeRateValue,
               maxPossibleSmudgeRateValue,
               colorRateValue, smudgeRadiusValue);

    m_layerOverlayDevice->writeRects(mirroredRects);

    return mirroredRects;
}
