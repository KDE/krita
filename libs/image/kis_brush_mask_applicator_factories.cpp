/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brush_mask_applicator_factories.h"
#include "vc_extra_math.h"

#include "kis_circle_mask_generator.h"
#include "kis_circle_mask_generator_p.h"
#include "kis_gauss_circle_mask_generator_p.h"
#include "kis_curve_circle_mask_generator_p.h"
#include "kis_gauss_rect_mask_generator_p.h"
#include "kis_curve_rect_mask_generator_p.h"
#include "kis_rect_mask_generator_p.h"

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

template<>
template<>
MaskApplicatorFactory<KisRectangleMaskGenerator, KisBrushMaskVectorApplicator>::ReturnType
MaskApplicatorFactory<KisRectangleMaskGenerator, KisBrushMaskVectorApplicator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisRectangleMaskGenerator,Vc::CurrentImplementation::current()>(maskGenerator);
}

template<>
template<>
MaskApplicatorFactory<KisGaussRectangleMaskGenerator, KisBrushMaskVectorApplicator>::ReturnType
MaskApplicatorFactory<KisGaussRectangleMaskGenerator, KisBrushMaskVectorApplicator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisGaussRectangleMaskGenerator,Vc::CurrentImplementation::current()>(maskGenerator);
}

template<>
template<>
MaskApplicatorFactory<KisCurveRectangleMaskGenerator, KisBrushMaskVectorApplicator>::ReturnType
MaskApplicatorFactory<KisCurveRectangleMaskGenerator, KisBrushMaskVectorApplicator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisCurveRectangleMaskGenerator,Vc::CurrentImplementation::current()>(maskGenerator);
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

    Vc::float_v vXCoeff(static_cast<float>(d->xcoef));
    Vc::float_v vYCoeff(static_cast<float>(d->ycoef));

    Vc::float_v vTransformedFadeX(static_cast<float>(d->transformedFadeX));
    Vc::float_v vTransformedFadeY(static_cast<float>(d->transformedFadeY));

    Vc::float_v vOne(Vc::One);

    for (size_t i=0; i < static_cast<size_t>(width); i+= Vc::float_v::size()){

        Vc::float_v x_ = currentIndices - vCenterX;

        Vc::float_v xr = x_ * vCosa - vSinaY_;
        Vc::float_v yr = x_ * vSina + vCosaY_;

        Vc::float_v n = pow2(xr * vXCoeff) + pow2(yr * vYCoeff);
        Vc::float_m outsideMask = n > vOne;

        if (!outsideMask.isFull()) {
            if (useSmoothing) {
                xr = Vc::abs(xr) + vOne;
                yr = Vc::abs(yr) + vOne;
            }
            Vc::float_v vNormFade = pow2(xr * vTransformedFadeX) + pow2(yr * vTransformedFadeY);
            Vc::float_m vNormLowMask = vNormFade < vOne;
            vNormFade.setZero(vNormLowMask);

            //255 * n * (normeFade - 1) / (normeFade - n)
            Vc::float_v vFade = n * (vNormFade - vOne) / (vNormFade - n);

            // Mask in the inner circle of the mask
            Vc::float_m mask = vNormFade < vOne;
            vFade.setZero(mask);

            // Mask out the outer circle of the mask
            vFade(outsideMask) = vOne;

            vFade.store(bufferPointer, Vc::Aligned);

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

    Vc::float_v increment(static_cast<float>(Vc::float_v::size()));
    Vc::float_v vCenterX(centerX);
    Vc::float_v vCenter(static_cast<float>(d->center));

    Vc::float_v vCosa(cosa);
    Vc::float_v vSina(sina);
    Vc::float_v vCosaY_(cosay_);
    Vc::float_v vSinaY_(sinay_);

    Vc::float_v vYCoeff(static_cast<float>(d->ycoef));
    Vc::float_v vDistfactor(static_cast<float>(d->distfactor));
    Vc::float_v vAlphafactor(static_cast<float>(d->alphafactor));

    Vc::float_v vZero(Vc::Zero);
    Vc::float_v vValMax(255.f);

    for (size_t i=0; i < static_cast<size_t>(width); i+= Vc::float_v::size()){

        Vc::float_v x_ = currentIndices - vCenterX;

        Vc::float_v xr = x_ * vCosa - vSinaY_;
        Vc::float_v yr = x_ * vSina + vCosaY_;

        Vc::float_v dist = sqrt(pow2(xr) + pow2(yr * vYCoeff));

        // Apply FadeMaker mask and operations
        Vc::float_m excludeMask = d->fadeMaker.needFade(dist);

        if (!excludeMask.isFull()) {
            Vc::float_v valDist = dist * vDistfactor;
            Vc::float_v fullFade = vAlphafactor * (  VcExtraMath::erf(valDist + vCenter) -  VcExtraMath::erf(valDist - vCenter));

            Vc::float_m mask;
            // Mask in the inner circle of the mask
            mask = fullFade < vZero;
            fullFade.setZero(mask);

            // Mask the outer circle
            mask = fullFade > 254.974f;
            fullFade(mask) = vValMax;

            // Mask (value - value), precision errors.
            Vc::float_v vFade = (vValMax - fullFade) / vValMax;

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

    Vc::float_v vYCoeff(static_cast<float>(d->ycoef));
    Vc::float_v vXCoeff(static_cast<float>(d->xcoef));
    Vc::float_v vCurveResolution(static_cast<float>(d->curveResolution));

    Vc::float_v vCurvedData(Vc::Zero);
    Vc::float_v vCurvedData1(Vc::Zero);

    Vc::float_v vOne(Vc::One);
    Vc::float_v vZero(Vc::Zero);

    for (size_t i=0; i < static_cast<size_t>(width); i+= Vc::float_v::size()){

        Vc::float_v x_ = currentIndices - vCenterX;

        Vc::float_v xr = x_ * vCosa - vSinaY_;
        Vc::float_v yr = x_ * vSina + vCosaY_;

        Vc::float_v dist = pow2(xr * vXCoeff) + pow2(yr * vYCoeff);

        // Apply FadeMaker mask and operations
        Vc::float_m excludeMask = d->fadeMaker.needFade(dist);

        if (!excludeMask.isFull()) {
            Vc::float_v valDist = dist * vCurveResolution;
            // truncate
            Vc::float_v::IndexType vAlphaValue(valDist);
            Vc::float_v vFloatAlphaValue = vAlphaValue;

            Vc::float_v alphaValueF = valDist - vFloatAlphaValue;

            Vc::float_m alphaMask = vAlphaValue < vZero;
            vAlphaValue.setZero(alphaMask);

            vCurvedData.gather(curveDataPointer,vAlphaValue);
            vCurvedData1.gather(curveDataPointer,vAlphaValue + 1);
//            Vc::float_v vCurvedData1(curveDataPointer,vAlphaValue + 1);

            // vAlpha
            Vc::float_v fullFade = (
                (vOne - alphaValueF) * vCurvedData +
                alphaValueF * vCurvedData1);

            Vc::float_m mask;
            // Mask in the inner circle of the mask
            mask = fullFade < vZero;
            fullFade.setZero(mask);

            // Mask outer circle of mask
            mask = fullFade >= vOne;
            Vc::float_v vFade = (vOne - fullFade);
            vFade.setZero(mask);

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

struct KisGaussRectangleMaskGenerator::FastRowProcessor
{
    FastRowProcessor(KisGaussRectangleMaskGenerator *maskGenerator)
        : d(maskGenerator->d.data()) {}

    template<Vc::Implementation _impl>
    void process(float* buffer, int width, float y, float cosa, float sina,
                 float centerX, float centerY);

    KisGaussRectangleMaskGenerator::Private *d;
};

struct KisRectangleMaskGenerator::FastRowProcessor
{
    FastRowProcessor(KisRectangleMaskGenerator *maskGenerator)
        : d(maskGenerator->d.data()) {}

    template<Vc::Implementation _impl>
    void process(float* buffer, int width, float y, float cosa, float sina,
                 float centerX, float centerY);

    KisRectangleMaskGenerator::Private *d;
};

template<> void KisRectangleMaskGenerator::
FastRowProcessor::process<Vc::CurrentImplementation::current()>(float* buffer, int width, float y, float cosa, float sina,
                                   float centerX, float centerY)
{
    const bool useSmoothing = d->copyOfAntialiasEdges;

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

    Vc::float_v vXCoeff(static_cast<float>(d->xcoeff));
    Vc::float_v vYCoeff(static_cast<float>(d->ycoeff));

    Vc::float_v vTransformedFadeX(static_cast<float>(d->transformedFadeX));
    Vc::float_v vTransformedFadeY(static_cast<float>(d->transformedFadeY));

    Vc::float_v vOne(Vc::One);
    Vc::float_v vZero(Vc::Zero);
    Vc::float_v vTolerance(10000.f);

    for (size_t i=0; i < static_cast<size_t>(width); i+= Vc::float_v::size()){

        Vc::float_v x_ = currentIndices - vCenterX;

        Vc::float_v xr = Vc::abs(x_ * vCosa - vSinaY_);
        Vc::float_v yr = Vc::abs(x_ * vSina + vCosaY_);

        Vc::float_v nxr = xr * vXCoeff;
        Vc::float_v nyr = yr * vYCoeff;

        Vc::float_m outsideMask = (nxr > vOne) || (nyr > vOne);

        if (!outsideMask.isFull()) {
            if (useSmoothing) {
                xr = Vc::abs(xr) + vOne;
                yr = Vc::abs(yr) + vOne;
            }

            Vc::float_v fxr = xr * vTransformedFadeX;
            Vc::float_v fyr = yr * vTransformedFadeY;

            Vc::float_v fxrNorm = nxr * (fxr - vOne) / (fxr - nxr);
            Vc::float_v fyrNorm = nyr * (fyr - vOne) / (fyr - nyr);

            Vc::float_v vFade(vZero);

            Vc::float_m vFadeMask = fxrNorm < fyrNorm;
            Vc::float_v vMaxVal = vFade;
            vMaxVal(fxr > vOne) = fxrNorm;
            vMaxVal(vFadeMask && fyr > vOne) = fyrNorm;
            vFade = vMaxVal;

            // Mask out the outer circle of the mask
            vFade(outsideMask) = vOne;
            vFade.store(bufferPointer, Vc::Aligned);
        } else {
            // Mask out everything outside the circle
            vOne.store(bufferPointer, Vc::Aligned);
        }

        currentIndices = currentIndices + increment;

        bufferPointer += Vc::float_v::size();
    }
}


template<> void KisGaussRectangleMaskGenerator::
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

    Vc::float_v vCosa(cosa);
    Vc::float_v vSina(sina);
    Vc::float_v vCosaY_(cosay_);
    Vc::float_v vSinaY_(sinay_);

    Vc::float_v vhalfWidth(static_cast<float>(d->halfWidth));
    Vc::float_v vhalfHeight(static_cast<float>(d->halfHeight));
    Vc::float_v vXFade(static_cast<float>(d->xfade));
    Vc::float_v vYFade(static_cast<float>(d->yfade));

    Vc::float_v vAlphafactor(static_cast<float>(d->alphafactor));

    Vc::float_v vOne(Vc::One);
    Vc::float_v vZero(Vc::Zero);
    Vc::float_v vValMax(255.f);

    for (size_t i = 0; i < static_cast<size_t>(width); i += Vc::float_v::size()) {
        Vc::float_v x_ = currentIndices - vCenterX;

        Vc::float_v xr = x_ * vCosa - vSinaY_;
        Vc::float_v yr = Vc::abs(x_ * vSina + vCosaY_);

        Vc::float_v vValue;

        // check if we need to apply fader on values
        Vc::float_m excludeMask = d->fadeMaker.needFade(xr,yr);
        vValue(excludeMask) = vOne;

        if (!excludeMask.isFull()) {
            Vc::float_v fullFade = vValMax - (vAlphafactor * (VcExtraMath::erf((vhalfWidth + xr) * vXFade) + VcExtraMath::erf((vhalfWidth - xr) * vXFade))
                                        * (VcExtraMath::erf((vhalfHeight + yr) * vYFade) + VcExtraMath::erf((vhalfHeight - yr) * vYFade)));

            // apply antialias fader
            d->fadeMaker.apply2DFader(fullFade,excludeMask,xr,yr);

            Vc::float_m mask;

            // Mask in the inner circle of the mask
            mask = fullFade < vZero;
            fullFade.setZero(mask);

            // Mask the outer circle
            mask = fullFade > 254.974f;
            fullFade(mask) = vValMax;

            // Mask (value - value), precision errors.
            Vc::float_v vFade = fullFade / vValMax;

            // return original vValue values before vFade transform
            vFade(excludeMask) = vValue;
            vFade.store(bufferPointer, Vc::Aligned);

        } else {
          vValue.store(bufferPointer, Vc::Aligned);
      }
      currentIndices = currentIndices + increment;

      bufferPointer += Vc::float_v::size();
    }
}

struct KisCurveRectangleMaskGenerator::FastRowProcessor
{
    FastRowProcessor(KisCurveRectangleMaskGenerator *maskGenerator)
        : d(maskGenerator->d.data()) {}

    template<Vc::Implementation _impl>
    void process(float* buffer, int width, float y, float cosa, float sina,
                 float centerX, float centerY);

    KisCurveRectangleMaskGenerator::Private *d;
};

template<> void KisCurveRectangleMaskGenerator::
FastRowProcessor::process<Vc::CurrentImplementation::current()>(float* buffer, int width, float y, float cosa, float sina,
                                   float centerX, float centerY)
{
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

    Vc::float_v vYCoeff(static_cast<float>(d->ycoeff));
    Vc::float_v vXCoeff(static_cast<float>(d->xcoeff));
    Vc::float_v vCurveResolution(static_cast<float>(d->curveResolution));

    Vc::float_v vOne(Vc::One);
    Vc::float_v vZero(Vc::Zero);
    Vc::float_v vValMax(255.f);

    for (size_t i = 0; i < static_cast<size_t>(width); i += Vc::float_v::size()) {
        Vc::float_v x_ = currentIndices - vCenterX;

        Vc::float_v xr = x_ * vCosa - vSinaY_;
        Vc::float_v yr = Vc::abs(x_ * vSina + vCosaY_);

        Vc::float_v vValue;

        // check if we need to apply fader on values
        Vc::float_m excludeMask = d->fadeMaker.needFade(xr,yr);
        vValue(excludeMask) = vOne;

        if (!excludeMask.isFull()) {
            // We need to mask the extra area given for aliniation
            // the next operation should never give values above 1
            Vc::float_v preSIndex = Vc::abs(xr) * vXCoeff;
            Vc::float_v preTIndex = Vc::abs(yr) * vYCoeff;

            preSIndex(preSIndex > vOne) = vOne;
            preTIndex(preTIndex > vOne) = vOne;

            Vc::float_v::IndexType sIndex( round(preSIndex * vCurveResolution));
            Vc::float_v::IndexType tIndex( round(preTIndex * vCurveResolution));

            Vc::float_v::IndexType sIndexInverted = vCurveResolution - sIndex;
            Vc::float_v::IndexType tIndexInverted = vCurveResolution - tIndex;

            Vc::float_v vCurvedDataSIndex(curveDataPointer, sIndex);
            Vc::float_v vCurvedDataTIndex(curveDataPointer, tIndex);
            Vc::float_v vCurvedDataSIndexInv(curveDataPointer, sIndexInverted);
            Vc::float_v vCurvedDataTIndexInv(curveDataPointer, tIndexInverted);

            Vc::float_v fullFade = vValMax * (vOne - (vCurvedDataSIndex * (vOne - vCurvedDataSIndexInv) *
                                    vCurvedDataTIndex * (vOne - vCurvedDataTIndexInv)));

            // apply antialias fader
            d->fadeMaker.apply2DFader(fullFade,excludeMask,xr,yr);

            Vc::float_m mask;

            // Mask in the inner circle of the mask
            mask = fullFade < vZero;
            fullFade.setZero(mask);

            // Mask the outer circle
            mask = fullFade > 254.974f;
            fullFade(mask) = vValMax;

            // Mask (value - value), precision errors.
            Vc::float_v vFade = fullFade / vValMax;

            // return original vValue values before vFade transform
            vFade(excludeMask) = vValue;
            vFade.store(bufferPointer, Vc::Aligned);

        } else {
          vValue.store(bufferPointer, Vc::Aligned);
      }
      currentIndices = currentIndices + increment;

      bufferPointer += Vc::float_v::size();
    }
}

#endif /* defined HAVE_VC */
