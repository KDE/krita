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


#include "KoVcMultiArchBuildSupport.h"


class KoCompositeOp;
class KoColorSpace;


template<Vc::Implementation _impl>
class KoOptimizedCompositeOpAlphaDarken32;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpOver32;

template<template<Vc::Implementation I> class CompositeOp>
struct KoOptimizedCompositeOpFactoryPerArch
{
    typedef const KoColorSpace* ParamType;
    typedef KoCompositeOp* ReturnType;

    template<Vc::Implementation _impl>
    static ReturnType create(ParamType param);
};

struct KoReportCurrentArch
{
    typedef void* ParamType;
    typedef void ReturnType;

    template<Vc::Implementation _impl>
    static ReturnType create(ParamType);
};

#endif /* KOOPTIMIZEDCOMPOSITEOPFACTORYPERARCH_H */
