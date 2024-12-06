/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOCOMPOSITEOPGENERICFUNCTORBASE_H
#define KOCOMPOSITEOPGENERICFUNCTORBASE_H

#include <KoColorSpaceMaths.h>

/**
 * KoCompositeOpGenericFunctorBase is a set of policies for
 * KoCompositeOpGenericSCFunctor that defines clamping of
 * source and destination **channels** before passing them to
 * the composite function.
 *
 * We cannot clamp the data in the composite function itself,
 * because KoCompositeOpGenericSCFunctor also dose quite heavy
 * blending of the channels when alpha values are non-null
 *
 * IMPORTANT: these policies work with channels_type, not with
 * composite_type! Hence they do **nothing** for integer color
 * spaces!
 */

// no clamping at all
template <typename T>
struct KoCompositeOpGenericFunctorBase
{
    static inline T clampSourceChannelValue(T value) {
        return value;
    }

    static inline T clampDestinationChannelValue(T value) {
        return value;
    }
};

// clamp source channel to SDR, keep destination channel unbounded
template <typename T>
struct KoClampedSourceCompositeOpGenericFunctorBase
{
    static inline T clampSourceChannelValue(T value) {
        using namespace Arithmetic;
        return clampChannelToSDR(value);
    }

    static inline T clampDestinationChannelValue(T value) {
        return value;
    }
};

// make sure source and destination channels are non-negative
template <typename T>
struct KoClampedSourceAndDestinationBottomCompositeOpGenericFunctorBase
{
    static inline T clampSourceChannelValue(T value) {
        using namespace Arithmetic;
        return clampChannelToSDRBottom(value);
    }

    static inline T clampDestinationChannelValue(T value) {
        using namespace Arithmetic;
        return clampChannelToSDRBottom(value);
    }
};

// clamp both source and destination channels to SDR range
template <typename T>
struct KoClampedSourceAndDestinationCompositeOpGenericFunctorBase
{
    static inline T clampSourceChannelValue(T value) {
        using namespace Arithmetic;
        return clampChannelToSDR(value);
    }

    static inline T clampDestinationChannelValue(T value) {
        using namespace Arithmetic;
        return clampChannelToSDR(value);
    }
};

#endif // KOCOMPOSITEOPGENERICFUNCTORBASE_H
