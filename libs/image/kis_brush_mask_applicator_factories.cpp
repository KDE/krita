/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brush_mask_applicator_factories.h"

#if XSIMD_UNIVERSAL_BUILD_PASS
#include "kis_circle_mask_generator.h"
#include "kis_gauss_circle_mask_generator.h"
#include "kis_curve_circle_mask_generator.h"
#include "kis_gauss_rect_mask_generator.h"
#include "kis_curve_rect_mask_generator.h"
#include "kis_rect_mask_generator.h"

#include "kis_brush_mask_vector_applicator.h"

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisMaskGenerator>::create<xsimd::current_arch>(
    KisMaskGenerator *maskGenerator)
{
    return new KisBrushMaskScalarApplicator<KisMaskGenerator,xsimd::current_arch>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisCircleMaskGenerator>::create<xsimd::current_arch>(
    KisCircleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisCircleMaskGenerator,xsimd::current_arch>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisGaussCircleMaskGenerator>::create<xsimd::current_arch>(
    KisGaussCircleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisGaussCircleMaskGenerator, xsimd::current_arch>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisCurveCircleMaskGenerator>::create<xsimd::current_arch>(
    KisCurveCircleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisCurveCircleMaskGenerator,xsimd::current_arch>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisRectangleMaskGenerator>::create<xsimd::current_arch>(
    KisRectangleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisRectangleMaskGenerator,xsimd::current_arch>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisGaussRectangleMaskGenerator>::create<
    xsimd::current_arch>(KisGaussRectangleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisGaussRectangleMaskGenerator,xsimd::current_arch>(maskGenerator);
}

template<>
template<>
KisBrushMaskApplicatorBase *
MaskApplicatorFactory<KisCurveRectangleMaskGenerator>::create<
    xsimd::current_arch>(KisCurveRectangleMaskGenerator *maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisCurveRectangleMaskGenerator,xsimd::current_arch>(maskGenerator);
}

#endif
