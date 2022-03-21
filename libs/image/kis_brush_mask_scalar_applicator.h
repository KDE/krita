/*
 *  SPDX-FileCopyrightText: 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BRUSH_SCALAR_APPLICATOR_H
#define KIS_BRUSH_SCALAR_APPLICATOR_H

#include "kis_brush_mask_applicator_base.h"
#include "kis_global.h"
#include "kis_random_source.h"

// 3x3 supersampling
#define SUPERSAMPLING 3

template<class MaskGenerator, Vc::Implementation impl>
struct KisBrushMaskScalarApplicator : public KisBrushMaskApplicatorBase {
    KisBrushMaskScalarApplicator(MaskGenerator *maskGenerator)
        : m_maskGenerator(maskGenerator)
    {
    }

    void process(const QRect &rect) override
    {
        processScalar(rect);
    }

protected:
    void processScalar(const QRect &rect)
    {
        const MaskProcessingData *m_d = KisBrushMaskApplicatorBase::m_d;
        MaskGenerator *m_maskGenerator = KisBrushMaskScalarApplicator<MaskGenerator, impl>::m_maskGenerator;

        qreal random = 1.0;
        quint8 *dabPointer = m_d->device->data() + rect.y() * rect.width() * m_d->pixelSize;
        quint8 alphaValue = OPACITY_TRANSPARENT_U8;
        // this offset is needed when brush size is smaller then fixed device size
        int offset = (m_d->device->bounds().width() - rect.width()) * m_d->pixelSize;
        int supersample = (m_maskGenerator->shouldSupersample() ? SUPERSAMPLING : 1);
        double invss = 1.0 / supersample;
        int samplearea = pow2(supersample);
        for (int y = rect.y(); y < rect.y() + rect.height(); y++) {
            for (int x = rect.x(); x < rect.x() + rect.width(); x++) {
                int value = 0;
                for (int sy = 0; sy < supersample; sy++) {
                    for (int sx = 0; sx < supersample; sx++) {
                        double x_ = x + sx * invss - m_d->centerX;
                        double y_ = y + sy * invss - m_d->centerY;
                        double maskX = m_d->cosa * x_ - m_d->sina * y_;
                        double maskY = m_d->sina * x_ + m_d->cosa * y_;
                        value += m_maskGenerator->valueAt(maskX, maskY);
                    }
                }
                if (supersample != 1)
                    value /= samplearea;

                if (m_d->randomness != 0.0) {
                    random = (1.0 - m_d->randomness) + m_d->randomness * m_randomSource.generateNormalized();
                }

                alphaValue = quint8((OPACITY_OPAQUE_U8 - value) * random);

                // avoid computation of random numbers if density is full
                if (m_d->density != 1.0) {
                    // compute density only for visible pixels of the mask
                    if (alphaValue != OPACITY_TRANSPARENT_U8) {
                        if (!(m_d->density >= m_randomSource.generateNormalized())) {
                            alphaValue = OPACITY_TRANSPARENT_U8;
                        }
                    }
                }

                if (m_d->color) {
                    memcpy(dabPointer, m_d->color, static_cast<size_t>(m_d->pixelSize));
                }

                m_d->colorSpace->applyAlphaU8Mask(dabPointer, &alphaValue, 1);
                dabPointer += m_d->pixelSize;
            } // endfor x
            dabPointer += offset;
        } // endfor y
    }

protected:
    MaskGenerator *m_maskGenerator;
    KisRandomSource m_randomSource; // TODO: make it more deterministic for LoD
};

#endif /* KIS_BRUSH_SCALAR_APPLICATOR_H */
