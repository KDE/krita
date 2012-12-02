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

#ifndef KOOPTIMIZEDCOMPOSITEOPFACTORYPERARCH_H
#define KOOPTIMIZEDCOMPOSITEOPFACTORYPERARCH_H

#include "config-vc.h"
#ifndef HAVE_SANE_VC
#error "BUG: There is no reason in including this file when Vc is not present"
#endif

#include "KoVcMultiArchBuildSupport.h"


class KoCompositeOp;
class KoColorSpace;

struct KoOptimizedCompositeOpFactoryPerArchBase
{
    virtual ~KoOptimizedCompositeOpFactoryPerArchBase() {}
    virtual KoCompositeOp* createAlphaDarkenOp32(const KoColorSpace *cs) = 0;
    virtual KoCompositeOp* createOverOp32(const KoColorSpace *cs) = 0;
    virtual void printArchInfo() = 0;
};

template<Vc::Implementation _impl>
struct KoOptimizedCompositeOpFactoryPerArch : public KoOptimizedCompositeOpFactoryPerArchBase
{
    KoCompositeOp* createAlphaDarkenOp32(const KoColorSpace *cs);
    KoCompositeOp* createOverOp32(const KoColorSpace *cs);
    void printArchInfo();
};

#define DECLARE_FOR_ARCH(__arch)                                        \
    template<> KoCompositeOp* KoOptimizedCompositeOpFactoryPerArch<__arch>::createAlphaDarkenOp32(const KoColorSpace *cs); \
    template<> KoCompositeOp* KoOptimizedCompositeOpFactoryPerArch<__arch>::createOverOp32(const KoColorSpace *cs); \
    template<> void KoOptimizedCompositeOpFactoryPerArch<__arch>::printArchInfo();

DECLARE_FOR_ALL_ARCHS_NO_SCALAR(DECLARE_FOR_ARCH);
#define createOptimizedCompositeOpFactory createOptimizedFactoryNoScalar<KoOptimizedCompositeOpFactoryPerArch, KoOptimizedCompositeOpFactoryPerArchBase>


#endif /* KOOPTIMIZEDCOMPOSITEOPFACTORYPERARCH_H */
