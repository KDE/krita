/*
 *  SPDX-FileCopyrightText: 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_BRUSH_VECTOR_APPLICATOR_H
#define KIS_BRUSH_VECTOR_APPLICATOR_H

#include <config-vc.h>

#if defined HAVE_VC

#include <KoVcMultiArchBuildSupport.h>

#include "kis_brush_mask_scalar_applicator.h"

template<class V>
struct FastRowProcessor {
    FastRowProcessor(V *maskGenerator)
        : d(maskGenerator->d.data())
    {
    }

    template<Vc::Implementation _impl>
    void process(float *buffer, int width, float y, float cosa, float sina, float centerX, float centerY);

    typename V::Private *d;
};

template<class MaskGenerator, Vc::Implementation _impl>
struct KisBrushMaskVectorApplicator : public KisBrushMaskScalarApplicator<MaskGenerator, _impl> {
    KisBrushMaskVectorApplicator(MaskGenerator *maskGenerator)
        : KisBrushMaskScalarApplicator<MaskGenerator, _impl>(maskGenerator)
    {
    }

    void process(const QRect &rect) override
    {
        startProcessing(rect, TypeHelper<MaskGenerator, _impl>());
    }

protected:
    void processVector(const QRect &rect);

private:
    template<class U, Vc::Implementation V>
    struct TypeHelper {
    };

private:
    template<class U>
    inline void startProcessing(const QRect &rect, TypeHelper<U, Vc::ScalarImpl>)
    {
        KisBrushMaskScalarApplicator<MaskGenerator, _impl>::processScalar(rect);
    }

    template<class U, Vc::Implementation V>
    inline void startProcessing(const QRect &rect, TypeHelper<U, V>)
    {
        MaskGenerator *m_maskGenerator = KisBrushMaskScalarApplicator<MaskGenerator, _impl>::m_maskGenerator;

        if (m_maskGenerator->shouldVectorize()) {
            processVector(rect);
        } else {
            KisBrushMaskScalarApplicator<MaskGenerator, _impl>::processScalar(rect);
        }
    }
};

template<class MaskGenerator, Vc::Implementation impl>
void KisBrushMaskVectorApplicator<MaskGenerator, impl>::processVector(const QRect &rect)
{
    const MaskProcessingData *m_d = KisBrushMaskApplicatorBase::m_d;
    MaskGenerator *m_maskGenerator = KisBrushMaskScalarApplicator<MaskGenerator, impl>::m_maskGenerator;

    qreal random = 1.0;
    quint8 *dabPointer = m_d->device->data() + rect.y() * rect.width() * m_d->pixelSize;
    quint8 alphaValue = OPACITY_TRANSPARENT_U8;
    // this offset is needed when brush size is smaller then fixed device size
    int offset = (m_d->device->bounds().width() - rect.width()) * m_d->pixelSize;

    int width = rect.width();

    // We need to calculate with a multiple of the width of the simd register
    size_t alignOffset = 0;
    if (width % Vc::float_v::size() != 0) {
        alignOffset = Vc::float_v::size() - (width % Vc::float_v::size());
    }
    size_t simdWidth = width + alignOffset;

    float *buffer = Vc::malloc<float, Vc::AlignOnCacheline>(simdWidth);

    FastRowProcessor<MaskGenerator> processor(m_maskGenerator);

    for (int y = rect.y(); y < rect.y() + rect.height(); y++) {
        processor.template process<impl>(buffer, simdWidth, y, m_d->cosa, m_d->sina, m_d->centerX, m_d->centerY);

        if (m_d->randomness != 0.0 || m_d->density != 1.0) {
            for (int x = 0; x < width; x++) {
                if (m_d->randomness != 0.0) {
                    random = (1.0 - m_d->randomness)
                        + m_d->randomness
                            * KisBrushMaskScalarApplicator<MaskGenerator, impl>::m_randomSource.generateNormalized();
                }

                alphaValue = quint8((OPACITY_OPAQUE_U8 - buffer[x] * 255) * random);

                // avoid computation of random numbers if density is full
                if (m_d->density != 1.0) {
                    // compute density only for visible pixels of the mask
                    if (alphaValue != OPACITY_TRANSPARENT_U8) {
                        if (!(m_d->density >= KisBrushMaskScalarApplicator<MaskGenerator, impl>::m_randomSource
                                                  .generateNormalized())) {
                            alphaValue = OPACITY_TRANSPARENT_U8;
                        }
                    }
                }

                if (m_d->color) {
                    memcpy(dabPointer, m_d->color, m_d->pixelSize);
                }

                m_d->colorSpace->applyAlphaU8Mask(dabPointer, &alphaValue, 1);
                dabPointer += m_d->pixelSize;
            }
        } else if (m_d->color) {
            m_d->colorSpace->fillInverseAlphaNormedFloatMaskWithColor(dabPointer, buffer, m_d->color, width);
            dabPointer += width * m_d->pixelSize;
        } else {
            m_d->colorSpace->applyInverseNormedFloatMask(dabPointer, buffer, width);
            dabPointer += width * m_d->pixelSize;
        } // endfor x
        dabPointer += offset;
    } // endfor y
    Vc::free(buffer);
}

#endif /* defined HAVE_VC */

#endif /* KIS_BRUSH_VECTOR_APPLICATOR_H */
