/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDitherOpImpl.h"

/**
 * THIS CLASS OVERRIDES THE STANDARD FACTORY.
 * Floating point CMYK uses different normalization for the color
 * and alpha channels.
 */

template<typename srcCSTraits, typename dstCSTraits, DitherType dType> class KisCmykDitherOpImpl : public KisDitherOpImpl<srcCSTraits, dstCSTraits, dType>
{
    using srcChannelsType = typename srcCSTraits::channels_type;
    using dstChannelsType = typename dstCSTraits::channels_type;

public:
    KisCmykDitherOpImpl(const KoID &srcId, const KoID &dstId)
        : KisDitherOpImpl<srcCSTraits, dstCSTraits, dType>(srcId, dstId)
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

private:
    template<DitherType t = dType, typename std::enable_if<t == DITHER_NONE && std::is_same<srcCSTraits, dstCSTraits>::value, void>::type * = nullptr> inline void ditherImpl(const quint8 *src, quint8 *dst, int, int) const
    {
        memcpy(dst, src, srcCSTraits::pixelSize);
    }

    template<DitherType t = dType, typename std::enable_if<t == DITHER_NONE && !std::is_same<srcCSTraits, dstCSTraits>::value, void>::type * = nullptr> inline void ditherImpl(const quint8 *src, quint8 *dst, int, int) const
    {
        const srcChannelsType *nativeSrc = srcCSTraits::nativeArray(src);
        dstChannelsType *nativeDst = dstCSTraits::nativeArray(dst);

        for (uint channelIndex = 0; channelIndex < srcCSTraits::channels_nb; ++channelIndex) {
            if (channelIndex == srcCSTraits::alpha_pos) {
                // The standard normalization.
                nativeDst[channelIndex] = KoColorSpaceMaths<srcChannelsType, dstChannelsType>::scaleToA(nativeSrc[channelIndex]);
            } else {
                // Normalize using unitCMYKValue.
                nativeDst[channelIndex] = scaleToA<srcChannelsType, dstChannelsType>(nativeSrc[channelIndex]);
            }
        }
    }

    template<DitherType t = dType, typename std::enable_if<t != DITHER_NONE, void>::type * = nullptr> inline void ditherImpl(const quint8 *src, quint8 *dst, int x, int y) const
    {
        const srcChannelsType *nativeSrc = srcCSTraits::nativeArray(src);
        dstChannelsType *nativeDst = dstCSTraits::nativeArray(dst);

        float f = factor(x, y);
        float s = scale();

        // In (non-integer) CMYK, all channels except alpha are normalized
        // to a different range, [0, 100].
        for (uint channelIndex = 0; channelIndex < srcCSTraits::channels_nb; ++channelIndex) {
            if (channelIndex == srcCSTraits::alpha_pos) {
                // The standard normalization.
                float c = KoColorSpaceMaths<srcChannelsType, float>::scaleToA(nativeSrc[channelIndex]);
                c = KisDitherMaths::apply_dither(c, f, s);
                nativeDst[channelIndex] = KoColorSpaceMaths<float, dstChannelsType>::scaleToA(c);
                ;
            } else {
                // Normalize using unitCMYKValue.
                float c = normalize<srcChannelsType>(nativeSrc[channelIndex]);
                c = KisDitherMaths::apply_dither(c, f, s);
                nativeDst[channelIndex] = denormalize<dstChannelsType>(c);
                ;
            }
        }
    }

    template<DitherType t = dType, typename std::enable_if<t == DITHER_NONE && std::is_same<srcCSTraits, dstCSTraits>::value, void>::type * = nullptr>
    inline void ditherImpl(const quint8 *srcRowStart, int srcRowStride, quint8 *dstRowStart, int dstRowStride, int, int, int columns, int rows) const
    {
        const quint8 *nativeSrc = srcRowStart;
        quint8 *nativeDst = dstRowStart;

        for (int y = 0; y < rows; ++y) {
            memcpy(nativeDst, nativeSrc, srcCSTraits::pixelSize * columns);

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
                    if (channelIndex == srcCSTraits::alpha_pos) {
                        // The standard normalization.
                        dstPtr[channelIndex] = KoColorSpaceMaths<srcChannelsType, dstChannelsType>::scaleToA(srcPtr[channelIndex]);
                    } else {
                        // Normalize using unitCMYKValue.
                        dstPtr[channelIndex] = scaleToA<srcChannelsType, dstChannelsType>(srcPtr[channelIndex]);
                    }
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
                    if (channelIndex == srcCSTraits::alpha_pos) {
                        // The standard normalization.
                        float c = KoColorSpaceMaths<srcChannelsType, float>::scaleToA(srcPtr[channelIndex]);
                        c = KisDitherMaths::apply_dither(c, f, s);
                        dstPtr[channelIndex] = KoColorSpaceMaths<float, dstChannelsType>::scaleToA(c);
                        ;
                    } else {
                        // Normalize using unitCMYKValue.
                        float c = normalize<srcChannelsType>(srcPtr[channelIndex]);
                        c = KisDitherMaths::apply_dither(c, f, s);
                        dstPtr[channelIndex] = denormalize<dstChannelsType>(c);
                        ;
                    }
                }

                srcPtr += srcCSTraits::channels_nb;
                dstPtr += dstCSTraits::channels_nb;
            }

            nativeSrc += srcRowStride;
            nativeDst += dstRowStride;
        }
    }

    // CMYK-specific normalization bits

    template<typename A, typename U = srcCSTraits, typename std::enable_if<std::numeric_limits<A>::is_integer, void>::type * = nullptr> inline float normalize(A value) const
    {
        return static_cast<float>(value) / KoColorSpaceMathsTraits<A>::unitValue;
    };

    template<typename A, typename std::enable_if<!std::numeric_limits<A>::is_integer, void>::type * = nullptr> inline float normalize(A value) const
    {
        return static_cast<float>(value) / KoCmykColorSpaceMathsTraits<A>::unitValueCMYK;
    };

    template<typename A, typename std::enable_if<std::numeric_limits<A>::is_integer, void>::type * = nullptr> inline A denormalize(float value) const
    {
        return static_cast<A>(value * static_cast<float>(KoColorSpaceMathsTraits<A>::unitValue));
    };

    template<typename A, typename std::enable_if<!std::numeric_limits<A>::is_integer, void>::type * = nullptr> inline A denormalize(float value) const
    {
        return static_cast<A>(value * static_cast<float>(KoCmykColorSpaceMathsTraits<A>::unitValueCMYK));
    };

    template<typename A, typename B> inline B scaleToA(A c) const
    {
        return denormalize<B>(normalize<A>(c));
    }

    template<typename U = typename dstCSTraits::channels_type, typename std::enable_if<!std::numeric_limits<U>::is_integer, void>::type * = nullptr> inline constexpr float scale() const
    {
        return 0.f; // no dithering for floating point
    }

    template<typename U = typename dstCSTraits::channels_type, typename std::enable_if<std::numeric_limits<U>::is_integer, void>::type * = nullptr> inline constexpr float scale() const
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

template<typename srcCSTraits, typename dstCSTraits> inline void addCmykDitherOpsByDepth(KoColorSpace *cs, const KoID &dstDepth)
{
    const KoID &srcDepth {cs->colorDepthId()};
    cs->addDitherOp(new KisCmykDitherOpImpl<srcCSTraits, dstCSTraits, DITHER_NONE>(srcDepth, dstDepth));
    cs->addDitherOp(new KisCmykDitherOpImpl<srcCSTraits, dstCSTraits, DITHER_BAYER>(srcDepth, dstDepth));
    cs->addDitherOp(new KisCmykDitherOpImpl<srcCSTraits, dstCSTraits, DITHER_BLUE_NOISE>(srcDepth, dstDepth));
}

template<class srcCSTraits> inline void addStandardDitherOps(KoColorSpace *cs)
{
    static_assert(std::is_same<srcCSTraits, KoCmykU8Traits>::value || std::is_same<srcCSTraits, KoCmykU16Traits>::value ||
#ifdef HAVE_OPENEXR
                      std::is_same<srcCSTraits, KoCmykF16Traits>::value ||
#endif
                      std::is_same<srcCSTraits, KoCmykF32Traits>::value,
                  "Missing colorspace, add a transform case!");

    KIS_ASSERT(cs->pixelSize() == srcCSTraits::pixelSize);

    addCmykDitherOpsByDepth<srcCSTraits, KoCmykU8Traits>(cs, Integer8BitsColorDepthID);
    addCmykDitherOpsByDepth<srcCSTraits, KoCmykU16Traits>(cs, Integer16BitsColorDepthID);
#ifdef HAVE_OPENEXR
    addCmykDitherOpsByDepth<srcCSTraits, KoCmykF16Traits>(cs, Float16BitsColorDepthID);
#endif
    addCmykDitherOpsByDepth<srcCSTraits, KoCmykF32Traits>(cs, Float32BitsColorDepthID);
}
