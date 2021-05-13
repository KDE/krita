/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KRITA_KISCOLORSMUDGESAMPLEUTILS_H
#define KRITA_KISCOLORSMUDGESAMPLEUTILS_H

#include "kis_fixed_paint_device.h"
#include "KoMixColorsOp.h"
#include "kis_algebra_2d.h"

#include <KoColor.h>

#include "kis_fixed_paint_device.h"
#include "KisColorSmudgeSource.h"


namespace KisColorSmudgeSampleUtils {

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


    bool shouldRestartWithBiggerRadius() const {
        // if all the pixels we sampled appeared to be masked out
        // we should ask the sampling algorithm to restart with
        // bigger sampling radius

        return m_mixer->currentWeightsSum() < 128;
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

    bool shouldRestartWithBiggerRadius() const {
        return false;
    }

    KoMixColorsOp::Mixer *m_mixer;
    int m_samplePixelSize;
    const QRect m_sampleRect;
    quint8 *m_samplePtr;
    const int m_sampleStride;
};

/**
 * Sample color from \p srcRect in weighted way
 *
 * The function samples \p srcRect in an efficient way. It samples "random"
 * pixels using Halton sequence and waits until the sampled color "converges"
 * to stable value. The sampled area is defined by \p sampleRadiusValue.
 *
 * The way of weighting the pixels is defined by \p WeightingModeWrapper
 * template policy. If `WeightedSampleWrapper` is used, then the sampled color
 * is weighted by the passed brush mask in \p maskDab. If
 * `AveragedSampleWrapper` is used, then \p maskDab is **not** used and all
 * sampled pixels are averaged in a uniform way.
 *
 * \param sampleRadiusValue defines how many pixels are sampled. When
 * \p sampleRadiusValue is 0.0, only the central pixel is sampled. When
 * \p sampleRadiusValue is 1.0, the entire range of \p srcRect is sampled.
 * If `AveragedSampleWrapper` is used, then \p sampleRadiusValue may be
 * increased up to 3.0 to sample outside \p srcRect.
 *
 * When `WeightedSampleWrapper` is used, the sampler may sample more pixels
 * than actually requested by \p sampleRadiusValue. It may happen if all the
 * pixels in the sample radius area are masked out by \p maskDab.
 *
 * \param tempFixedDevice is a temporary device that may be used by the
 * function for internal purposes.
 */
template<class WeightingModeWrapper>
void sampleColor(const QRect &srcRect,
                 qreal sampleRadiusValue,
                 KisColorSmudgeSourceSP sourceDevice,
                 KisFixedPaintDeviceSP tempFixedDevice,
                 KisFixedPaintDeviceSP maskDab,
                 KoColor *resultColor)

{
    WeightingModeWrapper::verifySampleRadiusValue(&sampleRadiusValue);

    KIS_ASSERT_RECOVER_RETURN(*resultColor->colorSpace() == *sourceDevice->colorSpace());
    KIS_ASSERT_RECOVER_RETURN(*tempFixedDevice->colorSpace() == *sourceDevice->colorSpace());

    const QRect minimalRect = QRect(srcRect.center(), QSize(1,1));

    do {
        const QRect sampleRect = sampleRadiusValue > 0 ?
                    KisAlgebra2D::blowRect(srcRect, 0.5 * (sampleRadiusValue - 1.0)) | minimalRect :
                    minimalRect;

        tempFixedDevice->setRect(sampleRect);
        tempFixedDevice->lazyGrowBufferWithoutInitialization();

        const KoColorSpace *cs = tempFixedDevice->colorSpace();
        const int numPixels = sampleRect.width() * sampleRect.height();
        sourceDevice->readRect(sampleRect);
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
            const QPoint pt(hGen.generate(sampleRect.width() - 1),
                            vGen.generate(sampleRect.height() - 1));

            weightingModeWrapper.samplePixel(pt);
        }

        mixer->computeMixedColor(resultColor->data());
        lastPickedColor = *resultColor;

        const int batchSize = 16;
        int numSamplesLeft = numPixels - minSamples;

        while (numSamplesLeft > 0) {
            const int currentBatchSize = qMin(numSamplesLeft, batchSize);
            for (int i = 0; i < currentBatchSize; i++) {
                const QPoint pt(hGen.generate(sampleRect.width() - 1),
                                vGen.generate(sampleRect.height() - 1));

                weightingModeWrapper.samplePixel(pt);
            }

            mixer->computeMixedColor(resultColor->data());

            const quint8 difference =
                    cs->differenceA(resultColor->data(), lastPickedColor.data());

            if (difference <= 2) break;

            lastPickedColor = *resultColor;
            numSamplesLeft -= currentBatchSize;
        }

        if (!weightingModeWrapper.shouldRestartWithBiggerRadius() || sampleRadiusValue >= 1.0) {
            break;
        }

        sampleRadiusValue = qMin(1.0, sampleRadiusValue + 0.05);

    } while (1);
}

}


#endif //KRITA_KISCOLORSMUDGESAMPLEUTILS_H
