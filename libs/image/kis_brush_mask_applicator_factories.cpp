/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brush_mask_applicator_factories.h"

#include "kis_circle_mask_generator.h"
#include "kis_gauss_circle_mask_generator.h"
#include "kis_curve_circle_mask_generator.h"
#include "kis_gauss_rect_mask_generator.h"
#include "kis_curve_rect_mask_generator.h"
#include "kis_rect_mask_generator.h"

#include "kis_brush_mask_applicator_base.h"
#include "kis_brush_mask_vector_applicator.h"

template<>
template<>
MaskApplicatorFactory<KisMaskGenerator>::ReturnType
MaskApplicatorFactory<KisMaskGenerator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskScalarApplicator<KisMaskGenerator, Vc::CurrentImplementation::current()>(maskGenerator);
}

template<>
template<>
MaskApplicatorFactory<KisCircleMaskGenerator>::ReturnType
MaskApplicatorFactory<KisCircleMaskGenerator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisCircleMaskGenerator, Vc::CurrentImplementation::current()>(maskGenerator);
}

template<>
template<>
MaskApplicatorFactory<KisGaussCircleMaskGenerator>::ReturnType
MaskApplicatorFactory<KisGaussCircleMaskGenerator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisGaussCircleMaskGenerator, Vc::CurrentImplementation::current()>(maskGenerator);
}

template<>
template<>
MaskApplicatorFactory<KisCurveCircleMaskGenerator>::ReturnType
MaskApplicatorFactory<KisCurveCircleMaskGenerator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisCurveCircleMaskGenerator, Vc::CurrentImplementation::current()>(maskGenerator);
}

template<>
template<>
MaskApplicatorFactory<KisRectangleMaskGenerator>::ReturnType
MaskApplicatorFactory<KisRectangleMaskGenerator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisRectangleMaskGenerator, Vc::CurrentImplementation::current()>(maskGenerator);
}

template<>
template<>
MaskApplicatorFactory<KisGaussRectangleMaskGenerator>::ReturnType
MaskApplicatorFactory<KisGaussRectangleMaskGenerator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisGaussRectangleMaskGenerator, Vc::CurrentImplementation::current()>(maskGenerator);
}

template<>
template<>
MaskApplicatorFactory<KisCurveRectangleMaskGenerator>::ReturnType
MaskApplicatorFactory<KisCurveRectangleMaskGenerator>::create<Vc::CurrentImplementation::current()>(ParamType maskGenerator)
{
    return new KisBrushMaskVectorApplicator<KisCurveRectangleMaskGenerator, Vc::CurrentImplementation::current()>(maskGenerator);
}
