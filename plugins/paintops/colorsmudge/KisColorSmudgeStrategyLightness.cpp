/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoColorModelStandardIds.h>
#include <KoCompositeOpRegistry.h>
#include "KisColorSmudgeStrategyLightness.h"

#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"
#include "kis_selection.h"

#include "KisColorSmudgeInterstrokeData.h"

KisColorSmudgeStrategyLightness::KisColorSmudgeStrategyLightness(KisPainter *painter, bool smearAlpha,
                                                                 bool useDullingMode)
        : KisColorSmudgeStrategyBase(useDullingMode)
        , m_maskDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->alpha8()))
        , m_origDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->rgb8()))
        , m_smearAlpha(smearAlpha)
        , m_initializationPainter(painter)
{

}

void KisColorSmudgeStrategyLightness::initializePainting()
{
    KisColorSmudgeInterstrokeData *colorSmudgeData =
            dynamic_cast<KisColorSmudgeInterstrokeData*>(m_initializationPainter->device()->interstrokeData().data());

    if (colorSmudgeData) {
        m_projectionDevice = colorSmudgeData->projectionDevice;
        m_colorOnlyDevice = colorSmudgeData->colorBlendDevice;
        m_heightmapDevice = colorSmudgeData->heightmapDevice;
        m_layerOverlayDevice = &colorSmudgeData->overlayDeviceWrapper;
    }

    KIS_SAFE_ASSERT_RECOVER(colorSmudgeData) {
        m_projectionDevice = new KisPaintDevice(*m_initializationPainter->device());

        const KoColorSpace *cs = m_initializationPainter->device()->colorSpace();
        m_projectionDevice->convertTo(
                KoColorSpaceRegistry::instance()->colorSpace(
                        cs->colorModelId().id(),
                        Integer16BitsColorDepthID.id(),
                        cs->profile()));

        m_colorOnlyDevice = new KisPaintDevice(*m_projectionDevice);
        m_heightmapDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    }

    initializePaintingImpl(m_colorOnlyDevice->colorSpace(),
                           m_smearAlpha,
                           m_initializationPainter->compositeOp()->id());

    m_heightmapPainter.begin(m_heightmapDevice);

    m_finalPainter.begin(m_colorOnlyDevice);
    m_finalPainter.setCompositeOp(COMPOSITE_COPY);
    m_finalPainter.setSelection(m_initializationPainter->selection());
    m_finalPainter.setChannelFlags(m_initializationPainter->channelFlags());
    m_finalPainter.copyMirrorInformationFrom(m_initializationPainter);

    m_heightmapPainter.setCompositeOp(COMPOSITE_ALPHA_DARKEN);
    m_heightmapPainter.setSelection(m_initializationPainter->selection());
    m_heightmapPainter.copyMirrorInformationFrom(m_initializationPainter);
}

KisColorSmudgeStrategyBase::DabColoringStrategy &KisColorSmudgeStrategyLightness::coloringStrategy()
{
    return m_coloringStrategy;
}

void KisColorSmudgeStrategyLightness::updateMask(KisDabCache *dabCache, const KisPaintInformation &info,
                                                 const KisDabShape &shape, const QPointF &cursorPoint,
                                                 QRect *dstDabRect)
{

    m_origDab = dabCache->fetchNormalizedImageDab(m_origDab->colorSpace(),
                                                  cursorPoint,
                                                  shape,
                                                  info,
                                                  1.0,
                                                  dstDabRect);

    const int numPixels = m_origDab->bounds().width() * m_origDab->bounds().height();

    m_maskDab->setRect(m_origDab->bounds());
    m_maskDab->lazyGrowBufferWithoutInitialization();
    m_origDab->colorSpace()->copyOpacityU8(m_origDab->data(), m_maskDab->data(), numPixels);

    m_shouldPreserveOriginalDab = !dabCache->needSeparateOriginal();
}

QVector<QRect>
KisColorSmudgeStrategyLightness::paintDab(const QRect &srcRect, const QRect &dstRect, const KoColor &currentPaintColor,
                                          qreal opacity, qreal colorRateValue, qreal smudgeRateValue,
                                          qreal maxPossibleSmudgeRateValue, qreal lightnessStrengthValue,
                                          qreal smudgeRadiusValue)
{
    const int numPixels = dstRect.width() * dstRect.height();

    const QVector<QRect> mirroredRects = m_finalPainter.calculateAllMirroredRects(dstRect);

    QVector<QRect> readRects;
    readRects << mirroredRects;
    readRects << srcRect;
    m_layerOverlayDevice->readRects(readRects);

    blendBrush({&m_finalPainter},
               toQShared(new KisColorSmudgeSourcePaintDevice(m_colorOnlyDevice)),
               m_maskDab, m_shouldPreserveOriginalDab,
               srcRect, dstRect,
               currentPaintColor,
               opacity,
               smudgeRateValue,
               maxPossibleSmudgeRateValue,
               colorRateValue,
               smudgeRadiusValue);

    m_heightmapPainter.bltFixed(dstRect.topLeft(), m_origDab, m_origDab->bounds());
    m_heightmapPainter.renderMirrorMaskSafe(dstRect, m_origDab, m_shouldPreserveOriginalDab);

    KisFixedPaintDeviceSP tempColorDevice =
            new KisFixedPaintDevice(m_colorOnlyDevice->colorSpace(), m_memoryAllocator);

    KisFixedPaintDeviceSP tempHeightmapDevice =
            new KisFixedPaintDevice(m_heightmapDevice->colorSpace(), m_memoryAllocator);

    Q_FOREACH (const QRect &rc, mirroredRects) {
            tempColorDevice->setRect(rc);
            tempColorDevice->lazyGrowBufferWithoutInitialization();

            tempHeightmapDevice->setRect(rc);
            tempHeightmapDevice->lazyGrowBufferWithoutInitialization();

            m_colorOnlyDevice->readBytes(tempColorDevice->data(), rc);
            m_heightmapDevice->readBytes(tempHeightmapDevice->data(), rc);
            tempColorDevice->colorSpace()->
                    modulateLightnessByGrayBrush(tempColorDevice->data(),
                                                 reinterpret_cast<const QRgb*>(tempHeightmapDevice->data()),
                                                 0,
                                                 lightnessStrengthValue,
                                                 numPixels);
            m_projectionDevice->writeBytes(tempColorDevice->data(), tempColorDevice->bounds());
        }
    m_layerOverlayDevice->writeRects(mirroredRects);

    return mirroredRects;
}
