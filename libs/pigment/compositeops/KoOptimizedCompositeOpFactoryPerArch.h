/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KOOPTIMIZEDCOMPOSITEOPFACTORYPERARCH_H
#define KOOPTIMIZEDCOMPOSITEOPFACTORYPERARCH_H


#include <compositeops/KoVcMultiArchBuildSupport.h>


class KoCompositeOp;
class KoColorSpace;


template<Vc::Implementation _impl>
class KoOptimizedCompositeOpAlphaDarkenCreamy32;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpAlphaDarkenHard32;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpOver32;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpAlphaDarkenHard128;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpAlphaDarkenCreamy128;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpAlphaDarkenHardU64;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpAlphaDarkenCreamyU64;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpOver128;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpOverU64;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpCopy128;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpCopyU64;

template<Vc::Implementation _impl>
class KoOptimizedCompositeOpCopy32;

template<template<Vc::Implementation I> class CompositeOp>
struct KoOptimizedCompositeOpFactoryPerArch
{
    typedef const KoColorSpace* ParamType;
    typedef KoCompositeOp* ReturnType;

    template<Vc::Implementation _impl>
    static ReturnType create(ParamType param);
};


#endif /* KOOPTIMIZEDCOMPOSITEOPFACTORYPERARCH_H */
