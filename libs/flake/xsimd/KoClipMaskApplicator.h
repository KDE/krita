/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOCLIPMASKAPPLICATOR_H
#define KOCLIPMASKAPPLICATOR_H

#include <KoStreamedMath.h>
#include <KoClipMaskApplicatorBase.h>
#include <QDebug>

template<typename _impl,
         typename EnableDummyType = void>
struct KoClipMaskApplicator : public KoClipMaskApplicatorBase {
    virtual void applyLuminanceMask(quint8 *pixels, quint8 *maskPixels, const int nPixels) const override {
        KoClipMaskApplicatorBase::fallbackLuminanceMask(pixels, maskPixels, nPixels);
    }
};

#if defined(HAVE_XSIMD) && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)

template<typename _impl>
struct KoClipMaskApplicator<_impl,
        typename std::enable_if<!std::is_same<_impl, xsimd::generic>::value>::type> : public KoClipMaskApplicatorBase
{
    using uint_v = typename KoStreamedMath<_impl>::uint_v;
    using float_v = typename KoStreamedMath<_impl>::float_v;

    const uint_v mask = 0xFF;
    const quint32 colorChannelsMask = 0x00FFFFFF;
    const float redLum = 0.2125f;
    const float greenLum = 0.7154f;
    const float blueLum = 0.0721f;
    const float normCoeff = 1.0f / 255.0f;

    virtual void applyLuminanceMask(quint8 *pixels,
                                    quint8 *maskPixels,
                                    const int nPixels) const override {

        const int block = nPixels / static_cast<int>(float_v::size);
        const int block2 = nPixels % static_cast<int>(float_v::size);
        const int vectorPixelStride = 4 * static_cast<int>(float_v::size);

        for (int i = 0; i < block; i++) {
            uint_v shapeData = uint_v::load_unaligned(reinterpret_cast<const quint32 *>(pixels));
            const uint_v maskData = uint_v::load_unaligned(reinterpret_cast<const quint32 *>(maskPixels));

            const float_v maskAlpha = xsimd::to_float(xsimd::bitwise_cast_compat<int>((maskData >> 24) & mask));
            const float_v maskRed   = xsimd::to_float(xsimd::bitwise_cast_compat<int>((maskData >> 16) & mask));
            const float_v maskGreen = xsimd::to_float(xsimd::bitwise_cast_compat<int>((maskData >> 8) & mask));
            const float_v maskBlue  = xsimd::to_float(xsimd::bitwise_cast_compat<int>((maskData) & mask));
            const float_v maskValue = maskAlpha * ((redLum * maskRed) + (greenLum * maskGreen) + (blueLum * maskBlue)) * normCoeff;

            const float_v pixelAlpha = xsimd::to_float(xsimd::bitwise_cast_compat<int>(shapeData >> 24U)) * normCoeff * maskValue;
            const uint_v pixelAlpha_i = xsimd::bitwise_cast_compat<unsigned int>(xsimd::nearbyint_as_int(pixelAlpha));
            shapeData = (shapeData & colorChannelsMask) | (pixelAlpha_i << 24);

            shapeData.store_unaligned(reinterpret_cast<typename uint_v::value_type *>(pixels));

            pixels += vectorPixelStride;
            maskPixels += vectorPixelStride;
        }

        KoClipMaskApplicatorBase::fallbackLuminanceMask(pixels, maskPixels, block2);
    }
};

#endif /* HAVE_XSIMD */

#endif // KOCLIPMASKAPPLICATOR_H
