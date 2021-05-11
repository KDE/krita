/*
 *  SPDX-FileCopyrightText: 2021 Peter Schatz <voronwe13@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include <KoColorModelStandardIds.h>
#include <KoCompositeOpRegistry.h>
#include "KisColorSmudgeStrategySmearLightness.h"

#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"
#include "kis_selection.h"
#include "kis_pressure_paint_thickness_option.h"

#include "KisColorSmudgeInterstrokeData.h"

KisColorSmudgeStrategySmearLightness::KisColorSmudgeStrategySmearLightness(KisPainter* painter, bool smearAlpha,
    bool useDullingMode)
    : KisColorSmudgeStrategyBase(useDullingMode)
    , m_maskDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->alpha8()))
    , m_origDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->rgb8()))
    , m_smearAlpha(smearAlpha)
    , m_initializationPainter(painter)
{
    m_layerOverlayDevice.reset(new KisOverlayPaintDeviceWrapper(painter->device(), 1, KisOverlayPaintDeviceWrapper::LazyPreciseMode));
}

void KisColorSmudgeStrategySmearLightness::initializePainting()
{

    initializePaintingImpl(m_layerOverlayDevice->overlayColorSpace(),
        m_smearAlpha,
        m_initializationPainter->compositeOp()->id());

    m_sourceWrapperDevice.reset(new KisColorSmudgeSourcePaintDevice(*m_layerOverlayDevice));

    m_finalPainter.begin(m_layerOverlayDevice->overlay());
    m_finalPainter.setCompositeOp(finalCompositeOp(m_smearAlpha));
    
    m_finalPainter.setSelection(m_initializationPainter->selection());
    m_finalPainter.setChannelFlags(m_initializationPainter->channelFlags());
    m_finalPainter.copyMirrorInformationFrom(m_initializationPainter);

}

KisColorSmudgeStrategyBase::DabColoringStrategy& KisColorSmudgeStrategySmearLightness::coloringStrategy()
{
    return m_coloringStrategy;
}

void KisColorSmudgeStrategySmearLightness::updateMask(KisDabCache* dabCache, const KisPaintInformation& info,
    const KisDabShape& shape, const QPointF& cursorPoint,
    QRect* dstDabRect, qreal paintThickness)
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
KisColorSmudgeStrategySmearLightness::paintDab(const QRect& srcRect, const QRect& dstRect, const KoColor& currentPaintColor,
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


    qreal strength = opacity * qMax(colorRateValue * colorRateValue, 0.025 * (1.0 - smudgeRateValue));
    KisPaintDeviceSP smudgeDevice = m_layerOverlayDevice->overlay();

    KisFixedPaintDeviceSP tempColorDevice =
        new KisFixedPaintDevice(smudgeDevice->colorSpace(), m_memoryAllocator);

    Q_FOREACH(const QRect & rc, mirroredRects) {
        tempColorDevice->setRect(rc);
        tempColorDevice->lazyGrowBufferWithoutInitialization();

        smudgeDevice->readBytes(tempColorDevice->data(), rc);
        smudgeDevice->colorSpace()->
            modulateLightnessByGrayBrush(tempColorDevice->data(),
                reinterpret_cast<const QRgb*>(m_origDab->data()),
                strength,
                numPixels);

        smudgeDevice->writeBytes(tempColorDevice->data(), tempColorDevice->bounds());
    }



    m_layerOverlayDevice->writeRects(mirroredRects);

    return mirroredRects;
}