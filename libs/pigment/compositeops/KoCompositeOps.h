/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "compositeops/KoCompositeOpAdd.h"
#include "compositeops/KoCompositeOpAlphaDarken.h"
#include "compositeops/KoCompositeOpBurn.h"
#include "compositeops/KoCompositeOpDivide.h"
#include "compositeops/KoCompositeOpDodge.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpMultiply.h"
#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpOverlay.h"
#include "compositeops/KoCompositeOpScreen.h"
#include "compositeops/KoCompositeOpSubtract.h"
#include "compositeops/KoCompositeOpInversedSubtract.h"
#include "compositeops/KoCompositeOpSoftlight.h"

/**
 * This function add to the colorspace all the composite ops defined by
 * the pigment library.
 */
template<class _Traits_>
void addStandardCompositeOps(KoColorSpace* cs)
{
    cs->addCompositeOp(new KoCompositeOpAdd<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpAlphaDarken<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpBurn<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpDivide<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpDodge<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpErase<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpMultiply<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpOver<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpOverlay<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpScreen<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpSubtract<_Traits_>(cs));
    cs->addCompositeOp(new KoCompositeOpSoftlight<_Traits_>(cs));
}

#endif
