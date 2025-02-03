/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOCOMPOSITEOPCLAMPPOLICY_H
#define KOCOMPOSITEOPCLAMPPOLICY_H

#include <KoColorSpaceMaths.h>

namespace KoCompositeOpClampPolicy
{
template <typename T>
struct ClampAsInteger {
    using channels_type = T;
    using compositetype = typename KoColorSpaceMathsTraits<T>::compositetype;

    static inline channels_type clampResultAllowNegative(compositetype value) {
        using namespace Arithmetic;
        return clampToSDR<channels_type>(value);
    }

    static inline channels_type clampResult(compositetype value) {
        using namespace Arithmetic;
        return clampToSDR<channels_type>(value);
    }

    static inline channels_type clampInvertedResult(compositetype value) {
        using namespace Arithmetic;
        return clampToSDR<channels_type>(value);
    }

    static inline compositetype fixInfiniteAfterDivision(compositetype value) {
        return value;
    }

    static inline T clippedMaxValue() {
        using namespace Arithmetic;
        return unitValue<T>();
    }
};

template <typename T>
struct ClampAsFloatSDR {
    using channels_type = T;
    using compositetype = typename KoColorSpaceMathsTraits<T>::compositetype;

    static channels_type clampResultAllowNegative(compositetype value) {
        using namespace Arithmetic;
        return clampToSDR<channels_type>(value);
    }

    static inline channels_type clampResult(compositetype value) {
        using namespace Arithmetic;
        return clampToSDR<channels_type>(value);
    }

    static channels_type clampInvertedResult(compositetype value) {
        using namespace Arithmetic;
        return clampToSDR<channels_type>(value);
    }

    static compositetype fixInfiniteAfterDivision(compositetype value) {
        // Constantly dividing by small numbers can quickly make the result
        // become infinity or NaN, so we check that and correct (kind of clamping)
        return std::isfinite(value) ? value : compositetype(KoColorSpaceMathsTraits<T>::unitValue);
    }

    static inline T clippedMaxValue() {
        using namespace Arithmetic;
        return unitValue<T>();
    }
};

template <typename T>
struct ClampAsFloatHDR {
    using channels_type = T;
    using compositetype = typename KoColorSpaceMathsTraits<T>::compositetype;

    static channels_type clampResultAllowNegative(compositetype value) {
        return value;
    }

    static inline channels_type clampResult(compositetype value) {
        using namespace Arithmetic;
        return clampToSDRBottom<channels_type>(value);
    }

    static channels_type clampInvertedResult(compositetype value) {
        using namespace Arithmetic;
        return clampToSDRTop<channels_type>(value);
    }

    static compositetype fixInfiniteAfterDivision(compositetype value) {
        // Constantly dividing by small numbers can quickly make the result
        // become infinity or NaN, so we check that and correct (kind of clamping)
        return std::isfinite(value) ? value : compositetype(KoColorSpaceMathsTraits<T>::max);
    }

    static inline T clippedMaxValue() {
        return KoColorSpaceMathsTraits<T>::max;
    }
};


template <template <typename U,
                  template <typename V> typename Policy> typename Functor,
         typename T>
struct FunctorWithSDRClampPolicy :
    std::conditional_t<std::numeric_limits<T>::is_integer,
                       Functor<T, ClampAsInteger>,
                       Functor<T, ClampAsFloatSDR>>
{
};

}

#endif // KOCOMPOSITEOPCLAMPPOLICY_H
