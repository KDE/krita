/**
 *  SPDX-FileCopyrightText: 2020-2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HEIF_EXPORT_TOOLS_H
#define KIS_HEIF_EXPORT_TOOLS_H

#include <cstdint>

#include <KoColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoColorTransferFunctions.h>
#include <kis_iterator_ng.h>

enum ConversionPolicy { KeepTheSame, ApplyPQ, ApplyHLG, ApplySMPTE428 };

constexpr uint16_t max12bit = 4095;
constexpr uint16_t max16bit = 65535;
constexpr float multiplier16bit = (1.0f / float(max16bit));

namespace Planar
{
template<bool hasAlpha>
inline void writeLayerImpl(const int width,
                           const int height,
                           uint8_t *ptrR,
                           const int strideR,
                           uint8_t *ptrG,
                           const int strideG,
                           uint8_t *ptrB,
                           const int strideB,
                           uint8_t *ptrA,
                           const int strideA,
                           KisHLineConstIteratorSP it)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const quint8 *data = it->rawDataConst();
            ptrR[y * strideR + x] = KoBgrU8Traits::red(data);
            ptrG[y * strideG + x] = KoBgrU8Traits::green(data);
            ptrB[y * strideB + x] = KoBgrU8Traits::blue(data);

            // If we already employ KoBgrU8Traits to access the iterator,
            // we can use it for the opacity too. -amyspark
            if (hasAlpha) {
                ptrA[y * strideA + x] = KoBgrU8Traits::opacityU8(data);
            }

            it->nextPixel();
        }

        it->nextRow();
    }
}

template<typename... Args>
inline auto writeLayer(bool hasAlpha, Args &&...args)
{
    if (hasAlpha) {
        return Planar::writeLayerImpl<true>(std::forward<Args>(args)...);
    } else {
        return Planar::writeLayerImpl<false>(std::forward<Args>(args)...);
    }
}
} // namespace Planar

namespace HDR
{
template<int endValue0, int endValue1, int channels>
inline void writeLayerImpl(const int width,
                           const int height,
                           uint8_t *ptr,
                           const int stride,
                           KisHLineConstIteratorSP it)
{
    std::array<KoBgrU16Traits::channels_type, channels> pixelValues{};

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const quint8 *data = it->rawDataConst();
            pixelValues[0] = KoBgrU16Traits::red(data);
            pixelValues[1] = KoBgrU16Traits::green(data);
            pixelValues[2] = KoBgrU16Traits::blue(data);
            if (channels == 4) {
                pixelValues[3] = static_cast<KoBgrU16Traits::channels_type>(
                    KoBgrU16Traits::opacityF(data) * max16bit);
            }

            for (int ch = 0; ch < channels; ch++) {
                uint16_t v = qBound<uint16_t>(
                    0,
                    static_cast<uint16_t>(float(pixelValues[ch])
                                          * multiplier16bit * max12bit),
                    max12bit);
                ptr[2 * (x * channels) + y * stride + endValue0 + (ch * 2)] =
                    static_cast<uint8_t>(v >> 8);
                ptr[2 * (x * channels) + y * stride + endValue1 + (ch * 2)] =
                    static_cast<uint8_t>(v & 0xFF);
            }

            it->nextPixel();
        }

        it->nextRow();
    }
}

template<int endValue0, int endValue1, typename... Args>
inline auto writeInterleavedWithAlpha(bool hasAlpha, Args &&...args)
{
    if (hasAlpha) {
        return HDR::writeLayerImpl<endValue0, endValue1, 4>(
            std::forward<Args>(args)...);
    } else {
        return HDR::writeLayerImpl<endValue0, endValue1, 3>(
            std::forward<Args>(args)...);
    }
}

template<typename... Args>
inline auto writeInterleavedLayer(QSysInfo::Endian endian, Args &&...args)
{
    if (endian == QSysInfo::LittleEndian) {
        return HDR::writeInterleavedWithAlpha<1, 0>(
            std::forward<Args>(args)...);
    } else {
        return HDR::writeInterleavedWithAlpha<0, 1>(
            std::forward<Args>(args)...);
    }
}
} // namespace HDR

#endif // KIS_HEIF_EXPORT_TOOLS_H
