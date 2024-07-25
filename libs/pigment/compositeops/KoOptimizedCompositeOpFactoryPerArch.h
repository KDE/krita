/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KOOPTIMIZEDCOMPOSITEOPFACTORYPERARCH_H
#define KOOPTIMIZEDCOMPOSITEOPFACTORYPERARCH_H

#include <KoMultiArchBuildSupport.h>

class KoCompositeOp;
class KoColorSpace;

template<typename _impl>
class KoOptimizedCompositeOpAlphaDarkenCreamy32;

template<typename _impl>
class KoOptimizedCompositeOpAlphaDarkenHard32;

template<typename _impl>
class KoOptimizedCompositeOpOver32;

template<typename _impl>
class KoOptimizedCompositeOpAlphaDarkenHard128;

template<typename _impl>
class KoOptimizedCompositeOpAlphaDarkenCreamy128;

template<typename _impl>
class KoOptimizedCompositeOpAlphaDarkenHardU64;

template<typename _impl>
class KoOptimizedCompositeOpAlphaDarkenCreamyU64;

template<typename _impl>
class KoOptimizedCompositeOpOver128;

template<typename _impl>
class KoOptimizedCompositeOpOverU64;

template<typename _impl>
class KoOptimizedCompositeOpCopy128;

template<typename _impl>
class KoOptimizedCompositeOpCopyU64;

template<typename _impl>
class KoOptimizedCompositeOpCopy32;

template<template<typename I> class CompositeOp>
struct KoOptimizedCompositeOpFactoryPerArch {
    template<typename _impl>
    static KoCompositeOp *create(const KoColorSpace *);
};

#endif /* KOOPTIMIZEDCOMPOSITEOPFACTORYPERARCH_H */
