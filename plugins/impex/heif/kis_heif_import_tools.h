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

// Not all embedded nclx color space definitions can be converted to icc, so we
// keep an enum to load those.
enum LinearizePolicy {
    KeepTheSame,
    LinearFromPQ,
    LinearFromHLG,
    LinearFromSMPTE428
};

static constexpr float max16bit = 65535.0f;
static constexpr float multiplier10bit = 1.0f / 1023.0f;
static constexpr float multiplier12bit = 1.0f / 4095.0f;
static constexpr float multiplier16bit = 1.0f / max16bit;

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

template<LinearizePolicy policy>
inline float linearizeValueAsNeeded(float value)
{
    if (policy == LinearFromPQ) {
        return removeSmpte2048Curve(value);
    } else if (policy == LinearFromHLG) {
        return removeHLGCurve(value);
    } else if (policy == LinearFromSMPTE428) {
        return removeSMPTE_ST_428Curve(value);
    }
    return value;
}

template<LinearizePolicy linearizePolicy, bool applyOOTF>
inline void linearize(QVector<float> &pixelValues,
                      const QVector<double> &lCoef,
                      float displayGamma,
                      float displayNits)
{
    if (linearizePolicy == KeepTheSame) {
        qSwap(pixelValues[0], pixelValues[2]);
    } else if (linearizePolicy == LinearFromHLG && applyOOTF) {
        applyHLGOOTF(pixelValues, lCoef, displayGamma, displayNits);
    }
}

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
template<int luma, LinearizePolicy linearizePolicy>
inline float valueInterleaved(const uint8_t *img,
                              int stride,
                              int x,
                              int y,
                              int channels,
                              int ch)
{
    uint16_t source = reinterpret_cast<const uint16_t *>(
        img)[y * (stride / 2) + (x * channels) + ch];
    if (luma == 10) {
        return linearizeValueAsNeeded<linearizePolicy>(float(0x03ff & (source))
                                                       * multiplier10bit);
    } else if (luma == 12) {
        return linearizeValueAsNeeded<linearizePolicy>(float(0x0fff & (source))
                                                       * multiplier12bit);
    } else {
        return linearizeValueAsNeeded<linearizePolicy>(float(source)
                                                       * multiplier16bit);
    }
}

template<int luma,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         int channels>
inline void readLayer(const int width,
                      const int height,
                      const uint8_t *img,
                      const int stride,
                      KisHLineIteratorSP it,
                      float displayGamma,
                      float displayNits,
                      const KoColorSpace *colorSpace)
{
    const QVector<qreal> lCoef{colorSpace->lumaCoefficients()};
    QVector<float> pixelValues(4);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            std::fill(pixelValues.begin(), pixelValues.end(), 1.0f);

            for (int ch = 0; ch < channels; ch++) {
                pixelValues[ch] =
                    valueInterleaved<luma, linearizePolicy>(img,
                                                            stride,
                                                            x,
                                                            y,
                                                            channels,
                                                            ch);
            }

            linearize<linearizePolicy, applyOOTF>(pixelValues,
                                                  lCoef,
                                                  displayGamma,
                                                  displayNits);

            colorSpace->fromNormalisedChannelsValue(it->rawData(), pixelValues);

            it->nextPixel();
        }

        it->nextRow();
    }
}

template<int luma,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename... Args>
inline auto readInterleavedWithAlpha(bool hasAlpha, Args &&...args)
{
    if (hasAlpha) {
        return HDR::readLayer<luma, linearizePolicy, applyOOTF, 4>(
            std::forward<Args>(args)...);
    } else {
        return HDR::readLayer<luma, linearizePolicy, applyOOTF, 3>(
            std::forward<Args>(args)...);
    }
}

template<int luma, LinearizePolicy linearizePolicy, typename... Args>
inline auto readInterleavedWithPolicy(bool applyOOTF, Args &&...args)
{
    if (applyOOTF) {
        return readInterleavedWithAlpha<luma, linearizePolicy, true>(
            std::forward<Args>(args)...);
    } else {
        return readInterleavedWithAlpha<luma, linearizePolicy, false>(
            std::forward<Args>(args)...);
    }
}

template<int luma, typename... Args>
inline auto readInterleavedWithLuma(LinearizePolicy linearizePolicy,
                                    Args &&...args)
{
    if (linearizePolicy == LinearFromHLG) {
        return readInterleavedWithPolicy<luma, LinearFromHLG>(
            std::forward<Args>(args)...);
    } else if (linearizePolicy == LinearFromPQ) {
        return readInterleavedWithPolicy<luma, LinearFromPQ>(
            std::forward<Args>(args)...);
    } else if (linearizePolicy == LinearFromSMPTE428) {
        return readInterleavedWithPolicy<luma, LinearFromSMPTE428>(
            std::forward<Args>(args)...);
    } else {
        return readInterleavedWithPolicy<luma, KeepTheSame>(
            std::forward<Args>(args)...);
    }
}

template<typename... Args>
inline auto readInterleavedLayer(const int luma, Args &&...args)
{
    if (luma == 10) {
        return readInterleavedWithLuma<10>(std::forward<Args>(args)...);
    } else if (luma == 12) {
        return readInterleavedWithLuma<12>(std::forward<Args>(args)...);
    } else {
        return readInterleavedWithLuma<16>(std::forward<Args>(args)...);
    }
}
} // namespace HDR

#endif // KIS_HEIF_IMPORT_TOOLS_H
