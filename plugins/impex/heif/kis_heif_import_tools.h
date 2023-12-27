/**
 *  SPDX-FileCopyrightText: 2020-2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HEIF_IMPORT_TOOLS_H
#define KIS_HEIF_IMPORT_TOOLS_H

#include <cstdint>

#include <KoColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoColorTransferFunctions.h>
#include <kis_iterator_ng.h>

namespace Gray
{
template<int luma>
inline void applyValue(KisHLineIteratorSP it,
                       const uint8_t *imgG,
                       int strideG,
                       int x,
                       int y)
{
    if (luma == 8) {
        KoGrayU8Traits::setGray(it->rawData(), imgG[y * strideG + x]);
    } else {
        uint16_t source =
            KoGrayU16Traits::nativeArray(imgG)[y * (strideG / 2) + (x)];

        if (luma == 10) {
            KoGrayU16Traits::setGray(
                it->rawData(),
                static_cast<uint16_t>(float(0x03ffu & (source))
                                      * multiplier10bit * max16bit));
        } else if (luma == 12) {
            KoGrayU16Traits::setGray(
                it->rawData(),
                static_cast<uint16_t>(float(0x0fffu & (source))
                                      * multiplier12bit * max16bit));
        } else {
            KoGrayU16Traits::setGray(
                it->rawData(),
                static_cast<uint16_t>(float(source) * multiplier16bit));
        }
    }
}

template<int luma, bool hasAlpha>
inline void applyAlpha(KisHLineIteratorSP it,
                       const uint8_t *imgA,
                       int strideA,
                       int x,
                       int y)
{
    if (hasAlpha) {
        if (luma == 8) {
            KoGrayU8Traits::setOpacity(it->rawData(),
                                       quint8(imgA[y * strideA + x]),
                                       1);
        } else {
            uint16_t source =
                KoGrayU16Traits::nativeArray(imgA)[y * (strideA / 2) + x];
            if (luma == 10) {
                KoGrayU16Traits::setOpacity(
                    it->rawData(),
                    static_cast<qreal>(float(0x0fff & (source))
                                       * multiplier10bit),
                    1);
            } else if (luma == 12) {
                KoGrayU16Traits::setOpacity(
                    it->rawData(),
                    static_cast<qreal>(float(0x0fff & (source))
                                       * multiplier12bit),
                    1);
            } else {
                KoGrayU16Traits::setOpacity(
                    it->rawData(),
                    static_cast<qreal>(float(source) * multiplier16bit),
                    1);
            }
        }
    } else {
        if (luma == 8) {
            KoGrayU8Traits::setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
        } else {
            KoGrayU16Traits::setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
        }
    }
}

template<int luma, bool hasAlpha>
inline void readLayer(const int width,
                      const int height,
                      KisHLineIteratorSP it,
                      const uint8_t *imgG,
                      const uint8_t *imgA,
                      const int strideG,
                      const int strideA)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            applyValue<luma>(it, imgG, strideG, x, y);

            applyAlpha<luma, hasAlpha>(it, imgA, strideA, x, y);
            it->nextPixel();
        }

        it->nextRow();
    }
}

template<int luma, typename... Args>
inline auto readPlanarWithLuma(bool hasAlpha, Args &&...args)
{
    if (hasAlpha) {
        return Gray::readLayer<luma, true>(std::forward<Args>(args)...);
    } else {
        return Gray::readLayer<luma, false>(std::forward<Args>(args)...);
    }
}

template<typename... Args>
inline auto readPlanarLayer(const int luma, Args &&...args)
{
    if (luma == 8) {
        return readPlanarWithLuma<8>(std::forward<Args>(args)...);
    } else if (luma == 10) {
        return readPlanarWithLuma<10>(std::forward<Args>(args)...);
    } else if (luma == 12) {
        return readPlanarWithLuma<12>(std::forward<Args>(args)...);
    } else {
        return readPlanarWithLuma<16>(std::forward<Args>(args)...);
    }
}
} // namespace Gray

namespace SDR
{
struct readLayerImpl {
    template<typename Arch>
    static void create(LinearizePolicy policy,
                       bool applyOOTF,
                       bool hasAlpha,
                       const int width,
                       const int height,
                       const uint8_t *img,
                       const int stride,
                       KisHLineIteratorSP it,
                       float displayGamma,
                       float displayNits,
                       const KoColorSpace *colorSpace);
};
} // namespace SDR

namespace Planar
{
struct readLayerImpl {
    template<typename Arch>
    static void create(const int luma,
                       LinearizePolicy policy,
                       bool applyOOTF,
                       bool hasAlpha,
                       const int width,
                       const int height,
                       const uint8_t *imgR,
                       const int strideR,
                       const uint8_t *imgG,
                       const int strideG,
                       const uint8_t *imgB,
                       const int strideB,
                       const uint8_t *imgA,
                       const int strideA,
                       KisHLineIteratorSP it,
                       float displayGamma,
                       float displayNits,
                       const KoColorSpace *colorSpace);
};
} // namespace Planar

namespace HDR
{
struct readLayerImpl {
    template<typename Arch>
    static void create(const int luma,
                       LinearizePolicy linearizePolicy,
                       bool applyOOTF,
                       const int channels,
                       const int width,
                       const int height,
                       const uint8_t *img,
                       const int stride,
                       KisHLineIteratorSP it,
                       float displayGamma,
                       float displayNits,
                       const KoColorSpace *colorSpace);
};
} // namespace HDR

#endif // KIS_HEIF_IMPORT_TOOLS_H
