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

#ifdef HAVE_SANE_VC
#include <Vc/global.h>
#include <Vc/common/support.h>
#endif

#include "KoColorSpaceTraits.h"
#include "KoCompositeOpAlphaDarken.h"
#include "KoCompositeOpOver.h"


KoCompositeOp* KoOptimizedCompositeOpFactory::createAlphaDarkenOp32(const KoColorSpace *cs)
{
#if defined HAVE_SANE_VC
    if (Vc::currentImplementationSupported()) {
        return new KoOptimizedCompositeOpAlphaDarken32(cs);
    }
#endif
    return new KoCompositeOpAlphaDarken<KoBgrU8Traits>(cs);
}

KoCompositeOp* KoOptimizedCompositeOpFactory::createOverOp32(const KoColorSpace *cs)
{
#if defined HAVE_SANE_VC
    if (Vc::currentImplementationSupported()) {
        return new KoOptimizedCompositeOpOver32(cs);
    }
#endif
    return new KoCompositeOpOver<KoBgrU8Traits>(cs);

}
