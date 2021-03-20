/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BRUSH_MASK_APPLICATOR_FACTORIES_H
#define __KIS_BRUSH_MASK_APPLICATOR_FACTORIES_H

#include <compositeops/KoVcMultiArchBuildSupport.h>

class KisBrushMaskApplicatorBase;

template<class MaskGenerator, Vc::Implementation _impl>
struct KisBrushMaskScalarApplicator;

#ifdef HAVE_VC

template<class MaskGenerator, Vc::Implementation _impl>
struct KisBrushMaskVectorApplicator;

#else /* HAVE_VC */

#define KisBrushMaskVectorApplicator KisBrushMaskScalarApplicator

#endif /* HAVE_VC */

template<class MaskGenerator,
         template<class U, Vc::Implementation V> class Applicator>
struct MaskApplicatorFactory
{
    typedef MaskGenerator* ParamType;
    typedef KisBrushMaskApplicatorBase* ReturnType;

    template<Vc::Implementation _impl>
    static ReturnType create(ParamType maskGenerator);
};

#endif /* __KIS_BRUSH_MASK_APPLICATOR_FACTORIES_H */
