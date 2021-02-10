/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <type_traits>

#include "DebugPigment.h"
#include "KoConfig.h"

#ifdef HAVE_OPENEXR
#include "half.h"
#endif

#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceTraits.h>

#include "KisDitherOp.h"
#include "KisDitherMaths.h"

template<typename srcCSTraits, typename dstCSTraits, DitherType dType> class KisDitherOpImpl : public KisDitherOp
{
    using srcChannelsType = typename srcCSTraits::channels_type;
    using dstChannelsType = typename dstCSTraits::channels_type;

public:
    KisDitherOpImpl(const KoID &srcId, const KoID &dstId)
        : m_srcDepthId(srcId)
        , m_dstDepthId(dstId)
    {
    }

    void dither(const quint8 *src, quint8 *dst, int x, int y) const override
    {
        ditherImpl(src, dst, x, y);
    }

    void dither(const quint8 *srcRowStart, int srcRowStride, quint8 *dstRowStart, int dstRowStride, int x, int y, int columns, int rows) const override
    {
        ditherImpl(srcRowStart, srcRowStride, dstRowStart, dstRowStride, x, y, columns, rows);
    }

    KoID sourceDepthId() const override
    {
        return m_srcDepthId;
    }

    KoID destinationDepthId() const override
    {
        return m_dstDepthId;
    }

    DitherType type() const override
    {
        return dType;
    }

private:
    const KoID m_srcDepthId, m_dstDepthId;

    template<DitherType t = dType, typename std::enable_if<t == DITHER_NONE && std::is_same<srcCSTraits, dstCSTraits>::value, void>::type * = nullptr> inline void ditherImpl(const quint8 *src, quint8 *dst, int, int) const
    {
        memcpy(dst, src, srcCSTraits::pixelSize);
    }

    template<DitherType t = dType, typename std::enable_if<t == DITHER_NONE && !std::is_same<srcCSTraits, dstCSTraits>::value, void>::type * = nullptr> inline void ditherImpl(const quint8 *src, quint8 *dst, int, int) const
    {
        const srcChannelsType *nativeSrc = srcCSTraits::nativeArray(src);
        dstChannelsType *nativeDst = dstCSTraits::nativeArray(dst);

        for (uint channelIndex = 0; channelIndex < srcCSTraits::channels_nb; ++channelIndex) {
            nativeDst[channelIndex] = KoColorSpaceMaths<srcChannelsType, dstChannelsType>::scaleToA(nativeSrc[channelIndex]);
        }
    }

    template<DitherType t = dType, typename std::enable_if<t != DITHER_NONE, void>::type * = nullptr>
    inline void ditherImpl(const quint8 *src, quint8 *dst, int x, int y) const
    {
        const srcChannelsType *nativeSrc = srcCSTraits::nativeArray(src);
        dstChannelsType *nativeDst = dstCSTraits::nativeArray(dst);

        float f = factor(x, y);
        float s = scale();

        for (uint channelIndex = 0; channelIndex < srcCSTraits::channels_nb; ++channelIndex) {
            float c = KoColorSpaceMaths<srcChannelsType, float>::scaleToA(nativeSrc[channelIndex]);
            c = KisDitherMaths::apply_dither(c, f, s);
            nativeDst[channelIndex] = KoColorSpaceMaths<float, dstChannelsType>::scaleToA(c);
        }
    }

    template<DitherType t = dType, typename std::enable_if<t == DITHER_NONE && std::is_same<srcCSTraits, dstCSTraits>::value, void>::type * = nullptr>
    inline void ditherImpl(const quint8 *srcRowStart, int srcRowStride, quint8 *dstRowStart, int dstRowStride, int, int, int columns, int rows) const
    {
        const quint8 *nativeSrc = srcRowStart;
        quint8 *nativeDst = dstRowStart;

        for (int y = 0; y < rows; ++y) {
            memcpy(nativeDst, nativeSrc, columns);

            nativeSrc += srcRowStride;
            nativeDst += dstRowStride;
        }
    }

    template<DitherType t = dType, typename std::enable_if<t == DITHER_NONE && !std::is_same<srcCSTraits, dstCSTraits>::value, void>::type * = nullptr>
    inline void ditherImpl(const quint8 *srcRowStart, int srcRowStride, quint8 *dstRowStart, int dstRowStride, int, int, int columns, int rows) const
    {
        const quint8 *nativeSrc = srcRowStart;
        quint8 *nativeDst = dstRowStart;

        for (int y = 0; y < rows; ++y) {
            const srcChannelsType *srcPtr = srcCSTraits::nativeArray(nativeSrc);
            dstChannelsType *dstPtr = dstCSTraits::nativeArray(nativeDst);

            for (int x = 0; x < columns; ++x) {
                for (uint channelIndex = 0; channelIndex < srcCSTraits::channels_nb; ++channelIndex) {
                    dstPtr[channelIndex] = KoColorSpaceMaths<srcChannelsType, dstChannelsType>::scaleToA(srcPtr[channelIndex]);
                }

                srcPtr += srcCSTraits::channels_nb;
                dstPtr += dstCSTraits::channels_nb;
            }

            nativeSrc += srcRowStride;
            nativeDst += dstRowStride;
        }
    }

    template<DitherType t = dType, typename std::enable_if<t != DITHER_NONE, void>::type * = nullptr>
    inline void ditherImpl(const quint8 *srcRowStart, int srcRowStride, quint8 *dstRowStart, int dstRowStride, int x, int y, int columns, int rows) const
    {
        const quint8 *nativeSrc = srcRowStart;
        quint8 *nativeDst = dstRowStart;

        float s = scale();

        for (int a = 0; a < rows; ++a) {
            const srcChannelsType *srcPtr = srcCSTraits::nativeArray(nativeSrc);
            dstChannelsType *dstPtr = dstCSTraits::nativeArray(nativeDst);

            for (int b = 0; b < columns; ++b) {
                float f = factor(x + b, y + a);

                for (uint channelIndex = 0; channelIndex < srcCSTraits::channels_nb; ++channelIndex) {
                    float c = KoColorSpaceMaths<srcChannelsType, float>::scaleToA(srcPtr[channelIndex]);
                    c = KisDitherMaths::apply_dither(c, f, s);
                    dstPtr[channelIndex] = KoColorSpaceMaths<float, dstChannelsType>::scaleToA(c);
                }

                srcPtr += srcCSTraits::channels_nb;
                dstPtr += dstCSTraits::channels_nb;
            }

            nativeSrc += srcRowStride;
            nativeDst += dstRowStride;
        }
    }

    template<typename U = typename dstCSTraits::channels_type, typename std::enable_if<!std::numeric_limits<U>::is_integer, void>::type * = nullptr> constexpr float scale() const
    {
        return 0.f; // no dithering for floating point
    }

    template<typename U = typename dstCSTraits::channels_type, typename std::enable_if<std::numeric_limits<U>::is_integer, void>::type * = nullptr> constexpr float scale() const
    {
        return 1.f / static_cast<float>(1 << dstCSTraits::depth);
    }

    template<DitherType t = dType, typename std::enable_if<t == DITHER_BAYER, void>::type * = nullptr> inline float factor(int x, int y) const
    {
        return KisDitherMaths::dither_factor_bayer_8(x, y);
    }

    template<DitherType t = dType, typename std::enable_if<t == DITHER_BLUE_NOISE, void>::type * = nullptr> inline float factor(int x, int y) const
    {
        return KisDitherMaths::dither_factor_blue_noise_64(x, y);
    }
};

template<typename srcCSTraits, class dstCSTraits> inline void addDitherOpsByDepth(KoColorSpace *cs, const KoID &dstDepth)
{
    const KoID &srcDepth {cs->colorDepthId()};
    cs->addDitherOp(new KisDitherOpImpl<srcCSTraits, dstCSTraits, DITHER_NONE>(srcDepth, dstDepth));
    cs->addDitherOp(new KisDitherOpImpl<srcCSTraits, dstCSTraits, DITHER_BAYER>(srcDepth, dstDepth));
    cs->addDitherOp(new KisDitherOpImpl<srcCSTraits, dstCSTraits, DITHER_BLUE_NOISE>(srcDepth, dstDepth));
}
