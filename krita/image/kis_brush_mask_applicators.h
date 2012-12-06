/*
 *  Copyright (c) 2012 Sven Langkamp  <sven.langkamp@gmail.com>
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_BRUSH_MASK_APPLICATORS_H
#define __KIS_BRUSH_MASK_APPLICATORS_H

#include "kis_brush_mask_applicator_base.h"

// 3x3 supersampling
#define SUPERSAMPLING 3

#if defined(_WIN32) || defined(_WIN64)
#include <stdlib.h>
#define srand48 srand
inline double drand48() {
    return double(rand()) / RAND_MAX;
}
#endif


template<class MaskGenerator, Vc::Implementation _impl>
struct KisBrushMaskScalarApplicator : public KisBrushMaskApplicatorBase
{
    KisBrushMaskScalarApplicator(MaskGenerator *maskGenerator)
        : m_maskGenerator(maskGenerator)
    {
    }

    void process(const QRect &rect) {
        processScalar(rect);
    }

protected:
    void processScalar(const QRect &rect);

protected:
    MaskGenerator *m_maskGenerator;
};

#if defined HAVE_VC

template<class MaskGenerator, Vc::Implementation _impl>
struct KisBrushMaskVectorApplicator : public KisBrushMaskScalarApplicator<MaskGenerator, _impl>
{
    KisBrushMaskVectorApplicator(MaskGenerator *maskGenerator)
        : KisBrushMaskScalarApplicator<MaskGenerator, _impl>(maskGenerator)
    {
    }

    void process(const QRect &rect) {
        startProcessing(rect, TypeHelper<MaskGenerator, _impl>());
    }

protected:
    void processVector(const QRect &rect);

private:
    template<class U, Vc::Implementation V> struct TypeHelper {};

private:
    template<class U>
    inline void startProcessing(const QRect &rect, TypeHelper<U, Vc::ScalarImpl>) {
        KisBrushMaskScalarApplicator<MaskGenerator, _impl>::processScalar(rect);
    }

    template<class U, Vc::Implementation V>
    inline void startProcessing(const QRect &rect, TypeHelper<U, V>) {
        MaskGenerator *m_maskGenerator = KisBrushMaskScalarApplicator<MaskGenerator, _impl>::m_maskGenerator;

        if (m_maskGenerator->shouldVectorize()) {
            processVector(rect);
        } else {
            KisBrushMaskScalarApplicator<MaskGenerator, _impl>::processScalar(rect);
        }
    }
};

template<class MaskGenerator, Vc::Implementation _impl>
void KisBrushMaskVectorApplicator<MaskGenerator, _impl>::processVector(const QRect &rect)
{
    const MaskProcessingData *m_d = KisBrushMaskApplicatorBase::m_d;
    MaskGenerator *m_maskGenerator = KisBrushMaskScalarApplicator<MaskGenerator, _impl>::m_maskGenerator;

    qreal random = 1.0;
    quint8* dabPointer = m_d->device->data() + rect.y() * rect.width() * m_d->pixelSize;
    quint8 alphaValue = OPACITY_TRANSPARENT_U8;
    // this offset is needed when brush size is smaller then fixed device size
    int offset = (m_d->device->bounds().width() - rect.width()) * m_d->pixelSize;

    int width = rect.width();

    // We need to calculate with a multiple of the width of the simd register
    int alignOffset = 0;
    if (width % Vc::float_v::Size != 0) {
        alignOffset = Vc::float_v::Size - (width % Vc::float_v::Size);
    }
    int simdWidth = width + alignOffset;

    float *buffer = Vc::malloc<float, Vc::AlignOnVector>(simdWidth);

    typename MaskGenerator::FastRowProcessor processor(m_maskGenerator);

    for (int y = rect.y(); y < rect.y() + rect.height(); y++) {

        processor.template process<_impl>(buffer, simdWidth, y, m_d->cosa, m_d->sina, m_d->centerX, m_d->centerY, m_d->invScaleX, m_d->invScaleY);

        if (m_d->randomness != 0.0 || m_d->density != 1.0) {
            for (int x = 0; x < width; x++) {

                if (m_d->randomness!= 0.0){
                    random = (1.0 - m_d->randomness) + m_d->randomness * float(rand()) / RAND_MAX;
                }

                alphaValue = quint8( (OPACITY_OPAQUE_U8 - buffer[x]*255) * random);

                // avoid computation of random numbers if density is full
                if (m_d->density != 1.0){
                    // compute density only for visible pixels of the mask
                    if (alphaValue != OPACITY_TRANSPARENT_U8){
                        if ( !(m_d->density >= drand48()) ){
                            alphaValue = OPACITY_TRANSPARENT_U8;
                        }
                    }
                }

                m_d->colorSpace->applyAlphaU8Mask(dabPointer, &alphaValue, 1);
                dabPointer += m_d->pixelSize;
            }
        } else {
            m_d->colorSpace->applyInverseNormedFloatMask(dabPointer, buffer, width);
            dabPointer += width * m_d->pixelSize;
        }//endfor x
        dabPointer += offset;
    }//endfor y
    Vc::free(buffer);
}

#endif /* defined HAVE_VC */

template<class MaskGenerator, Vc::Implementation _impl>
void KisBrushMaskScalarApplicator<MaskGenerator, _impl>::processScalar(const QRect &rect)
{
    const MaskProcessingData *m_d = KisBrushMaskApplicatorBase::m_d;
    MaskGenerator *m_maskGenerator = KisBrushMaskScalarApplicator<MaskGenerator, _impl>::m_maskGenerator;

    qreal random = 1.0;
    quint8* dabPointer = m_d->device->data() + rect.y() * rect.width() * m_d->pixelSize;
    quint8 alphaValue = OPACITY_TRANSPARENT_U8;
    // this offset is needed when brush size is smaller then fixed device size
    int offset = (m_d->device->bounds().width() - rect.width()) * m_d->pixelSize;
    int supersample = (m_maskGenerator->shouldSupersample() ? SUPERSAMPLING : 1);
    double invss = 1.0 / supersample;
    int samplearea = supersample * supersample;
    for (int y = rect.y(); y < rect.y() + rect.height(); y++) {
        for (int x = rect.x(); x < rect.x() + rect.width(); x++) {
            int value = 0;
            for (int sy = 0; sy < supersample; sy++) {
                for (int sx = 0; sx < supersample; sx++) {
                    double x_ = (x + sx * invss - m_d->centerX) * m_d->invScaleX;
                    double y_ = (y + sy * invss - m_d->centerY) * m_d->invScaleY;
                    double maskX = m_d->cosa * x_ - m_d->sina * y_;
                    double maskY = m_d->sina * x_ + m_d->cosa * y_;
                    value += m_maskGenerator->valueAt(maskX, maskY);
                }
            }
            if (supersample != 1) value /= samplearea;

            if (m_d->randomness!= 0.0){
                random = (1.0 - m_d->randomness) + m_d->randomness * float(rand()) / RAND_MAX;
            }

            alphaValue = quint8( (OPACITY_OPAQUE_U8 - value) * random);

            // avoid computation of random numbers if density is full
            if (m_d->density != 1.0){
                // compute density only for visible pixels of the mask
                if (alphaValue != OPACITY_TRANSPARENT_U8){
                    if ( !(m_d->density >= drand48()) ){
                        alphaValue = OPACITY_TRANSPARENT_U8;
                    }
                }
            }

            m_d->colorSpace->applyAlphaU8Mask(dabPointer, &alphaValue, 1);
            dabPointer += m_d->pixelSize;
        }//endfor x
        dabPointer += offset;
    }//endfor y
}

#endif /* __KIS_BRUSH_MASK_APPLICATORS_H */
