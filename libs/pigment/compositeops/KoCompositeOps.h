/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KOCOMPOSITEOPS_H_
#define _KOCOMPOSITEOPS_H_

#include "KoColorSpace.h"

// #include "compositeops/KoCompositeOpAdd.h"
#include "compositeops/KoCompositeOpAlphaDarken.h"
// #include "compositeops/KoCompositeOpBurn.h"
#include "compositeops/KoCompositeOpDivide.h"
// #include "compositeops/KoCompositeOpDodge.h"
#include "compositeops/KoCompositeOpErase.h"
// #include "compositeops/KoCompositeOpMultiply.h"
// #include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpOverlay.h"
// #include "compositeops/KoCompositeOpScreen.h"
// #include "compositeops/KoCompositeOpSubtract.h"
#include "compositeops/KoCompositeOpInversedSubtract.h"
#include "compositeops/KoCompositeOpSoftlight.h"
#include "compositeops/KoCompositeOpHardlight.h"
#include "compositeops/KoCompositeOpCopy2.h"
#include "compositeops/KoCompositeOpGeneric.h"

/**
 * This function add to the colorspace all the composite ops defined by
 * the pigment library.
 */
template<class _Traits_>
void addStandardCompositeOps(KoColorSpace* cs)
{
    //cs->addCompositeOp(new KoCompositeOpAdd<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpAlphaDarken<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpBurn<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpCopy2<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpDivide<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpDodge<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpErase<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpMultiply<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpOver<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpOverlay<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpScreen<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpSubtract<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpSoftlight<_Traits_>(cs));
    //cs->addCompositeOp(new KoCompositeOpHardlight<_Traits_>(cs));
    
    typedef typename _Traits_::channels_type type;
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfOver <type> >(cs, COMPOSITE_OVER, i18n("Normal"), KoCompositeOp::categoryMix()));
    
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfColorBurn  <type> >(cs, COMPOSITE_BURN        , i18n("Color Burn"), KoCompositeOp::categoryLight()));
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfColorDodge <type> >(cs, COMPOSITE_DODGE       , i18n("Color Dodge"), KoCompositeOp::categoryLight()));
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfLinearBurn <type> >(cs, COMPOSITE_LINEAR_BURN , i18n("Linear Burn"), KoCompositeOp::categoryLight()));
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfAddition   <type> >(cs, COMPOSITE_LINEAR_DODGE, i18n("Linear Dodge"), KoCompositeOp::categoryLight()));
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfDarkenOnly <type> >(cs, COMPOSITE_DARKEN      , i18n("Darken"), KoCompositeOp::categoryLight()));
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfLightenOnly<type> >(cs, COMPOSITE_LIGHTEN     , i18n("Lighten"), KoCompositeOp::categoryLight()));
    
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfAddition  <type> >(cs, COMPOSITE_ADD      , i18n("Addition"), KoCompositeOp::categoryArithmetic()));
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfSubtract  <type> >(cs, COMPOSITE_SUBTRACT , i18n("Subtract"), KoCompositeOp::categoryArithmetic()));
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfDifference<type> >(cs, COMPOSITE_DIFF     , i18n("Difference"), KoCompositeOp::categoryArithmetic()));
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfMultiply  <type> >(cs, COMPOSITE_MULT     , i18n("Multiply"), KoCompositeOp::categoryArithmetic()));
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfDivide    <type> >(cs, COMPOSITE_DIVIDE   , i18n("Divide"), KoCompositeOp::categoryArithmetic()));
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfExclusion <type> >(cs, COMPOSITE_EXCLUSION, i18n("Exclusion"), KoCompositeOp::categoryArithmetic()));
    cs->addCompositeOp(new KoCompositeOpGeneric< _Traits_, &cfScreen    <type> >(cs, COMPOSITE_SCREEN   , i18n("Screen"), KoCompositeOp::categoryArithmetic()));
}

#endif
