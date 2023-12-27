/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brush_mask_applicator_factories.h"

#include "kis_circle_mask_generator.h"
#include "kis_curve_circle_mask_generator.h"
#include "kis_curve_rect_mask_generator.h"
#include "kis_gauss_circle_mask_generator.h"
#include "kis_gauss_rect_mask_generator.h"
#include "kis_rect_mask_generator.h"

#include "kis_brush_mask_scalar_applicator.h"

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisMaskGenerator>::create<xsimd::generic>(
    KisMaskGenerator *maskGenerator)
{
    return new KisBrushMaskScalarApplicator<KisMaskGenerator, xsimd::generic>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisCircleMaskGenerator>::create<xsimd::generic>(
    KisCircleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskScalarApplicator<KisCircleMaskGenerator, xsimd::generic>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisGaussCircleMaskGenerator>::create<xsimd::generic>(
    KisGaussCircleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskScalarApplicator<KisGaussCircleMaskGenerator, xsimd::generic>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisCurveCircleMaskGenerator>::create<xsimd::generic>(
    KisCurveCircleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskScalarApplicator<KisCurveCircleMaskGenerator, xsimd::generic>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisRectangleMaskGenerator>::create<xsimd::generic>(
    KisRectangleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskScalarApplicator<KisRectangleMaskGenerator, xsimd::generic>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisGaussRectangleMaskGenerator>::create<xsimd::generic>(
    KisGaussRectangleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskScalarApplicator<KisGaussRectangleMaskGenerator, xsimd::generic>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisCurveRectangleMaskGenerator>::create<xsimd::generic>(
    KisCurveRectangleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskScalarApplicator<KisCurveRectangleMaskGenerator, xsimd::generic>(maskGenerator);
}
