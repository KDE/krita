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
#include "kis_pressure_paint_thickness_option.h"

#include "KisColorSmudgeInterstrokeData.h"
#include "kis_algebra_2d.h"

KisColorSmudgeStrategyLightness::KisColorSmudgeStrategyLightness(KisPainter *painter, bool smearAlpha,
                                                                 bool useDullingMode, KisPressurePaintThicknessOption::ThicknessMode thicknessMode)
        : KisColorSmudgeStrategyBase(useDullingMode)
        , m_maskDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->alpha8()))
        , m_origDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->rgb8()))
        , m_smearAlpha(smearAlpha)
        , m_initializationPainter(painter)
        , m_thicknessMode(thicknessMode)
{
    KIS_SAFE_ASSERT_RECOVER(thicknessMode == KisPressurePaintThicknessOption::OVERLAY ||
                            thicknessMode == KisPressurePaintThicknessOption::OVERWRITE) {

        thicknessMode = KisPressurePaintThicknessOption::OVERLAY;
    }
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

    // we should read data from the color layer, not from the final projection layer
    m_sourceWrapperDevice = toQShared(new KisColorSmudgeSourcePaintDevice(*m_layerOverlayDevice, 1));

    m_finalPainter.begin(m_colorOnlyDevice);
    m_finalPainter.setCompositeOp(COMPOSITE_COPY);
    m_finalPainter.setSelection(m_initializationPainter->selection());
    m_finalPainter.setChannelFlags(m_initializationPainter->channelFlags());
    m_finalPainter.copyMirrorInformationFrom(m_initializationPainter);

    m_heightmapPainter.setCompositeOp(COMPOSITE_OVER);
    m_heightmapPainter.setSelection(m_initializationPainter->selection());
    m_heightmapPainter.copyMirrorInformationFrom(m_initializationPainter);

}

KisColorSmudgeStrategyBase::DabColoringStrategy &KisColorSmudgeStrategyLightness::coloringStrategy()
{
    return m_coloringStrategy;
}

void KisColorSmudgeStrategyLightness::updateMask(KisDabCache *dabCache, const KisPaintInformation &info,
                                                 const KisDabShape &shape, const QPointF &cursorPoint,
                                                 QRect *dstDabRect, qreal paintThickness)
{

    static KoColor color(QColor(127, 127, 127), m_origDab->colorSpace());
    m_origDab = dabCache->fetchDab(m_origDab->colorSpace(),
        color,
        cursorPoint,
        shape,
        info,
        1.0,
        dstDabRect,
        paintThickness);

    const int numPixels = m_origDab->bounds().width() * m_origDab->bounds().height();

    m_maskDab->setRect(m_origDab->bounds());
    m_maskDab->lazyGrowBufferWithoutInitialization();
    m_origDab->colorSpace()->copyOpacityU8(m_origDab->data(), m_maskDab->data(), numPixels);

    m_shouldPreserveOriginalDab = !dabCache->needSeparateOriginal();
}

QVector<QRect>
KisColorSmudgeStrategyLightness::paintDab(const QRect &srcRect, const QRect &dstRect, const KoColor &currentPaintColor,
                                          qreal opacity, qreal colorRateValue, qreal smudgeRateValue,
                                          qreal maxPossibleSmudgeRateValue, qreal paintThicknessValue,
                                          qreal smudgeRadiusValue)
{
    const int numPixels = dstRect.width() * dstRect.height();

    const QVector<QRect> mirroredRects = m_finalPainter.calculateAllMirroredRects(dstRect);

    QVector<QRect> readRects;
    readRects << mirroredRects;
    readRects << srcRect;
    m_sourceWrapperDevice->readRects(readRects);


    blendBrush({ &m_finalPainter },
        m_sourceWrapperDevice,
        m_maskDab, m_shouldPreserveOriginalDab,
        srcRect, dstRect,
        currentPaintColor,
        opacity,
        smudgeRateValue,
        maxPossibleSmudgeRateValue,
        colorRateValue,
        smudgeRadiusValue);


    const qreal overlaySmearRate = smudgeRateValue - 0.01; //adjust so minimum value is 0 instead of 1%
    const qreal overlayAdjustment =
        (m_thicknessMode == KisPressurePaintThicknessOption::ThicknessMode::OVERWRITE) ?
        1.0 : KisAlgebra2D::lerp(overlaySmearRate, 1.0, paintThicknessValue);
    const quint8 brushHeightmapOpacity = qRound(opacity * overlayAdjustment * 255.0);
    m_heightmapPainter.setOpacity(brushHeightmapOpacity);
    m_heightmapPainter.bltFixed(dstRect.topLeft(), m_origDab, m_origDab->bounds());
    m_heightmapPainter.renderMirrorMaskSafe(dstRect, m_origDab, m_shouldPreserveOriginalDab);


    KisFixedPaintDeviceSP tempColorDevice =
        new KisFixedPaintDevice(m_colorOnlyDevice->colorSpace(), m_memoryAllocator);

    KisFixedPaintDeviceSP tempHeightmapDevice =
        new KisFixedPaintDevice(m_heightmapDevice->colorSpace(), m_memoryAllocator);

    Q_FOREACH(const QRect& rc, mirroredRects) {
        tempColorDevice->setRect(rc);
        tempColorDevice->lazyGrowBufferWithoutInitialization();

        tempHeightmapDevice->setRect(rc);
        tempHeightmapDevice->lazyGrowBufferWithoutInitialization();

        m_colorOnlyDevice->readBytes(tempColorDevice->data(), rc);
        m_heightmapDevice->readBytes(tempHeightmapDevice->data(), rc);
        tempColorDevice->colorSpace()->
            modulateLightnessByGrayBrush(tempColorDevice->data(),
                reinterpret_cast<const QRgb*>(tempHeightmapDevice->data()),
                1.0,
                numPixels);
        m_projectionDevice->writeBytes(tempColorDevice->data(), tempColorDevice->bounds());
    }
 
    m_layerOverlayDevice->writeRects(mirroredRects);

    return mirroredRects;
}
