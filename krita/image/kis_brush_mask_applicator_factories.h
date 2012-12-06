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

#ifndef __KIS_BRUSH_MASK_APPLICATOR_FACTORIES_H
#define __KIS_BRUSH_MASK_APPLICATOR_FACTORIES_H

#include "KoVcMultiArchBuildSupport.h"

#include "kis_brush_mask_applicator_base.h"


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
