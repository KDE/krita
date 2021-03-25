/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KOOPTIMIZEDCOMPOSITEOPFACTORY_H
#define KOOPTIMIZEDCOMPOSITEOPFACTORY_H

#include "kritapigment_export.h"

class KoCompositeOp;
class KoColorSpace;

/**
 * The creation of the optimized composite ops is moved into a separate
 * objects module for two reasons:
 *
 * 1) They are not templated, that is they do not need inlining into
 *    the user's code.
 * 2) This removes compilation dependencies.
 * 3) (most important!) When the object module is shared with a colorspace
 *    class, which is quite huge itself, GCC layouts the code somehow badly
 *    that causes 60% performance degradation.
 */

class KRITAPIGMENT_EXPORT KoOptimizedCompositeOpFactory
{
public:
    static KoCompositeOp* createAlphaDarkenOpHard32(const KoColorSpace *cs);
    static KoCompositeOp* createAlphaDarkenOpCreamy32(const KoColorSpace *cs);
    static KoCompositeOp* createOverOp32(const KoColorSpace *cs);
    static KoCompositeOp* createAlphaDarkenOpHard128(const KoColorSpace *cs);
    static KoCompositeOp* createAlphaDarkenOpCreamy128(const KoColorSpace *cs);
    static KoCompositeOp* createOverOp128(const KoColorSpace *cs);
    static KoCompositeOp* createOverOpU64(const KoColorSpace *cs);
    static KoCompositeOp* createCopyOp128(const KoColorSpace *cs);
    static KoCompositeOp* createCopyOpU64(const KoColorSpace *cs);
    static KoCompositeOp* createCopyOp32(const KoColorSpace *cs);
    static KoCompositeOp* createAlphaDarkenOpHardU64(const KoColorSpace *cs);
    static KoCompositeOp* createAlphaDarkenOpCreamyU64(const KoColorSpace *cs);
};

#endif /* KOOPTIMIZEDCOMPOSITEOPFACTORY_H */
