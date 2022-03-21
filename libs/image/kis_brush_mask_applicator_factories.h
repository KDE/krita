/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BRUSH_MASK_APPLICATOR_FACTORIES_H
#define __KIS_BRUSH_MASK_APPLICATOR_FACTORIES_H

#include <compositeops/KoVcMultiArchBuildSupport.h>

class KisBrushMaskApplicatorBase;

template<class MaskGenerator>
struct MaskApplicatorFactory {
    using ParamType = MaskGenerator *;
    using ReturnType = KisBrushMaskApplicatorBase *;

    template<Vc::Implementation _impl>
    static ReturnType create(ParamType maskGenerator);
};

#endif /* __KIS_BRUSH_MASK_APPLICATOR_FACTORIES_H */
