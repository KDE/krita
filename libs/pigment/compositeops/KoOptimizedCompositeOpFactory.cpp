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

#include "KoOptimizedCompositeOpFactory.h"

/**
 * We include these headers even when no vectorization
 * is available on the system to ensure they build correctly
 */
#include "KoOptimizedCompositeOpAlphaDarken32.h"
#include "KoOptimizedCompositeOpOver32.h"


#include "config-vc.h"

#if ! defined HAVE_VC
#include "KoColorSpaceTraits.h"
#include "KoCompositeOpAlphaDarken.h"
#include "KoCompositeOpOver.h"
#endif


KoCompositeOp* KoOptimizedCompositeOpFactory::createAlphaDarkenOp32(const KoColorSpace *cs)
{
#if defined HAVE_VC
    return new KoOptimizedCompositeOpAlphaDarken32(cs);
#else
    return new KoCompositeOpAlphaDarken<KoBgrU8Traits>(cs);
#endif
}

KoCompositeOp* KoOptimizedCompositeOpFactory::createOverOp32(const KoColorSpace *cs)
{
#if defined HAVE_VC
    return new KoOptimizedCompositeOpOver32(cs);
#else
    return new KoCompositeOpOver<KoBgrU8Traits>(cs);
#endif
}
