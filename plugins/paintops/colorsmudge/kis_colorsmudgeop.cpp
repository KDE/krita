/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_colorsmudgeop.h"

#include <cmath>
#include <memory>
#include <QRect>

#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoColorProfile.h>
#include <KoMixColorsOp.h>
#include <KoCompositeOpRegistry.h>

#include <kis_algebra_2d.h>
#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_iterator_ng.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_brush_based_paintop_settings.h>
#include <kis_cross_device_color_sampler.h>
#include <kis_fixed_paint_device.h>
#include <kis_lod_transform.h>
#include <kis_spacing_information.h>
#include <KoColorModelStandardIds.h>
#include "kis_paintop_plugin_utils.h"

#include "kis_paint_device_debug_utils.h"

#include "KisInterstrokeData.h"
#include "KisInterstrokeDataFactory.h"

#include "kis_brush_option.h"
#include "kis_transaction.h"
#include "KisOverlayPaintDeviceWrapper.h"

struct ColorSmudgeInterstrokeData : public KisInterstrokeData
{
    KisPaintDeviceSP colorBlendDevice;
    KisPaintDeviceSP heightmapDevice;
    KisPaintDeviceSP projectionDevice;
    KisOverlayPaintDeviceWrapper overlayDeviceWrapper;

    ColorSmudgeInterstrokeData(KisPaintDeviceSP source)
        : KisInterstrokeData(source)
        , overlayDeviceWrapper(source, 2, KisOverlayPaintDeviceWrapper::PreciseMode)
    {
        projectionDevice = overlayDeviceWrapper.overlay(0);
        colorBlendDevice = overlayDeviceWrapper.overlay(1);
        heightmapDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    }

    ~ColorSmudgeInterstrokeData() override {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!m_parentCommand);
    }

    void beginTransaction() override {
        KIS_SAFE_ASSERT_RECOVER_RETURN(!m_parentCommand);

        m_parentCommand.reset(new KUndo2Command());
        m_colorBlendDeviceTransaction.reset(new KisTransaction(colorBlendDevice, m_parentCommand.data()));
        m_heightmapDeviceTransaction.reset(new KisTransaction(heightmapDevice, m_parentCommand.data()));
        m_projectionDeviceTransaction.reset(new KisTransaction(projectionDevice, m_parentCommand.data()));
    }

    KUndo2Command * endTransaction() override {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_parentCommand, 0);

        // the internal undo commands are owned by m_parentCommand
        (void) m_colorBlendDeviceTransaction->endAndTake();
        (void) m_heightmapDeviceTransaction->endAndTake();
        (void) m_projectionDeviceTransaction->endAndTake();

        return m_parentCommand.take();
    }

private:
    QScopedPointer<KUndo2Command> m_parentCommand;
    QScopedPointer<KisTransaction> m_colorBlendDeviceTransaction;
    QScopedPointer<KisTransaction> m_heightmapDeviceTransaction;
    QScopedPointer<KisTransaction> m_projectionDeviceTransaction;
};

struct ColorSmudgeInterstrokeDataFactory : public KisInterstrokeDataFactory
{
    bool isCompatible(KisInterstrokeData *data) override {
        ColorSmudgeInterstrokeData *colorSmudgeData =
            dynamic_cast<ColorSmudgeInterstrokeData*>(data);

        return colorSmudgeData;
    }

    KisInterstrokeData * create(KisPaintDeviceSP device) override {
        ColorSmudgeInterstrokeData *data = new ColorSmudgeInterstrokeData(device);



        return data;
    }
};

struct ColorSmudgeStrategyBase
{
    ColorSmudgeStrategyBase()
        : m_memoryAllocator(new KisOptimizedByteArray::PooledMemoryAllocator())
    {
    }

    virtual ~ColorSmudgeStrategyBase() {
    }

    virtual void initializePainting() = 0;

    virtual void updateMask(KisDabCache *dabCache,
                            const KisPaintInformation& info,
                            const KisDabShape &shape,
                            const QPointF &cursorPoint,
                            QRect *dstDabRect) = 0;

    virtual QVector<QRect> paintDab(const QRect &srcRect, const QRect &dstRect,
                                    const QPoint &canvasLocalSamplePoint,
                                    const KoColor &currentPaintColor,
                                    qreal opacity,
                                    qreal colorRateValue,
                                    qreal smudgeRateValue,
                                    qreal lightnessStrengthValue,
                                    qreal smudgeRadiusValue) = 0;

    virtual const KoColorSpace* preciseColorSpace() const = 0;

protected:
        KisOptimizedByteArray::MemoryAllocatorSP m_memoryAllocator;
};

struct WeightedSampleWrapper
{
    WeightedSampleWrapper(KoMixColorsOp::Mixer *mixer,
                          KisFixedPaintDeviceSP maskDab, const QRect &maskRect,
                          KisFixedPaintDeviceSP sampleDab, const QRect &sampleRect)
        : m_mixer(mixer),
          m_maskRect(maskRect),
          m_maskPtr(maskDab->data()),
          m_maskStride(maskDab->bounds().width()),
          m_samplePixelSize(sampleDab->colorSpace()->pixelSize()),
          m_sampleRect(sampleRect),
          m_samplePtr(sampleDab->data()),
          m_sampleStride(sampleDab->bounds().width() * m_samplePixelSize)
    {

    }

    inline void samplePixel(const QPoint &relativeSamplePoint) {
        const QPoint maskPt(relativeSamplePoint - m_maskRect.topLeft() + m_sampleRect.topLeft());

        const qint16 opacity = *(m_maskPtr + maskPt.x() + maskPt.y() * m_maskStride);
        const quint8 *ptr = m_samplePtr + relativeSamplePoint.x() * m_samplePixelSize + relativeSamplePoint.y() * m_sampleStride;

        m_mixer->accumulate(ptr, &opacity, opacity, 1);
    }

    static void verifySampleRadiusValue(qreal *sampleRadiusValue) {
        KIS_SAFE_ASSERT_RECOVER(*sampleRadiusValue <= 1.0) {
            *sampleRadiusValue = 1.0;
        }
    }

    KoMixColorsOp::Mixer *m_mixer;
    const QRect m_maskRect;
    quint8 *m_maskPtr;
    const int m_maskStride;
    int m_samplePixelSize;
    const QRect m_sampleRect;
    quint8 *m_samplePtr;
    const int m_sampleStride;
};


struct AveragedSampleWrapper
{
    AveragedSampleWrapper(KoMixColorsOp::Mixer *mixer,
                          KisFixedPaintDeviceSP maskDab, const QRect &maskRect,
                          KisFixedPaintDeviceSP sampleDab, const QRect &sampleRect)
        : m_mixer(mixer),
          m_samplePixelSize(sampleDab->colorSpace()->pixelSize()),
          m_sampleRect(sampleRect),
          m_samplePtr(sampleDab->data()),
          m_sampleStride(sampleDab->bounds().width() * m_samplePixelSize)
    {
        Q_UNUSED(maskDab);
        Q_UNUSED(maskRect);
    }

    inline void samplePixel(const QPoint &relativeSamplePoint) {
        const quint8 *ptr = m_samplePtr + relativeSamplePoint.x() * m_samplePixelSize + relativeSamplePoint.y() * m_sampleStride;
        m_mixer->accumulateAverage(ptr, 1);
    }

    static void verifySampleRadiusValue(qreal *sampleRadiusValue) {
        Q_UNUSED(sampleRadiusValue);
    }

    KoMixColorsOp::Mixer *m_mixer;
    int m_samplePixelSize;
    const QRect m_sampleRect;
    quint8 *m_samplePtr;
    const int m_sampleStride;
};

template<class WeightingModeWrapper>
void sampleDullingColor(const QRect &srcRect,
                        qreal sampleRadiusValue,
                        KisPaintDeviceSP sourceDevice,
                        KisFixedPaintDeviceSP tempFixedDevice,
                        KisFixedPaintDeviceSP maskDab,
                        KoColor *resultColor)

{
    WeightingModeWrapper::verifySampleRadiusValue(&sampleRadiusValue);

    KIS_SAFE_ASSERT_RECOVER(*resultColor->colorSpace() == *sourceDevice->colorSpace()) {
        resultColor->convertTo(sourceDevice->colorSpace());
    }

    const QRect minimalRect = QRect(srcRect.center(), QSize(1,1));

    const QRect sampleRect = sampleRadiusValue > 0 ?
        KisAlgebra2D::blowRect(srcRect, 0.5 * (sampleRadiusValue - 1.0)) | minimalRect :
        minimalRect;

    tempFixedDevice->setRect(sampleRect);
    tempFixedDevice->lazyGrowBufferWithoutInitialization();

    const KoColorSpace *cs = tempFixedDevice->colorSpace();
    const int numPixels = sampleRect.width() * sampleRect.height();
    sourceDevice->readBytes(tempFixedDevice->data(), sampleRect);

    KisAlgebra2D::HaltonSequenceGenerator hGen(2);
    KisAlgebra2D::HaltonSequenceGenerator vGen(3);

    QScopedPointer<KoMixColorsOp::Mixer> mixer(cs->mixColorsOp()->createMixer());

    const int minSamples =
        qMin(numPixels, qMax(64, qRound(0.02 * numPixels)));

    WeightingModeWrapper weightingModeWrapper(mixer.data(),
                                              maskDab, srcRect,
                                              tempFixedDevice, sampleRect);

    KoColor lastPickedColor(*resultColor);

    for (int i = 0; i < minSamples; i++) {
        const QPoint pt(hGen.generate(sampleRect.width()),
                        vGen.generate(sampleRect.height()));

        weightingModeWrapper.samplePixel(pt);
    }

    mixer->computeMixedColor(resultColor->data());
    lastPickedColor = *resultColor;

    const int batchSize = 16;
    int numSamplesLeft = numPixels - minSamples;

    while (numSamplesLeft > 0) {
        const int currentBatchSize = qMin(numSamplesLeft, batchSize);
        for (int i = 0; i < currentBatchSize; i++) {
            const QPoint pt(hGen.generate(sampleRect.width()),
                            vGen.generate(sampleRect.height()));

            weightingModeWrapper.samplePixel(pt);
        }

        mixer->computeMixedColor(resultColor->data());

        const quint8 difference =
            cs->differenceA(resultColor->data(), lastPickedColor.data());

        if (difference <= 2) break;

        lastPickedColor = *resultColor;
        numSamplesLeft -= currentBatchSize;
    }
}

struct ColorSmudgeStrategyColorBased : public ColorSmudgeStrategyBase
{
    ColorSmudgeStrategyColorBased(bool useDullingMode)
        : m_useDullingMode(useDullingMode)
    {
    }

    void initializePainting() override
    {
    }

    void initializePaintingImpl(const KoColorSpace *dstColorSpace,
                                bool smearAlpha,
                                const QString &colorRateCompositeOpId)
    {
        initializePaintingImpl(dstColorSpace, smearAlpha ? COMPOSITE_COPY : COMPOSITE_OVER, colorRateCompositeOpId);
    }

    void initializePaintingImpl(const KoColorSpace *dstColorSpace,
                                const QString &smearCompositeOpId,
                                const QString &colorRateCompositeOpId)
    {
        m_blendDevice = new KisFixedPaintDevice(dstColorSpace, m_memoryAllocator);
        m_smearOp = dstColorSpace->compositeOp(smearCompositeOpId);
        m_colorRateOp = dstColorSpace->compositeOp(colorRateCompositeOpId);
        m_preparedDullingColor.convertTo(dstColorSpace);
    }

    const KoColorSpace* preciseColorSpace() const override
    {
        // verify that initialize() has already been called!
        KIS_ASSERT_RECOVER_RETURN_VALUE(m_smearOp, KoColorSpaceRegistry::instance()->rgb8());

        return m_smearOp->colorSpace();
    }

    virtual quint8 finalPainterOpacity(qreal opacity, qreal smudgeRateValue)
    {
        return OPACITY_OPAQUE_U8;
    }

    virtual quint8 colorRateOpacity(qreal opacity, qreal smudgeRateValue, qreal colorRateValue)
    {
        Q_UNUSED(smudgeRateValue);
        return qRound(colorRateValue * colorRateValue * opacity * 255.0);
    }

    virtual quint8 dullingRateOpacity(qreal opacity, qreal smudgeRateValue)
    {
        return qRound(0.8 * smudgeRateValue * opacity * 255.0);
    }

    virtual quint8 smearRateOpacity(qreal opacity, qreal smudgeRateValue)
    {
        return qRound(smudgeRateValue * opacity * 255.0);
    }

    virtual void sampleDullingColor(const QRect &srcRect,
                                    const QRect &dstRect,
                                    qreal sampleRadiusValue,
                                    KisPaintDeviceSP sourceDevice,
                                    KisFixedPaintDeviceSP tempFixedDevice,
                                    KisFixedPaintDeviceSP maskDab,
                                    KoColor *resultColor)
    {
        Q_UNUSED(dstRect);
        ::sampleDullingColor<WeightedSampleWrapper>(srcRect, sampleRadiusValue,
                                                    sourceDevice, tempFixedDevice,
                                                    maskDab, resultColor);
    }

    void blendBrush(KisPainter *dstPainter,
                    KisPaintDeviceSP src,
                    KisFixedPaintDeviceSP maskDab,
                    const QRect &srcRect, const QRect &dstRect,
                    const QPoint &canvasLocalSamplePoint,
                    const KoColor &currentPaintColor,
                    qreal opacity,
                    qreal smudgeRateValue,
                    qreal colorRateValue,
                    qreal smudgeRadiusValue)
    {
        const quint8 colorRateOpacity = this->colorRateOpacity(opacity, smudgeRateValue, colorRateValue);

        if (m_useDullingMode) {
            this->sampleDullingColor(srcRect, dstRect,
                                     smudgeRadiusValue,
                                     dstPainter->device(), m_blendDevice,
                                     maskDab, &m_preparedDullingColor);

            KIS_SAFE_ASSERT_RECOVER(*m_preparedDullingColor.colorSpace() == *m_colorRateOp->colorSpace()) {
                m_preparedDullingColor.convertTo(m_colorRateOp->colorSpace());
            }
        }

        m_blendDevice->setRect(dstRect);
        m_blendDevice->lazyGrowBufferWithoutInitialization();

        if (colorRateOpacity > 0 &&
            m_useDullingMode &&
            supportsFusedDullingBlending() &&
            m_smearOp->id() == COMPOSITE_OVER &&
            m_colorRateOp->id() == COMPOSITE_OVER) {

            blendInFusedBackgroundAndColorRateWithDulling(m_blendDevice,
                                                          src,
                                                          srcRect, dstRect,
                                                          canvasLocalSamplePoint,
                                                          currentPaintColor.convertedTo(m_preparedDullingColor.colorSpace()),
                                                          opacity,
                                                          smudgeRateValue,
                                                          colorRateValue);

        } else {
            if (!m_useDullingMode) {
                blendInBackgroundWithSmearing(m_blendDevice, src,
                                              srcRect, dstRect, canvasLocalSamplePoint, opacity, smudgeRateValue);
            } else {
                blendInBackgroundWithDulling(m_blendDevice, src,
                                             srcRect, dstRect, canvasLocalSamplePoint, opacity, smudgeRateValue);
            }

            if (colorRateOpacity > 0) {
                blendInColorRate(colorRateOpacity,
                                 currentPaintColor.convertedTo(m_preparedDullingColor.colorSpace()),
                                 m_blendDevice, dstRect);
            }
        }

        dstPainter->setOpacity(finalPainterOpacity(opacity, smudgeRateValue));

        dstPainter->bltFixedWithFixedSelection(dstRect.x(), dstRect.y(),
                                               m_blendDevice, maskDab,
                                               maskDab->bounds().x(), maskDab->bounds().y(),
                                               m_blendDevice->bounds().x(), m_blendDevice->bounds().y(),
                                               dstRect.width(), dstRect.height());
        dstPainter->renderMirrorMaskSafe(dstRect, m_blendDevice, maskDab, false);

    }

    virtual bool supportsFusedDullingBlending() const {
        return true;
    }

    virtual void blendInColorRate(quint8 colorRateOpacity,
                                  const KoColor &paintColor,
                                  KisFixedPaintDeviceSP dstDevice,
                                  const QRect dstRect)
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN(*paintColor.colorSpace() == *m_colorRateOp->colorSpace());

        m_colorRateOp->composite(dstDevice->data(), dstRect.width() * dstDevice->pixelSize(),
                                 paintColor.data(), 0,
                                 0, 0,
                                 dstRect.height(), dstRect.width(),
                                 colorRateOpacity);
    }

    void blendInBackgroundWithSmearing(KisFixedPaintDeviceSP dst,
                                       KisPaintDeviceSP src,
                                       const QRect &srcRect, const QRect &dstRect,
                                       const QPoint &canvasLocalSamplePoint,
                                       qreal opacity,
                                       qreal smudgeRateValue)
    {
        Q_UNUSED(canvasLocalSamplePoint);

        const quint8 smudgeRateOpacity = this->smearRateOpacity(opacity, smudgeRateValue);

        if (m_smearOp->id() == COMPOSITE_COPY && smudgeRateOpacity == OPACITY_OPAQUE_U8) {
            src->readBytes(dst->data(), srcRect);
        } else {
            src->readBytes(dst->data(), dstRect);

            KisFixedPaintDevice tempDevice(src->colorSpace(), m_memoryAllocator);
            tempDevice.setRect(srcRect);
            tempDevice.lazyGrowBufferWithoutInitialization();

            src->readBytes(tempDevice.data(), srcRect);
            m_smearOp->composite(dst->data(), dstRect.width() * dst->pixelSize(),
                                 tempDevice.data(), dstRect.width() * tempDevice.pixelSize(), // stride should be random non-zero
                                 0, 0,
                                 1, dstRect.width() * dstRect.height(),
                                 smudgeRateOpacity);
        }
    }

    void blendInBackgroundWithDulling(KisFixedPaintDeviceSP dst,
                                      KisPaintDeviceSP src,
                                      const QRect &srcRect, const QRect &dstRect,
                                      const QPoint &canvasLocalSamplePoint,
                                      qreal opacity,
                                      qreal smudgeRateValue)
    {
        Q_UNUSED(srcRect);

        const quint8 smudgeRateOpacity = this->dullingRateOpacity(opacity, smudgeRateValue);

        if (m_smearOp->id() == COMPOSITE_COPY && smudgeRateOpacity == OPACITY_OPAQUE_U8) {
            dst->fill(dst->bounds(), m_preparedDullingColor);
        } else {
            src->readBytes(dst->data(), dstRect);
            m_smearOp->composite(dst->data(), dstRect.width() * dst->pixelSize(),
                                 m_preparedDullingColor.data(), 0,
                                 0, 0,
                                 1, dstRect.width() * dstRect.height(),
                                 smudgeRateOpacity);
        }
    }

    void blendInFusedBackgroundAndColorRateWithDulling(KisFixedPaintDeviceSP dst,
                                                       KisPaintDeviceSP src,
                                                       const QRect &srcRect, const QRect &dstRect,
                                                       const QPoint &canvasLocalSamplePoint,
                                                       const KoColor &paintColor,
                                                       qreal opacity,
                                                       qreal smudgeRateValue,
                                                       qreal colorRateValue)
    {
        Q_UNUSED(srcRect);

        const quint8 smudgeRateOpacity = this->dullingRateOpacity(opacity, smudgeRateValue);
        const quint8 colorRateOpacity = this->colorRateOpacity(opacity, smudgeRateValue, colorRateValue);

        KoColor dullingFillColor(m_preparedDullingColor);

        KIS_SAFE_ASSERT_RECOVER_RETURN(*paintColor.colorSpace() == *m_colorRateOp->colorSpace());
        m_colorRateOp->composite(dullingFillColor.data(), 1, paintColor.data(), 1, 0, 0, 1, 1, colorRateOpacity);

        src->readBytes(dst->data(), dstRect);
        m_smearOp->composite(dst->data(), dstRect.width() * dst->pixelSize(),
                             dullingFillColor.data(), 0,
                             0, 0,
                             1, dstRect.width() * dstRect.height(),
                             smudgeRateOpacity);
    }

protected:
    const KoCompositeOp * m_colorRateOp;
private:
    KisFixedPaintDeviceSP m_blendDevice;
    const KoCompositeOp * m_smearOp;
    bool m_useDullingMode = true;
    KoColor m_preparedDullingColor;
};

struct ColorSmudgeStrategyLightness : public ColorSmudgeStrategyColorBased
{
    ColorSmudgeStrategyLightness(KisPainter *painter,
                                 bool smearAlpha,
                                 bool useDullingMode)
        : ColorSmudgeStrategyColorBased(useDullingMode)
        , m_maskDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->alpha8()))
        , m_origDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->rgb8()))
    {
        ColorSmudgeInterstrokeData *colorSmudgeData =
            dynamic_cast<ColorSmudgeInterstrokeData*>(painter->device()->interstrokeData().data());

        if (colorSmudgeData) {
            m_projectionDevice = colorSmudgeData->projectionDevice;
            m_colorOnlyDevice = colorSmudgeData->colorBlendDevice;
            m_heightmapDevice = colorSmudgeData->heightmapDevice;
            m_overlayDevice = &colorSmudgeData->overlayDeviceWrapper;
        }

        KIS_SAFE_ASSERT_RECOVER(colorSmudgeData) {
            m_projectionDevice = new KisPaintDevice(*painter->device());

            const KoColorSpace *cs = painter->device()->colorSpace();
            m_projectionDevice->convertTo(
                KoColorSpaceRegistry::instance()->colorSpace(
                    cs->colorModelId().id(),
                    Integer16BitsColorDepthID.id(),
                    cs->profile()));

            m_colorOnlyDevice = new KisPaintDevice(*m_projectionDevice);
            m_heightmapDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
        }

        initializePaintingImpl(m_colorOnlyDevice->colorSpace(),
                               smearAlpha,
                               painter->compositeOp()->id());

        m_heightmapPainter.begin(m_heightmapDevice);

        m_finalPainter.begin(m_colorOnlyDevice);
        m_finalPainter.setCompositeOp(COMPOSITE_COPY);
        m_finalPainter.setSelection(painter->selection());
        m_finalPainter.setChannelFlags(painter->channelFlags());
        m_finalPainter.copyMirrorInformationFrom(painter);

        m_heightmapPainter.setCompositeOp(COMPOSITE_ALPHA_DARKEN);
        m_heightmapPainter.setSelection(painter->selection());
        m_heightmapPainter.copyMirrorInformationFrom(painter);
    }

    void updateMask(KisDabCache *dabCache,
                    const KisPaintInformation& info,
                    const KisDabShape &shape,
                    const QPointF &cursorPoint,
                    QRect *dstDabRect) override {

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

    QVector<QRect> paintDab(const QRect &srcRect, const QRect &dstRect,
                  const QPoint &canvasLocalSamplePoint,
                  const KoColor &currentPaintColor,
                  qreal opacity,
                  qreal colorRateValue,
                  qreal smudgeRateValue,
                  qreal lightnessStrengthValue,
                  qreal smudgeRadiusValue) override
    {
        const int numPixels = dstRect.width() * dstRect.height();

        const QVector<QRect> mirroredRects = m_finalPainter.calculateAllMirroredRects(dstRect);

        QVector<QRect> readRects;
        readRects << mirroredRects;
        readRects << srcRect;
        m_overlayDevice->readRects(readRects);

        blendBrush(&m_finalPainter,
                   m_colorOnlyDevice,
                   m_maskDab,
                   srcRect, dstRect,
                   canvasLocalSamplePoint,
                   currentPaintColor,
                   opacity, smudgeRateValue, colorRateValue, smudgeRadiusValue);

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
        m_overlayDevice->writeRects(mirroredRects);

        return mirroredRects;
    }

    KisFixedPaintDeviceSP m_maskDab;
    KisFixedPaintDeviceSP m_origDab;
    KisPaintDeviceSP m_heightmapDevice;
    KisPaintDeviceSP m_colorOnlyDevice;
    KisPaintDeviceSP m_projectionDevice;
    KisOverlayPaintDeviceWrapper *m_overlayDevice;
    KisPainter m_finalPainter;
    KisPainter m_heightmapPainter;
    bool m_shouldPreserveOriginalDab = true;
};

struct ColorSmudgeStrategyMask : public ColorSmudgeStrategyColorBased
{
    ColorSmudgeStrategyMask(KisPainter *painter,
                            bool smearAlpha,
                            bool useDullingMode)
        : ColorSmudgeStrategyColorBased(useDullingMode)
        , m_overlayDevice(new KisOverlayPaintDeviceWrapper(painter->device(), 1, KisOverlayPaintDeviceWrapper::LazyPreciseMode))
        , m_maskDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->alpha8()))
        , m_smearAlpha(smearAlpha)
        , m_initializationPainter(painter)
    {
        m_projectionDevice = m_overlayDevice->overlay();
    }

    void initializePainting() override
    {
        initializePaintingImpl(m_overlayDevice->overlayColorSpace(),
                               smearCompositeOp(m_smearAlpha),
                               m_initializationPainter->compositeOp()->id());

        m_finalPainter.begin(m_projectionDevice);
        m_finalPainter.setCompositeOp(finalCompositeOp(m_smearAlpha));
        m_finalPainter.setSelection(m_initializationPainter->selection());
        m_finalPainter.setChannelFlags(m_initializationPainter->channelFlags());
        m_finalPainter.copyMirrorInformationFrom(m_initializationPainter);
    }

    virtual QString smearCompositeOp(bool smearAlpha) const
    {
        return smearAlpha ? COMPOSITE_COPY : COMPOSITE_OVER;
    }

    virtual QString finalCompositeOp(bool smearAlpha) const
    {
        Q_UNUSED(smearAlpha);
        return COMPOSITE_COPY;
    }

    void updateMask(KisDabCache *dabCache,
                    const KisPaintInformation& info,
                    const KisDabShape &shape,
                    const QPointF &cursorPoint,
                    QRect *dstDabRect) override {

        static const KoColorSpace* cs = KoColorSpaceRegistry::instance()->alpha8();
        static KoColor color(Qt::black, cs);

        m_maskDab = dabCache->fetchDab(cs,
            color,
            cursorPoint,
            shape,
            info,
            1.0,
            dstDabRect);

        m_shouldPreserveOriginalDab = !dabCache->needSeparateOriginal();
    }

    QVector<QRect> paintDab(const QRect &srcRect, const QRect &dstRect,
                            const QPoint &canvasLocalSamplePoint,
                            const KoColor &currentPaintColor,
                            qreal opacity,
                            qreal colorRateValue,
                            qreal smudgeRateValue,
                            qreal lightnessStrengthValue,
                            qreal smudgeRadiusValue) override
    {
        const QVector<QRect> mirroredRects = m_finalPainter.calculateAllMirroredRects(dstRect);

        QVector<QRect> readRects;
        readRects << mirroredRects;
        readRects << srcRect;
        m_overlayDevice->readRects(readRects);

        blendBrush(&m_finalPainter,
                   m_projectionDevice,
                   m_maskDab,
                   srcRect, dstRect,
                   canvasLocalSamplePoint,
                   currentPaintColor,
                   opacity, smudgeRateValue, colorRateValue, smudgeRadiusValue);

        m_overlayDevice->writeRects(mirroredRects);

        return mirroredRects;
    }

    QScopedPointer<KisOverlayPaintDeviceWrapper> m_overlayDevice;
    KisFixedPaintDeviceSP m_maskDab;
    KisPaintDeviceSP m_projectionDevice;
    KisPainter m_finalPainter;
    bool m_shouldPreserveOriginalDab = true;
    bool m_smearAlpha = true;
    KisPainter *m_initializationPainter = 0;
};

struct ColorSmudgeStrategyStamp : public ColorSmudgeStrategyColorBased
{
    ColorSmudgeStrategyStamp(KisPainter *painter,
                             bool smearAlpha,
                             bool useDullingMode)
        : ColorSmudgeStrategyColorBased(useDullingMode)
        , m_overlayDevice(new KisOverlayPaintDeviceWrapper(painter->device(), 1, KisOverlayPaintDeviceWrapper::LazyPreciseMode))
        , m_maskDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->alpha8()))
        , m_origDab(new KisFixedPaintDevice(m_overlayDevice->overlayColorSpace())) // TODO: check compositionSourceColorSpace!
    {
        initializePaintingImpl(m_overlayDevice->overlayColorSpace(),
                               smearAlpha,
                               painter->compositeOp()->id());

        m_projectionDevice = m_overlayDevice->overlay();

        m_finalPainter.begin(m_projectionDevice);
        m_finalPainter.setCompositeOp(COMPOSITE_COPY);
        m_finalPainter.setSelection(painter->selection());
        m_finalPainter.setChannelFlags(painter->channelFlags());
        m_finalPainter.copyMirrorInformationFrom(painter);

    }

    void updateMask(KisDabCache *dabCache,
                    const KisPaintInformation& info,
                    const KisDabShape &shape,
                    const QPointF &cursorPoint,
                    QRect *dstDabRect) override

    {

        static KoColor color(Qt::black, m_origDab->colorSpace());

        m_origDab = dabCache->fetchDab(m_origDab->colorSpace(),
                                       color,
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

    QVector<QRect> paintDab(const QRect &srcRect, const QRect &dstRect,
                            const QPoint &canvasLocalSamplePoint,
                            const KoColor &currentPaintColor,
                            qreal opacity,
                            qreal colorRateValue,
                            qreal smudgeRateValue,
                            qreal lightnessStrengthValue,
                            qreal smudgeRadiusValue) override
    {
        Q_UNUSED(lightnessStrengthValue);

        const QVector<QRect> mirroredRects = m_finalPainter.calculateAllMirroredRects(dstRect);

        QVector<QRect> readRects;
        readRects << mirroredRects;
        readRects << srcRect;
        m_overlayDevice->readRects(readRects);

        blendBrush(&m_finalPainter,
                   m_projectionDevice,
                   m_maskDab,
                   srcRect, dstRect,
                   canvasLocalSamplePoint,
                   currentPaintColor,
                   opacity, smudgeRateValue, colorRateValue, smudgeRadiusValue);

        m_overlayDevice->writeRects(mirroredRects);

        return mirroredRects;
    }

    void blendInColorRate(quint8 colorRateOpacity,
                          const KoColor &paintColor,
                          KisFixedPaintDeviceSP dstDevice,
                          const QRect dstRect) override
    {
        Q_UNUSED(paintColor);

        // TODO: check correctness for composition source device (transparency masks)
        KIS_ASSERT_RECOVER_RETURN(*dstDevice->colorSpace() == *m_origDab->colorSpace());

        m_colorRateOp->composite(dstDevice->data(), dstRect.width() * dstDevice->pixelSize(),
                                 m_origDab->data(), dstRect.width() * m_origDab->pixelSize(),
                                 0, 0,
                                 dstRect.height(), dstRect.width(),
                                 colorRateOpacity);
    }

    bool supportsFusedDullingBlending() const override {
        return false;
    }

    QScopedPointer<KisOverlayPaintDeviceWrapper> m_overlayDevice;
    KisFixedPaintDeviceSP m_maskDab;
    KisFixedPaintDeviceSP m_origDab;
    KisPaintDeviceSP m_projectionDevice;
    KisPainter m_finalPainter;
    bool m_shouldPreserveOriginalDab = true;
};

struct ColorSmudgeStrategyMaskLegacy : public ColorSmudgeStrategyMask
{
    ColorSmudgeStrategyMaskLegacy(KisPainter *painter,
                                  bool smearAlpha,
                                  bool useDullingMode)
        : ColorSmudgeStrategyMask(painter,
                                  smearAlpha,
                                  useDullingMode)
    {
    }

    void sampleDullingColor(const QRect &srcRect,
                            const QRect &dstRect,
                            qreal sampleRadiusValue,
                            KisPaintDeviceSP sourceDevice,
                            KisFixedPaintDeviceSP tempFixedDevice,
                            KisFixedPaintDeviceSP maskDab,
                            KoColor *resultColor) override
    {
        Q_UNUSED(srcRect);

        ::sampleDullingColor<AveragedSampleWrapper>(dstRect, sampleRadiusValue,
                                                    sourceDevice, tempFixedDevice,
                                                    maskDab, resultColor);
    }

    QString smearCompositeOp(bool smearAlpha) const override
    {
        Q_UNUSED(smearAlpha);
        return COMPOSITE_COPY;
    }

    QString finalCompositeOp(bool smearAlpha) const override
    {
        return smearAlpha ? COMPOSITE_COPY : COMPOSITE_OVER;
    }

    quint8 finalPainterOpacity(qreal opacity, qreal smudgeRateValue) override
    {
        return qBound(OPACITY_TRANSPARENT_U8,
                      quint8(qRound(255.0 *  smudgeRateValue * opacity)),
                      OPACITY_OPAQUE_U8);
    }

    quint8 colorRateOpacity(qreal opacity, qreal smudgeRateValue, qreal colorRateValue) override
    {
        const qreal maxColorRate = qMax<qreal>(1.0 - smudgeRateValue, 0.2);

        return qBound(OPACITY_TRANSPARENT_U8,
                      quint8(qRound(255.0 * KisAlgebra2D::lerp(0.0, maxColorRate, colorRateValue * opacity))),
                      OPACITY_OPAQUE_U8);
    }

    quint8 dullingRateOpacity(qreal opacity, qreal smudgeRateValue) override
    {
        Q_UNUSED(opacity);
        Q_UNUSED(smudgeRateValue);

        return OPACITY_OPAQUE_U8;
    }

    quint8 smearRateOpacity(qreal opacity, qreal smudgeRateValue) override
    {
        Q_UNUSED(opacity);
        Q_UNUSED(smudgeRateValue);

        return OPACITY_OPAQUE_U8;
    }
};

KisColorSmudgeOp::KisColorSmudgeOp(const KisPaintOpSettingsSP settings, KisPainter* painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
    , m_firstRun(true)
    , m_image(image)
    , m_precisePainterWrapper(painter->device(), 1, KisOverlayPaintDeviceWrapper::LazyPreciseMode)
    , m_tempDev(m_precisePainterWrapper.createPreciseCompositionSourceDevice())
    , m_backgroundPainter(new KisPainter(m_tempDev))
    , m_smudgePainter(new KisPainter(m_tempDev))
    , m_overlayPainter(new KisPainter(m_tempDev))
    , m_colorRatePainter(new KisPainter(m_tempDev))
    , m_finalPainter(new KisPainter(m_precisePainterWrapper.overlay()))
    , m_smudgeRateOption()
    , m_colorRateOption("ColorRate", KisPaintOpOption::GENERAL, false)
    , m_smudgeRadiusOption()
    , m_maskDab(new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->alpha8()))
    , m_origDab(new KisFixedPaintDevice(m_tempDev->colorSpace()))
{
    Q_UNUSED(node);

    Q_ASSERT(settings);
    Q_ASSERT(painter);
    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_ratioOption.readOptionSetting(settings);
    m_spacingOption.readOptionSetting(settings);
    m_rateOption.readOptionSetting(settings);
    m_lightnessStrengthOption.readOptionSetting(settings);
    m_smudgeRateOption.readOptionSetting(settings);
    m_colorRateOption.readOptionSetting(settings);

    m_overlayModeOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_scatterOption.readOptionSetting(settings);
    m_gradientOption.readOptionSetting(settings);
    m_airbrushOption.readOptionSetting(settings);

    m_sizeOption.resetAllSensors();
    m_opacityOption.resetAllSensors();
    m_ratioOption.resetAllSensors();
    m_spacingOption.resetAllSensors();
    m_rateOption.resetAllSensors();
    m_lightnessStrengthOption.resetAllSensors();
    m_smudgeRateOption.resetAllSensors();
    m_colorRateOption.resetAllSensors();

    m_rotationOption.resetAllSensors();
    m_scatterOption.resetAllSensors();
    m_gradientOption.resetAllSensors();


    m_gradient = painter->gradient();

    m_backgroundPainter->setCompositeOp(COMPOSITE_COPY);
    // Smudge Painter works in default COMPOSITE_OVER mode
    m_colorRatePainter->setCompositeOp(painter->compositeOp()->id());
    if (m_brush->brushApplication() != ALPHAMASK) { //Forces this, in case the widget somehow doesn't
        m_useNewEngine = true;
    } else {
        m_useNewEngine = m_smudgeRateOption.getUseNewEngine();
    }

    if (m_useNewEngine){
        m_finalPainter->setCompositeOp(COMPOSITE_COPY);
        m_smudgePainter->setCompositeOp(m_smudgeRateOption.getSmearAlpha() ? COMPOSITE_COPY : COMPOSITE_OVER);
        m_smudgeRadiusOption.updateRange(0.0, 1.0);
    } else {
        m_finalPainter->setCompositeOp(m_smudgeRateOption.getSmearAlpha() ? COMPOSITE_COPY : COMPOSITE_OVER);
        m_smudgePainter->setCompositeOp(COMPOSITE_OVER);
        m_smudgeRadiusOption.updateRange(0.0, 3.0);
    }

    // Intialize smudge radius only after the proper range is set
    m_smudgeRadiusOption.readOptionSetting(settings);
    m_smudgeRadiusOption.resetAllSensors();

    m_finalPainter->setSelection(painter->selection());
    m_finalPainter->setChannelFlags(painter->channelFlags());
    m_finalPainter->copyMirrorInformationFrom(painter);

    m_paintColor = painter->paintColor().convertedTo(m_tempDev->colorSpace());
    m_preciseColorRateCompositeOp =
        m_tempDev->colorSpace()->compositeOp(m_colorRatePainter->compositeOp()->id());

    m_hsvOptions.append(KisPressureHSVOption::createHueOption());
    m_hsvOptions.append(KisPressureHSVOption::createSaturationOption());
    m_hsvOptions.append(KisPressureHSVOption::createValueOption());

    Q_FOREACH (KisPressureHSVOption * option, m_hsvOptions) {
        option->readOptionSetting(settings);
        option->resetAllSensors();
        if (option->isChecked() && !m_hsvTransform) {
            m_hsvTransform = m_paintColor.colorSpace()->createColorTransformation("hsv_adjustment", QHash<QString, QVariant>());
        }
    }
    m_rotationOption.applyFanCornersInfo(this);

    if (m_overlayModeOption.isChecked() && m_image && m_image->projection()){
        m_preciseImageDeviceWrapper.reset(new KisOverlayPaintDeviceWrapper(m_image->projection(), 1, KisOverlayPaintDeviceWrapper::LazyPreciseMode));
    }

    const bool useDullingMode = m_smudgeRateOption.getMode() == KisSmudgeOption::DULLING_MODE;

    if (m_useNewEngine && m_brush->brushApplication() == LIGHTNESSMAP) {
        m_strategy.reset(new ColorSmudgeStrategyLightness(painter,
                                                          m_smudgeRateOption.getSmearAlpha(),
                                                          useDullingMode));
    } else if (m_useNewEngine && m_brush->brushApplication() == ALPHAMASK) {
        m_strategy.reset(new ColorSmudgeStrategyMask(painter,
                                                     m_smudgeRateOption.getSmearAlpha(),
                                                     useDullingMode));
    } else if (m_useNewEngine) {
        m_strategy.reset(new ColorSmudgeStrategyStamp(painter,
                                                      m_smudgeRateOption.getSmearAlpha(),
                                                      useDullingMode));
    } else {
        m_strategy.reset(new ColorSmudgeStrategyMaskLegacy(painter,
                                                           m_smudgeRateOption.getSmearAlpha(),
                                                           useDullingMode));
    }

    if (m_strategy) {
        m_strategy->initializePainting();
        m_paintColor.convertTo(m_strategy->preciseColorSpace());
    }
}

KisColorSmudgeOp::~KisColorSmudgeOp()
{
    qDeleteAll(m_hsvOptions);
    delete m_hsvTransform;
}


void KisColorSmudgeOp::updateMask(const KisPaintInformation& info, const KisDabShape &shape, const QPointF &cursorPoint)
{
    qreal lightnessStrength = m_lightnessStrengthOption.apply(info);

    if (!m_useNewEngine) {
        static const KoColorSpace* cs = KoColorSpaceRegistry::instance()->alpha8();
        static KoColor color(Qt::black, cs);

        m_maskDab = m_dabCache->fetchDab(cs,
            color,
            cursorPoint,
            shape,
            info,
            1.0,
            &m_dstDabRect);
    }
    else {
        KoColor color = m_paintColor;
        m_gradientOption.apply(color, m_gradient, info);
        if (m_hsvTransform) {
            Q_FOREACH(KisPressureHSVOption * option, m_hsvOptions) {
                option->apply(m_hsvTransform, info);
            }
            m_hsvTransform->transform(color.data(), color.data(), 1);
        }

        m_origDab = m_dabCache->fetchDab(m_tempDev->colorSpace(),
            color,
            cursorPoint,
            shape,
            info,
            1.0,
            &m_dstDabRect,
            lightnessStrength);

        m_maskDab->setRect(m_origDab->bounds());
        m_maskDab->initialize();
        int numPixels = m_maskDab->bounds().width() * m_maskDab->bounds().height();
        m_origDab->colorSpace()->copyOpacityU8(m_origDab->data(), m_maskDab->data(), numPixels);
    }


    // sanity check
    KIS_ASSERT_RECOVER_NOOP(m_dstDabRect.size() == m_maskDab->bounds().size());
}


inline void KisColorSmudgeOp::getTopLeftAligned(const QPointF &pos, const QPointF &hotSpot, qint32 *x, qint32 *y)
{
    QPointF topLeft = pos - hotSpot;

    qreal xFraction, yFraction; // will not be used
    splitCoordinate(topLeft.x(), x, &xFraction);
    splitCoordinate(topLeft.y(), y, &yFraction);
}

KoColor KisColorSmudgeOp::getOverlayDullingFillColor(QPoint canvasLocalSamplePoint) {
    // stored in the color space of the paintColor
    KoColor dullingFillColor = m_paintColor;
    // get the pixel on the canvas that lies beneath the hot spot
    // of the dab and fill  the temporary paint device with that color
    KisCrossDeviceColorSamplerInt colorPicker(m_tempDev, dullingFillColor);
    colorPicker.sampleColor(canvasLocalSamplePoint.x(), canvasLocalSamplePoint.y(), dullingFillColor.data());
    KIS_SAFE_ASSERT_RECOVER_NOOP(*dullingFillColor.colorSpace() == *m_tempDev->colorSpace());

    return dullingFillColor;
}

KoColor KisColorSmudgeOp::getDullingFillColor(const KisPaintInformation& info, KisOverlayPaintDeviceWrapper& activeWrapper, QPoint canvasLocalSamplePoint) {
    // stored in the color space of the paintColor
    KoColor dullingFillColor = m_paintColor;
    if (m_smudgeRadiusOption.isChecked()) {
        const qreal effectiveSize = 0.5 * (m_dstDabRect.width() + m_dstDabRect.height());

        const QRect sampleRect = m_smudgeRadiusOption.sampleRect(info, effectiveSize, canvasLocalSamplePoint);
        activeWrapper.readRect(sampleRect);

        m_smudgeRadiusOption.apply(&dullingFillColor, info, effectiveSize, canvasLocalSamplePoint.x(), canvasLocalSamplePoint.y(), activeWrapper.overlay());
        KIS_SAFE_ASSERT_RECOVER_NOOP(*dullingFillColor.colorSpace() == *m_tempDev->colorSpace());
    }
    else {
        // get the pixel on the canvas that lies beneath the hot spot
        // of the dab and fill  the temporary paint device with that color
        activeWrapper.readRect(QRect(canvasLocalSamplePoint, QSize(1, 1)));
        KisCrossDeviceColorSamplerInt colorPicker(activeWrapper.overlay(), dullingFillColor);
        colorPicker.sampleColor(canvasLocalSamplePoint.x(), canvasLocalSamplePoint.y(), dullingFillColor.data());
        KIS_SAFE_ASSERT_RECOVER_NOOP(*dullingFillColor.colorSpace() == *m_tempDev->colorSpace());
    }

    return dullingFillColor;
}

void KisColorSmudgeOp::mixSmudgePaintAt(const KisPaintInformation& info, KisOverlayPaintDeviceWrapper& activeWrapper, QRect srcDabRect, QPoint canvasLocalSamplePoint, bool useDullingMode)
{
    KisBrushSP brush = m_brush;

    QRect dabBounds = m_origDab->bounds();
    qint32 width = dabBounds.width();
    qint32 height = dabBounds.height();

    const KoColorSpace* preciseCS = m_tempDev->colorSpace();

    const qreal colorRate = m_colorRateOption.isChecked() ? m_colorRateOption.computeSizeLikeValue(info) : 0.0;
    const qreal smudgeLength = m_smudgeRateOption.isChecked() ? m_smudgeRateOption.computeSizeLikeValue(info) : 1.0;
    const qreal fpOpacity = m_opacityOption.getOpacityf(info);

    const int colorAlpha = qRound(colorRate * colorRate * fpOpacity * 255.0);
    const int smudgeAlpha = qRound(smudgeLength * fpOpacity * 255.0);
    const int dullingAlpha = qRound(smudgeLength * 0.8 * fpOpacity * 255.0);

    m_precisePainterWrapper.readRect(m_dstDabRect); //copy the current data in the destination
    m_backgroundPainter->bitBlt(QPoint(), m_precisePainterWrapper.overlay(), m_dstDabRect);


    activeWrapper.readRect(srcDabRect);
    if (m_image && m_overlayModeOption.isChecked()) {
        m_smudgePainter->setOpacity(smudgeAlpha);
        m_image->blockUpdates();
        m_smudgePainter->bitBlt(QPoint(), m_image->projection(), srcDabRect);
        m_image->unblockUpdates();
        m_overlayPainter->setOpacity(smudgeAlpha);
        m_overlayPainter->bitBlt(QPoint(), activeWrapper.overlay(), srcDabRect); //necessary because image->projection doesn't update each frame

        if (useDullingMode) {
            KoColor dullingFillColor = m_smudgeRadiusOption.isChecked() ?
                getDullingFillColor(info, activeWrapper, canvasLocalSamplePoint) :
                getOverlayDullingFillColor(canvasLocalSamplePoint - srcDabRect.topLeft());
            dullingFillColor.convertTo(preciseCS); //convert to mix with background
            m_smudgePainter->setOpacity(dullingAlpha);
            m_smudgePainter->fill(0, 0, m_dstDabRect.width(), m_dstDabRect.height(), dullingFillColor);
        }

    } else {
        if (useDullingMode) {
            KoColor dullingFillColor = getDullingFillColor(info, activeWrapper, canvasLocalSamplePoint);
            dullingFillColor.convertTo(preciseCS); //convert to mix with background
            m_smudgePainter->setOpacity(dullingAlpha);
            m_smudgePainter->fill(0, 0, m_dstDabRect.width(), m_dstDabRect.height(), dullingFillColor);
        } else {
            m_smudgePainter->setOpacity(smudgeAlpha);
            m_smudgePainter->bitBlt(QPoint(), activeWrapper.overlay(), srcDabRect);
        }
    }

    m_colorRatePainter->setOpacity(colorAlpha);
    m_colorRatePainter->bltFixed(0, 0, m_origDab, 0, 0, width, height);

    m_precisePainterWrapper.readRects(m_finalPainter->calculateAllMirroredRects(m_dstDabRect));

    // then blit the temporary painting device on the canvas at the current brush position
    // the alpha mask (maskDab) will be used here to only blit the pixels that are in the area (shape) of the brush

    m_finalPainter->bitBltWithFixedSelection(m_dstDabRect.x(), m_dstDabRect.y(), m_tempDev, m_maskDab, m_dstDabRect.width(), m_dstDabRect.height());
    m_finalPainter->renderMirrorMaskSafe(m_dstDabRect, m_tempDev, 0, 0, m_maskDab, !m_dabCache->needSeparateOriginal());

    const QVector<QRect> dirtyRects = m_finalPainter->takeDirtyRegion();
    m_precisePainterWrapper.writeRects(dirtyRects);
    painter()->addDirtyRects(dirtyRects);

}

KisSpacingInformation KisColorSmudgeOp::paintAt(const KisPaintInformation& info)
{
    KisBrushSP brush = m_brush;
    const bool useDullingMode = m_smudgeRateOption.getMode() == KisSmudgeOption::DULLING_MODE;

    /* This is a fix for dulling + overlay + paint,
     * this should allow the image to composite paint addition effects correctly
     * while also respecting overlay mode. */
    bool useAlternatePrecisionSource = (m_overlayModeOption.isChecked() &&
                                        useDullingMode &&
                                        m_preciseImageDeviceWrapper);

    KisOverlayPaintDeviceWrapper &activeWrapper = useAlternatePrecisionSource ? *m_preciseImageDeviceWrapper :
                                                                                 m_precisePainterWrapper;

    // Simple error catching
    if (!painter()->device() || !brush || !brush->canPaintFor(info)) {
        return KisSpacingInformation(1.0);
    }

    if (m_smudgeRateOption.getMode() == KisSmudgeOption::SMEARING_MODE) {
        /**
        * Disable handling of the subpixel precision. In the smudge op we
        * should read from the aligned areas of the image, so having
        * additional internal offsets, created by the subpixel precision,
        * will worsen the quality (at least because
        * QRectF(m_dstDabRect).center() will not point to the real center
        * of the brush anymore).
        * Of course, this only really matters with smearing_mode (bug:327235),
        * and you only notice the lack of subpixel precision in the dulling methods.
        */
        m_dabCache->disableSubpixelPrecision();
    }

#if 0
    //if precision
    KoColor colorSpaceChanger = painter()->paintColor();
    const KoColorSpace* preciseColorSpace = colorSpaceChanger.colorSpace();
    if (colorSpaceChanger.colorSpace()->colorDepthId().id() == "U8") {
    preciseColorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceChanger.colorSpace()->colorModelId().id(), "U16", colorSpaceChanger.profile() );
        colorSpaceChanger.convertTo(preciseColorSpace);
    }
    painter()->setPaintColor(colorSpaceChanger);
#endif

    // get the scaling factor calculated by the size option
    qreal scale = m_sizeOption.apply(info);
    scale *= KisLodTransform::lodToScale(painter()->device());
    qreal rotation = m_rotationOption.apply(info);
    if (checkSizeTooSmall(scale)) return KisSpacingInformation();

    qreal ratio = m_ratioOption.apply(info);

    KisDabShape shape(scale, ratio, rotation);

    QPointF scatteredPos =
        m_scatterOption.apply(info,
                              brush->maskWidth(shape, 0, 0, info),
                              brush->maskHeight(shape, 0, 0, info));

    QPointF hotSpot = brush->hotSpot(shape, info);

    const qreal smudgeRadiusPortion = m_smudgeRadiusOption.isChecked() ? m_smudgeRadiusOption.computeSizeLikeValue(info) / 100.0 : 0.0;

    if (m_strategy) {
        m_strategy->updateMask(m_dabCache, info, shape, scatteredPos, &m_dstDabRect);

        QPointF newCenterPos = QRectF(m_dstDabRect).center();
        /**
         * Save the center of the current dab to know where to read the
         * data during the next pass. We do not save scatteredPos here,
         * because it may differ slightly from the real center of the
         * brush (due to rounding effects), which will result in a
         * really weird quality.
         */
        QRect srcDabRect = m_dstDabRect.translated((m_lastPaintPos - newCenterPos).toPoint());

        m_lastPaintPos = newCenterPos;


        KisSpacingInformation spacingInfo =
            effectiveSpacing(scale, rotation,
                             &m_airbrushOption, &m_spacingOption, info);

        if (m_firstRun) {
            m_firstRun = false;
            return spacingInfo;
        }

        QPoint canvasLocalSamplePoint = (srcDabRect.topLeft() + hotSpot).toPoint();

        const qreal colorRate = m_colorRateOption.isChecked() ? m_colorRateOption.computeSizeLikeValue(info) : 0.0;
        const qreal smudgeLength = m_smudgeRateOption.isChecked() ? m_smudgeRateOption.computeSizeLikeValue(info) : 1.0;
        const qreal fpOpacity = m_opacityOption.getOpacityf(info);
        const qreal lightnessStrength = m_lightnessStrengthOption.apply(info);

        KoColor paintColor = m_paintColor;
        m_gradientOption.apply(paintColor, m_gradient, info);
        if (m_hsvTransform) {
            Q_FOREACH (KisPressureHSVOption * option, m_hsvOptions) {
                option->apply(m_hsvTransform, info);
            }
            m_hsvTransform->transform(paintColor.data(), paintColor.data(), 1);
        }

        const QVector<QRect> dirtyRects =
            m_strategy->paintDab(srcDabRect, m_dstDabRect,
                                 canvasLocalSamplePoint,
                                 paintColor,
                                 fpOpacity, colorRate, smudgeLength,
                                 lightnessStrength,
                                 smudgeRadiusPortion);

        painter()->addDirtyRects(dirtyRects);

        return spacingInfo;
    }

    /**
     * Update the brush mask.
     *
     * Upon leaving the function:
     *   o m_maskDab stores the new mask
     *   o m_maskBounds stores the extents of the mask paint device
     *   o m_dstDabRect stores the destination rect where the mask is going
     *     to be written to
     */
    updateMask(info, shape, scatteredPos);

    QPointF newCenterPos = QRectF(m_dstDabRect).center();
    /**
     * Save the center of the current dab to know where to read the
     * data during the next pass. We do not save scatteredPos here,
     * because it may differ slightly from the real center of the
     * brush (due to rounding effects), which will result in a
     * really weird quality.
     */
    QRect srcDabRect = m_dstDabRect.translated((m_lastPaintPos - newCenterPos).toPoint());

    m_lastPaintPos = newCenterPos;

    KisSpacingInformation spacingInfo =
        effectiveSpacing(scale, rotation,
                         &m_airbrushOption, &m_spacingOption, info);

    if (m_firstRun) {
        m_firstRun = false;
        return spacingInfo;
    }

    QPoint canvasLocalSamplePoint = (srcDabRect.topLeft() + hotSpot).toPoint();

    if (m_useNewEngine) {
        mixSmudgePaintAt(info, activeWrapper, srcDabRect, canvasLocalSamplePoint, useDullingMode);
        return spacingInfo;
    }

    const qreal fpOpacity = (qreal(painter()->opacity()) / 255.0) * m_opacityOption.getOpacityf(info);

    if (m_image && m_overlayModeOption.isChecked()) {
        m_image->blockUpdates();
        m_backgroundPainter->bitBlt(QPoint(), m_image->projection(), srcDabRect);
        m_image->unblockUpdates();
    }
    else {
        // IMPORTANT: Clear the temporary painting device to transparent black.
        //            It will only clear the extents of the brush.
        m_tempDev->clear(QRect(QPoint(), m_dstDabRect.size()));
    }

    // stored in the color space of the paintColor
    KoColor dullingFillColor = m_paintColor;

    if (!useDullingMode) {
        activeWrapper.readRect(srcDabRect);
        m_smudgePainter->bitBlt(QPoint(), activeWrapper.overlay(), srcDabRect);
    } else {
        dullingFillColor = getDullingFillColor(info, activeWrapper, canvasLocalSamplePoint);
    }

    // if the user selected the color smudge option,
    // we will mix some color into the temporary painting device (m_tempDev)
    if (m_colorRateOption.isChecked()) {
        // this will apply the opacity (selected by the user) to copyPainter
        // (but fit the rate inbetween the range 0.0 to (1.0-SmudgeRate))
        qreal maxColorRate = qMax<qreal>(1.0 - m_smudgeRateOption.getRate(), 0.2);
        m_colorRateOption.apply(*m_colorRatePainter, info, 0.0, maxColorRate, fpOpacity);

        // paint a rectangle with the current color (foreground color)
        // or a gradient color (if enabled)
        // into the temporary painting device and use the user selected
        // composite mode
        KoColor color = m_paintColor;
        m_gradientOption.apply(color, m_gradient, info);
        if (m_hsvTransform) {
            Q_FOREACH (KisPressureHSVOption * option, m_hsvOptions) {
                option->apply(m_hsvTransform, info);
            }
            m_hsvTransform->transform(color.data(), color.data(), 1);
        }

        if (!useDullingMode) {
            KIS_SAFE_ASSERT_RECOVER(*m_colorRatePainter->device()->colorSpace() == *color.colorSpace()) {
                color.convertTo(m_colorRatePainter->device()->colorSpace());
            }

            m_colorRatePainter->fill(0, 0, m_dstDabRect.width(), m_dstDabRect.height(), color);
        } else {
            KIS_SAFE_ASSERT_RECOVER(*dullingFillColor.colorSpace() == *color.colorSpace()) {
                color.convertTo(dullingFillColor.colorSpace());
            }
            KIS_SAFE_ASSERT_RECOVER_NOOP(*dullingFillColor.colorSpace() == *m_tempDev->colorSpace());
            m_preciseColorRateCompositeOp->composite(dullingFillColor.data(), 0,
                                                     color.data(), 0,
                                                     0, 0,
                                                     1, 1,
                                                     m_colorRatePainter->opacity());
        }
    }

    if (useDullingMode) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(*dullingFillColor.colorSpace() == *m_tempDev->colorSpace());
        m_tempDev->fill(QRect(0, 0, m_dstDabRect.width(), m_dstDabRect.height()), dullingFillColor);
    }

    m_precisePainterWrapper.readRects(m_finalPainter->calculateAllMirroredRects(m_dstDabRect));

    // if color is disabled (only smudge) and "overlay mode" is enabled
    // then first blit the region under the brush from the image projection
    // to the painting device to prevent a rapid build up of alpha value
    // if the color to be smudged is semi transparent.
    if (m_image && m_overlayModeOption.isChecked() && !m_colorRateOption.isChecked()) {
        m_finalPainter->setOpacity(OPACITY_OPAQUE_U8);
        m_image->blockUpdates();
        // TODO: check if this code is correct in mirrored mode! Technically, the
        //       painter renders the mirrored dab only, so we should also prepare
        //       the overlay for it in all the places.
        m_finalPainter->bitBlt(m_dstDabRect.topLeft(), m_image->projection(), m_dstDabRect);
        m_image->unblockUpdates();
    }

    // set opacity calculated by the rate option
    m_smudgeRateOption.apply(*m_finalPainter, info, 0.0, 1.0, fpOpacity);

    // then blit the temporary painting device on the canvas at the current brush position
    // the alpha mask (maskDab) will be used here to only blit the pixels that are in the area (shape) of the brush
    m_finalPainter->bitBltWithFixedSelection(m_dstDabRect.x(), m_dstDabRect.y(), m_tempDev, m_maskDab, m_dstDabRect.width(), m_dstDabRect.height());
    m_finalPainter->renderMirrorMaskSafe(m_dstDabRect, m_tempDev, 0, 0, m_maskDab, !m_dabCache->needSeparateOriginal());

    const QVector<QRect> dirtyRects = m_finalPainter->takeDirtyRegion();
    m_precisePainterWrapper.writeRects(dirtyRects);
    painter()->addDirtyRects(dirtyRects);

    return spacingInfo;

}

KisSpacingInformation KisColorSmudgeOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    const qreal scale = m_sizeOption.apply(info) * KisLodTransform::lodToScale(painter()->device());
    const qreal rotation = m_rotationOption.apply(info);
    return effectiveSpacing(scale, rotation, &m_airbrushOption, &m_spacingOption, info);
}


KisTimingInformation KisColorSmudgeOp::updateTimingImpl(const KisPaintInformation &info) const
{
    return KisPaintOpPluginUtils::effectiveTiming(&m_airbrushOption, &m_rateOption, info);
}

KisInterstrokeDataFactory *KisColorSmudgeOp::createInterstrokeDataFactory(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface)
{
    bool needsInterstrokeData =
        settings->getBool(QString("SmudgeRate") + "UseNewEngine", false);

    if (!needsInterstrokeData) return 0;

    KisBrushOptionProperties brushOption;
    needsInterstrokeData &=
        brushOption.brushApplication(settings.data(), resourcesInterface) == LIGHTNESSMAP;

    return needsInterstrokeData ? new ColorSmudgeInterstrokeDataFactory() : 0;
}
