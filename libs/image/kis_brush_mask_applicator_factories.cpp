/*
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

#include "kis_brush_mask_applicator_factories.h"

#include "kis_circle_mask_generator.h"
#include "kis_circle_mask_generator_p.h"
#include "kis_gauss_circle_mask_generator_p.h"
#include "kis_curve_circle_mask_generator_p.h"

#include "kis_brush_mask_applicators.h"
#include "kis_brush_mask_applicator_base.h"

#define a(_s) #_s
#define b(_s) a(_s)

template<>
template<>
MaskApplicatorFactory<KisMaskGenerator, KisBrushMaskScalarApplicator>::ReturnType
MaskApplicatorFactory<KisMaskGenerator, KisBrushMaskScalarApplicator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskScalarApplicator<KisMaskGenerator,Vc::CurrentImplementation::current()>(maskGenerator);
}

template<>
template<>
MaskApplicatorFactory<KisCircleMaskGenerator, KisBrushMaskVectorApplicator>::ReturnType
MaskApplicatorFactory<KisCircleMaskGenerator, KisBrushMaskVectorApplicator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisCircleMaskGenerator,Vc::CurrentImplementation::current()>(maskGenerator);
}

template<>
template<>
MaskApplicatorFactory<KisGaussCircleMaskGenerator, KisBrushMaskVectorApplicator>::ReturnType
MaskApplicatorFactory<KisGaussCircleMaskGenerator, KisBrushMaskVectorApplicator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisGaussCircleMaskGenerator,Vc::CurrentImplementation::current()>(maskGenerator);
}


template<>
template<>
MaskApplicatorFactory<KisCurveCircleMaskGenerator, KisBrushMaskVectorApplicator>::ReturnType
MaskApplicatorFactory<KisCurveCircleMaskGenerator, KisBrushMaskVectorApplicator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisCurveCircleMaskGenerator,Vc::CurrentImplementation::current()>(maskGenerator);
}


#if defined HAVE_VC

struct KisCircleMaskGenerator::FastRowProcessor
{
    FastRowProcessor(KisCircleMaskGenerator *maskGenerator)
        : d(maskGenerator->d.data()) {}

    template<Vc::Implementation _impl>
    void process(float* buffer, int width, float y, float cosa, float sina,
                 float centerX, float centerY);

    KisCircleMaskGenerator::Private *d;
};

template<> void KisCircleMaskGenerator::
FastRowProcessor::process<Vc::CurrentImplementation::current()>(float* buffer, int width, float y, float cosa, float sina,
                                   float centerX, float centerY)
{
    const bool useSmoothing = d->copyOfAntialiasEdges;
    const bool noFading = d->noFading;

    float y_ = y - centerY;
    float sinay_ = sina * y_;
    float cosay_ = cosa * y_;

    float* bufferPointer = buffer;

    Vc::float_v currentIndices = Vc::float_v::IndexesFromZero();

    Vc::float_v increment((float)Vc::float_v::size());
    Vc::float_v vCenterX(centerX);

    Vc::float_v vCosa(cosa);
    Vc::float_v vSina(sina);
    Vc::float_v vCosaY_(cosay_);
    Vc::float_v vSinaY_(sinay_);

    Vc::float_v vXCoeff(d->xcoef);
    Vc::float_v vYCoeff(d->ycoef);

    Vc::float_v vTransformedFadeX(d->transformedFadeX);
    Vc::float_v vTransformedFadeY(d->transformedFadeY);

    Vc::float_v vOne(Vc::One);

    for (int i=0; i < width; i+= Vc::float_v::size()){

        Vc::float_v x_ = currentIndices - vCenterX;

        Vc::float_v xr = x_ * vCosa - vSinaY_;
        Vc::float_v yr = x_ * vSina + vCosaY_;

        Vc::float_v n = pow2(xr * vXCoeff) + pow2(yr * vYCoeff);
        Vc::float_m outsideMask = n > vOne;

        if (!outsideMask.isFull()) {

            if (noFading) {
                Vc::float_v vFade(Vc::Zero);
                vFade(outsideMask) = vOne;
                vFade.store(bufferPointer, Vc::Aligned);
            } else {
                if (useSmoothing) {
                    xr = Vc::abs(xr) + vOne;
                    yr = Vc::abs(yr) + vOne;
                }

                Vc::float_v vNormFade = pow2(xr * vTransformedFadeX) + pow2(yr * vTransformedFadeY);

                //255 * n * (normeFade - 1) / (normeFade - n)
                Vc::float_v vFade = n * (vNormFade - vOne) / (vNormFade - n);

                // Mask in the inner circe of the mask
                Vc::float_m mask = vNormFade < vOne;
                vFade.setZero(mask);

                // Mask out the outer circe of the mask
                vFade(outsideMask) = vOne;

                vFade.store(bufferPointer, Vc::Aligned);
            }
        } else {
            // Mask out everything outside the circle
            vOne.store(bufferPointer, Vc::Aligned);
        }

        currentIndices = currentIndices + increment;

        bufferPointer += Vc::float_v::size();
    }
}


struct KisGaussCircleMaskGenerator::FastRowProcessor
{
    FastRowProcessor(KisGaussCircleMaskGenerator *maskGenerator)
        : d(maskGenerator->d.data()) {}

    template<Vc::Implementation _impl>
    void process(float* buffer, int width, float y, float cosa, float sina,
                 float centerX, float centerY);

    KisGaussCircleMaskGenerator::Private *d;
};

template<> void KisGaussCircleMaskGenerator::
FastRowProcessor::process<Vc::CurrentImplementation::current()>(float* buffer, int width, float y, float cosa, float sina,
                                   float centerX, float centerY)
{   
    float y_ = y - centerY;
    float sinay_ = sina * y_;
    float cosay_ = cosa * y_;

    float* bufferPointer = buffer;

    Vc::float_v currentIndices = Vc::float_v::IndexesFromZero();

    Vc::float_v increment((float)Vc::float_v::size());
    Vc::float_v vCenterX(centerX);
    Vc::float_v vCenter(d->center);

    Vc::float_v vCosa(cosa);
    Vc::float_v vSina(sina);
    Vc::float_v vCosaY_(cosay_);
    Vc::float_v vSinaY_(sinay_);

    Vc::float_v vYCoeff(d->ycoef);
    Vc::float_v vDistfactor(d->distfactor);
    Vc::float_v vAlphafactor(d->alphafactor);

    Vc::float_v vFadeRadius(d->fadeMaker.getRadius());
    Vc::float_v vFadeStartValue(d->fadeMaker.getFadeStartValue());
    Vc::float_v vFadeAFadeStart(d->fadeMaker.getAntialiasingFadeStart());
    Vc::float_v vFadeAFadeCoeff(d->fadeMaker.getAntialiasingFadeCoeff());

    Vc::float_v vOne(Vc::One);
    Vc::float_v vZero(Vc::Zero);
    Vc::float_v vValMax(255.f);

    for (int i=0; i < width; i+= Vc::float_v::size()){

        Vc::float_v x_ = currentIndices - vCenterX;

        Vc::float_v xr = x_ * vCosa - vSinaY_;
        Vc::float_v yr = x_ * vSina + vCosaY_;

        Vc::float_v dist = sqrt(pow2(xr) + pow2(yr * vYCoeff));

        // BEGIN FadeMaker needFade vectorized
        // follow fademaker rules for outsideMask
        Vc::float_m outsideMask = dist > vFadeRadius;
        dist(outsideMask) = vOne;

        Vc::float_m fadeStartMask = dist > vFadeAFadeStart;
        dist((outsideMask ^ fadeStartMask) & fadeStartMask) = (vFadeStartValue + (dist - vFadeAFadeStart) * vFadeAFadeCoeff) / vValMax;

        Vc::float_m excludeMask = outsideMask | fadeStartMask;

        if (!excludeMask.isFull()) {
            Vc::float_v valDist = dist * vDistfactor;
            Vc::float_v fullFade = vAlphafactor * ( d->vErf(valDist + vCenter) - d->vErf(valDist - vCenter));

            Vc::float_m mask;
            // Mask in the inner circe of the mask
            mask = fullFade < vZero;
            fullFade.setZero(mask);

            // Mask (value - value), presicion errors.
            mask = fullFade >= vValMax;
            Vc::float_v vFade = (vValMax - fullFade) / vValMax;
            vFade(mask) = vZero;

            // filter nan and inf. Vc uses float, imprecission errors are frequent
            mask = Vc::isfinite(vFade);
            vFade(!mask) = vOne;

            // Mask the inner circe of the mask
            mask = vFade < vZero;
            vFade(mask) = vZero;

            // return original dist values before vFade transform
            vFade(excludeMask) = dist;
            vFade.store(bufferPointer, Vc::Aligned);

        } else {
          dist.store(bufferPointer, Vc::Aligned);
      }
      currentIndices = currentIndices + increment;

      bufferPointer += Vc::float_v::size();
    }
}

struct KisCurveCircleMaskGenerator::FastRowProcessor
{
    FastRowProcessor(KisCurveCircleMaskGenerator *maskGenerator)
        : d(maskGenerator->d.data()) {}

    template<Vc::Implementation _impl>
    void process(float* buffer, int width, float y, float cosa, float sina,
                 float centerX, float centerY);

    KisCurveCircleMaskGenerator::Private *d;
};


template<> void KisCurveCircleMaskGenerator::
FastRowProcessor::process<Vc::CurrentImplementation::current()>(float* buffer, int width, float y, float cosa, float sina,
                                   float centerX, float centerY)
{
    const bool antialiasOn = d->fadeMaker.getAliasingEnabled();

    float y_ = y - centerY;
    float sinay_ = sina * y_;
    float cosay_ = cosa * y_;

    float* bufferPointer = buffer;

    qreal* curveDataPointer = d->curveData.data();

    Vc::float_v currentIndices = Vc::float_v::IndexesFromZero();

    Vc::float_v increment((float)Vc::float_v::size());
    Vc::float_v vCenterX(centerX);

    Vc::float_v vCosa(cosa);
    Vc::float_v vSina(sina);
    Vc::float_v vCosaY_(cosay_);
    Vc::float_v vSinaY_(sinay_);

    Vc::float_v vYCoeff(d->ycoef);
    Vc::float_v vXCoeff(d->xcoef);
    Vc::float_v vCurveResolution(d->curveResolution);

    Vc::float_v vCurvedData(Vc::Zero);

    Vc::float_v vFadeRadius(d->fadeMaker.getRadius());
    Vc::float_v vFadeStartValue(d->fadeMaker.getFadeStartValue());
    Vc::float_v vFadeAFadeStart(d->fadeMaker.getAntialiasingFadeStart());
    Vc::float_v vFadeAFadeCoeff(d->fadeMaker.getAntialiasingFadeCoeff());

    Vc::float_v vOne(Vc::One);
    Vc::float_v vZero(Vc::Zero);
    Vc::float_v vValMax(255.f);

    for (int i=0; i < width; i+= Vc::float_v::size()){

        Vc::float_v x_ = currentIndices - vCenterX;

        Vc::float_v xr = x_ * vCosa - vSinaY_;
        Vc::float_v yr = x_ * vSina + vCosaY_;

        Vc::float_v dist = pow2(xr * vXCoeff) + pow2(yr * vYCoeff);

        // BEGIN FadeMaker needFade vectorized
        // follow fademaker rules for outsideMask
        Vc::float_m outsideMask = dist > vFadeRadius;
        dist(outsideMask) = vOne;

        Vc::float_m fadeStartMask(false);
        // if antialias is off, do not process
        if(antialiasOn){
            fadeStartMask = dist > vFadeAFadeStart;
            dist((outsideMask ^ fadeStartMask) & fadeStartMask) = (vFadeStartValue + (dist - vFadeAFadeStart) * vFadeAFadeCoeff) / vValMax;
        }

        Vc::float_m excludeMask = outsideMask | fadeStartMask;

        if (!excludeMask.isFull()) {
            Vc::float_v valDist = dist * vCurveResolution;
            // truncate
            Vc::SimdArray<quint16,Vc::float_v::size()> vAlphaValue(valDist);
            Vc::float_v vFloatAlphaValue = vAlphaValue;

            Vc::float_v alphaValueF = valDist - vFloatAlphaValue;

            vCurvedData.gather(curveDataPointer,vAlphaValue);
            Vc::float_v vCurvedData1(curveDataPointer,vAlphaValue + 1);

            // vAlpha
            Vc::float_v fullFade = (
                (1.0f - alphaValueF) * vCurvedData +
                alphaValueF * vCurvedData1);

            Vc::float_m mask;
            // Mask in the inner circe of the mask
            mask = fullFade < vZero;
            fullFade.setZero(mask);

            // Mask (value - value), presicion errors.
            mask = fullFade >= 1.0f;
            Vc::float_v vFade = (1.0f - fullFade);
            vFade(mask) = vZero;


            // filter nan and inf. Vc uses float, imprecission errors are frequent
            mask = Vc::isfinite(vFade);
            vFade(!mask) = vOne;

            // Mask the inner circe of the mask
            mask = vFade < vZero;
            vFade(mask) = vZero;

            // return original dist values before vFade transform
            vFade(excludeMask) = dist;
            vFade.store(bufferPointer, Vc::Aligned);

        } else {
          dist.store(bufferPointer, Vc::Aligned);
      }
      currentIndices = currentIndices + increment;

      bufferPointer += Vc::float_v::size();
    }
}

#endif /* defined HAVE_VC */
