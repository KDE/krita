/**
 *  SPDX-FileCopyrightText: 2020-2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_heif_import_tools.h>

#if XSIMD_UNIVERSAL_BUILD_PASS

namespace Planar
{
#if defined HAVE_XSIMD && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)
template<typename Arch,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename std::enable_if_t<!std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
inline void linearize(float *pixelValues,
                      const double *lCoef,
                      float displayGamma,
                      float displayNits)
{
    using float_v = typename KoColorTransferFunctions<Arch>::float_v;
    if (linearizePolicy == LinearizePolicy::LinearFromPQ) {
        auto v = float_v::load_unaligned(pixelValues);
        KoColorTransferFunctions<Arch>::removeSmpte2048Curve(v);
        v.store_unaligned(pixelValues);
    } else if (linearizePolicy == LinearizePolicy::LinearFromHLG) {
        auto v = float_v::load_unaligned(pixelValues);
        KoColorTransferFunctions<Arch>::removeHLGCurve(v);
        v.store_unaligned(pixelValues);
    } else if (linearizePolicy == LinearizePolicy::LinearFromSMPTE428) {
        auto v = float_v::load_unaligned(pixelValues);
        KoColorTransferFunctions<Arch>::removeSMPTE_ST_428Curve(v);
        v.store_unaligned(pixelValues);
    }

    if (linearizePolicy == LinearizePolicy::KeepTheSame) {
        qSwap(pixelValues[0], pixelValues[2]);
    } else if (linearizePolicy == LinearizePolicy::LinearFromHLG && applyOOTF) {
        applyHLGOOTF(pixelValues, lCoef, displayGamma, displayNits);
    }
}
#endif

template<typename Arch,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename std::enable_if_t<std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
inline void linearize(float *pixelValues,
                      const double *lCoef,
                      float displayGamma,
                      float displayNits)
{
    if (linearizePolicy == LinearizePolicy::KeepTheSame) {
        qSwap(pixelValues[0], pixelValues[2]);
    } else if (linearizePolicy == LinearizePolicy::LinearFromHLG && applyOOTF) {
        applyHLGOOTF(pixelValues, lCoef, displayGamma, displayNits);
    }
}

template<typename Arch,
         int luma,
         LinearizePolicy linearizePolicy,
         typename std::enable_if_t<!std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
inline float value(const uint8_t *img, int stride, int x, int y)
{
    if (luma == 8) {
        return float(img[y * (stride) + x]) / 255.0f;
    } else {
        uint16_t source =
            reinterpret_cast<const uint16_t *>(img)[y * (stride / 2) + x];
        if (luma == 10) {
            return float(0x03ff & (source)) * multiplier10bit;
        } else if (luma == 12) {
            return float(0x0fff & (source)) * multiplier12bit;
        } else {
            return float(source) * multiplier16bit;
        }
    }
}

template<LinearizePolicy policy>
inline float linearizeValueAsNeeded(float value)
{
    if (policy == LinearizePolicy::LinearFromPQ) {
        return removeSmpte2048Curve(value);
    } else if (policy == LinearizePolicy::LinearFromHLG) {
        return removeHLGCurve(value);
    } else if (policy == LinearizePolicy::LinearFromSMPTE428) {
        return removeSMPTE_ST_428Curve(value);
    }
    return value;
}

template<typename Arch,
         int luma,
         LinearizePolicy linearizePolicy,
         typename std::enable_if_t<std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
inline float value(const uint8_t *img, int stride, int x, int y)
{
    if (luma == 8) {
        return linearizeValueAsNeeded<linearizePolicy>(
            float(img[y * (stride) + x]) / 255.0f);
    } else {
        uint16_t source =
            reinterpret_cast<const uint16_t *>(img)[y * (stride / 2) + x];
        if (luma == 10) {
            return linearizeValueAsNeeded<linearizePolicy>(
                float(0x03ff & (source)) * multiplier10bit);
        } else if (luma == 12) {
            return linearizeValueAsNeeded<linearizePolicy>(
                float(0x0fff & (source)) * multiplier12bit);
        } else {
            return linearizeValueAsNeeded<linearizePolicy>(float(source)
                                                           * multiplier16bit);
        }
    }
}

template<typename Arch,
         typename std::enable_if_t<std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
constexpr int bufferSize()
{
    return 4;
}

#if defined HAVE_XSIMD && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)
template<typename Arch,
         typename std::enable_if_t<!std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
constexpr int bufferSize()
{
    return qMax<int>(4, KoStreamedMath<Arch>::float_v::size);
}
#endif

template<typename Arch,
         int luma,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         bool hasAlpha>
inline void readLayer(const int width,
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
                      const KoColorSpace *colorSpace)
{
    const QVector<qreal> lCoef{colorSpace->lumaCoefficients()};
    QVector<float> pixelValues(bufferSize<Arch>());
    float *data = pixelValues.data();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int i = 0; i < bufferSize<Arch>(); i++) {
                data[i] = 1.0f;
            }

            data[0] = value<Arch, luma, linearizePolicy>(imgR, strideR, x, y);
            data[1] = value<Arch, luma, linearizePolicy>(imgG, strideG, x, y);
            data[2] = value<Arch, luma, linearizePolicy>(imgB, strideB, x, y);

            linearize<Arch, linearizePolicy, applyOOTF>(data,
                                                        lCoef.constData(),
                                                        displayGamma,
                                                        displayNits);

            if (hasAlpha) {
                data[3] =
                    value<Arch, luma, LinearizePolicy::KeepTheSame>(imgA,
                                                                    strideA,
                                                                    x,
                                                                    y);
            }

            if (luma == 8) {
                KoBgrU8Traits::fromNormalisedChannelsValue(it->rawData(),
                                                           pixelValues);
            } else if (luma > 8 && linearizePolicy != LinearizePolicy::KeepTheSame) {
                KoBgrF32Traits::fromNormalisedChannelsValue(it->rawData(),
                                                            pixelValues);
            } else {
                KoBgrU16Traits::fromNormalisedChannelsValue(it->rawData(),
                                                            pixelValues);
            }

            it->nextPixel();
        }

        it->nextRow();
    }
}

template<typename Arch,
         int luma,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename... Args>
inline auto readPlanarLayerWithAlpha(bool hasAlpha, Args &&...args)
{
    if (hasAlpha) {
        return Planar::readLayer<Arch, luma, linearizePolicy, applyOOTF, true>(
            std::forward<Args>(args)...);
    } else {
        return Planar::readLayer<Arch, luma, linearizePolicy, applyOOTF, false>(
            std::forward<Args>(args)...);
    }
}

template<typename Arch,
         int luma,
         LinearizePolicy linearizePolicy,
         typename... Args>
inline auto readPlanarLayerWithPolicy(bool applyOOTF, Args &&...args)
{
    if (applyOOTF) {
        return readPlanarLayerWithAlpha<Arch, luma, linearizePolicy, true>(
            std::forward<Args>(args)...);
    } else {
        return readPlanarLayerWithAlpha<Arch, luma, linearizePolicy, false>(
            std::forward<Args>(args)...);
    }
}

template<typename Arch, int luma, typename... Args>
inline auto readPlanarLayerWithLuma(LinearizePolicy linearizePolicy,
                                    Args &&...args)
{
    if (linearizePolicy == LinearizePolicy::LinearFromHLG) {
        return readPlanarLayerWithPolicy<Arch, luma, LinearizePolicy::LinearFromHLG>(std::forward<Args>(args)...);
    } else if (linearizePolicy == LinearizePolicy::LinearFromPQ) {
        return readPlanarLayerWithPolicy<Arch, luma, LinearizePolicy::LinearFromPQ>(std::forward<Args>(args)...);
    } else if (linearizePolicy == LinearizePolicy::LinearFromSMPTE428) {
        return readPlanarLayerWithPolicy<Arch, luma, LinearizePolicy::LinearFromSMPTE428>(std::forward<Args>(args)...);
    } else {
        return readPlanarLayerWithPolicy<Arch, luma, LinearizePolicy::KeepTheSame>(std::forward<Args>(args)...);
    }
}

template<typename Arch>
void readLayerImpl::create(const int luma,
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
                           const KoColorSpace *colorSpace)
{
    if (luma == 8) {
        return readPlanarLayerWithLuma<xsimd::current_arch, 8>(policy,
                                                               applyOOTF,
                                                               hasAlpha,
                                                               width,
                                                               height,
                                                               imgR,
                                                               strideR,
                                                               imgG,
                                                               strideG,
                                                               imgB,
                                                               strideB,
                                                               imgA,
                                                               strideA,
                                                               it,
                                                               displayGamma,
                                                               displayNits,
                                                               colorSpace);
    } else if (luma == 10) {
        return readPlanarLayerWithLuma<xsimd::current_arch, 10>(policy,
                                                                applyOOTF,
                                                                hasAlpha,
                                                                width,
                                                                height,
                                                                imgR,
                                                                strideR,
                                                                imgG,
                                                                strideG,
                                                                imgB,
                                                                strideB,
                                                                imgA,
                                                                strideA,
                                                                it,
                                                                displayGamma,
                                                                displayNits,
                                                                colorSpace);
    } else if (luma == 12) {
        return readPlanarLayerWithLuma<xsimd::current_arch, 12>(policy,
                                                                applyOOTF,
                                                                hasAlpha,
                                                                width,
                                                                height,
                                                                imgR,
                                                                strideR,
                                                                imgG,
                                                                strideG,
                                                                imgB,
                                                                strideB,
                                                                imgA,
                                                                strideA,
                                                                it,
                                                                displayGamma,
                                                                displayNits,
                                                                colorSpace);
    } else {
        return readPlanarLayerWithLuma<xsimd::current_arch, 16>(policy,
                                                                applyOOTF,
                                                                hasAlpha,
                                                                width,
                                                                height,
                                                                imgR,
                                                                strideR,
                                                                imgG,
                                                                strideG,
                                                                imgB,
                                                                strideB,
                                                                imgA,
                                                                strideA,
                                                                it,
                                                                displayGamma,
                                                                displayNits,
                                                                colorSpace);
    }
}

template void
readLayerImpl::create<xsimd::current_arch>(const int luma,
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

} // namespace Planar

namespace HDR
{
#if defined HAVE_XSIMD && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)
template<typename Arch,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename std::enable_if_t<!std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
inline void linearize(float *pixelValues,
                      const double *lCoef,
                      float displayGamma,
                      float displayNits)
{
    using float_v = typename KoColorTransferFunctions<Arch>::float_v;
    if (linearizePolicy == LinearizePolicy::LinearFromPQ) {
        auto v = float_v::load_unaligned(pixelValues);
        KoColorTransferFunctions<Arch>::removeSmpte2048Curve(v);
        v.store_unaligned(pixelValues);
    } else if (linearizePolicy == LinearizePolicy::LinearFromHLG) {
        auto v = float_v::load_unaligned(pixelValues);
        KoColorTransferFunctions<Arch>::removeHLGCurve(v);
        v.store_unaligned(pixelValues);
    } else if (linearizePolicy == LinearizePolicy::LinearFromSMPTE428) {
        auto v = float_v::load_unaligned(pixelValues);
        KoColorTransferFunctions<Arch>::removeSMPTE_ST_428Curve(v);
        v.store_unaligned(pixelValues);
    }

    if (linearizePolicy == LinearizePolicy::KeepTheSame) {
        qSwap(pixelValues[0], pixelValues[2]);
    } else if (linearizePolicy == LinearizePolicy::LinearFromHLG && applyOOTF) {
        applyHLGOOTF(pixelValues, lCoef, displayGamma, displayNits);
    }
}
#endif

template<typename Arch,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename std::enable_if_t<std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
inline void linearize(float *pixelValues,
                      const double *lCoef,
                      float displayGamma,
                      float displayNits)
{
    if (linearizePolicy == LinearizePolicy::KeepTheSame) {
        qSwap(pixelValues[0], pixelValues[2]);
    } else if (linearizePolicy == LinearizePolicy::LinearFromHLG && applyOOTF) {
        applyHLGOOTF(pixelValues, lCoef, displayGamma, displayNits);
    }
}

template<typename Arch,
         int luma,
         LinearizePolicy linearizePolicy,
         typename std::enable_if_t<!std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
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
        return float(0x03ff & (source)) * multiplier10bit;
    } else if (luma == 12) {
        return float(0x0fff & (source)) * multiplier12bit;
    } else {
        return float(source) * multiplier16bit;
    }
}

template<LinearizePolicy policy>
inline float linearizeValueAsNeeded(float value)
{
    if (policy == LinearizePolicy::LinearFromPQ) {
        return removeSmpte2048Curve(value);
    } else if (policy == LinearizePolicy::LinearFromHLG) {
        return removeHLGCurve(value);
    } else if (policy == LinearizePolicy::LinearFromSMPTE428) {
        return removeSMPTE_ST_428Curve(value);
    }
    return value;
}

template<typename Arch,
         int luma,
         LinearizePolicy linearizePolicy,
         typename std::enable_if_t<std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
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

template<typename Arch,
         int channels,
         typename std::enable_if_t<std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
constexpr int bufferSize()
{
    return channels;
}

#if defined HAVE_XSIMD && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)
template<typename Arch,
         int channels,
         typename std::enable_if_t<!std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
constexpr int bufferSize()
{
    return qMax<int>(channels, KoStreamedMath<Arch>::float_v::size);
}
#endif

template<typename Arch,
         int luma,
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
    QVector<float> pixelValues(bufferSize<Arch, channels>());
    float *data = pixelValues.data();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int i = 0; i < bufferSize<Arch, channels>(); i++) {
                data[i] = 1.0f;
            }

            const int alphaPos = [&]() {
                if (luma == 8) {
                    return KoBgrU8Traits::alpha_pos;
                } else if (luma > 8
                           && linearizePolicy != LinearizePolicy::KeepTheSame) {
                    return KoBgrF32Traits::alpha_pos;
                } else {
                    return KoBgrU16Traits::alpha_pos;
                }
            }();

            for (int ch = 0; ch < channels; ch++) {
                if (ch == alphaPos) {
                    data[ch] =
                        valueInterleaved<Arch,
                                         luma,
                                         LinearizePolicy::KeepTheSame>(img,
                                                                       stride,
                                                                       x,
                                                                       y,
                                                                       channels,
                                                                       ch);
                } else {
                    data[ch] =
                        valueInterleaved<Arch, luma, linearizePolicy>(img,
                                                                      stride,
                                                                      x,
                                                                      y,
                                                                      channels,
                                                                      ch);
                }
            }

            linearize<Arch, linearizePolicy, applyOOTF>(data,
                                                        lCoef.constData(),
                                                        displayGamma,
                                                        displayNits);

            if (luma == 8) {
                KoBgrU8Traits::fromNormalisedChannelsValue(it->rawData(),
                                                           pixelValues);
            } else if (luma > 8 && linearizePolicy != LinearizePolicy::KeepTheSame) {
                KoBgrF32Traits::fromNormalisedChannelsValue(it->rawData(),
                                                            pixelValues);
            } else {
                KoBgrU16Traits::fromNormalisedChannelsValue(it->rawData(),
                                                            pixelValues);
            }

            it->nextPixel();
        }

        it->nextRow();
    }
}

template<typename Arch,
         int luma,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename... Args>
inline auto readInterleavedWithAlpha(bool hasAlpha, Args &&...args)
{
    if (hasAlpha) {
        return HDR::readLayer<Arch, luma, linearizePolicy, applyOOTF, 4>(
            std::forward<Args>(args)...);
    } else {
        return HDR::readLayer<Arch, luma, linearizePolicy, applyOOTF, 3>(
            std::forward<Args>(args)...);
    }
}

template<typename Arch,
         int luma,
         LinearizePolicy linearizePolicy,
         typename... Args>
inline auto readInterleavedWithPolicy(bool applyOOTF, Args &&...args)
{
    if (applyOOTF) {
        return readInterleavedWithAlpha<Arch, luma, linearizePolicy, true>(
            std::forward<Args>(args)...);
    } else {
        return readInterleavedWithAlpha<Arch, luma, linearizePolicy, false>(
            std::forward<Args>(args)...);
    }
}

template<typename Arch, int luma, typename... Args>
inline auto readInterleavedWithLuma(LinearizePolicy linearizePolicy,
                                    Args &&...args)
{
    if (linearizePolicy == LinearizePolicy::LinearFromHLG) {
        return readInterleavedWithPolicy<Arch, luma, LinearizePolicy::LinearFromHLG>(std::forward<Args>(args)...);
    } else if (linearizePolicy == LinearizePolicy::LinearFromPQ) {
        return readInterleavedWithPolicy<Arch, luma, LinearizePolicy::LinearFromPQ>(std::forward<Args>(args)...);
    } else if (linearizePolicy == LinearizePolicy::LinearFromSMPTE428) {
        return readInterleavedWithPolicy<Arch, luma, LinearizePolicy::LinearFromSMPTE428>(std::forward<Args>(args)...);
    } else {
        return readInterleavedWithPolicy<Arch, luma, LinearizePolicy::KeepTheSame>(std::forward<Args>(args)...);
    }
}

template<typename Arch>
void readLayerImpl::create(const int luma,
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
                           const KoColorSpace *colorSpace)
{
    if (luma == 10) {
        return readInterleavedWithLuma<Arch, 10>(linearizePolicy,
                                                 applyOOTF,
                                                 channels,
                                                 width,
                                                 height,
                                                 img,
                                                 stride,
                                                 it,
                                                 displayGamma,
                                                 displayNits,
                                                 colorSpace);
    } else if (luma == 12) {
        return readInterleavedWithLuma<Arch, 12>(linearizePolicy,
                                                 applyOOTF,
                                                 channels,
                                                 width,
                                                 height,
                                                 img,
                                                 stride,
                                                 it,
                                                 displayGamma,
                                                 displayNits,
                                                 colorSpace);
    } else {
        return readInterleavedWithLuma<Arch, 16>(linearizePolicy,
                                                 applyOOTF,
                                                 channels,
                                                 width,
                                                 height,
                                                 img,
                                                 stride,
                                                 it,
                                                 displayGamma,
                                                 displayNits,
                                                 colorSpace);
    }
}

template void
readLayerImpl::create<xsimd::current_arch>(const int luma,
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
} // namespace HDR

namespace SDR
{
#if defined HAVE_XSIMD && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)
template<typename Arch,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename std::enable_if_t<!std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
inline void linearize(float *pixelValues,
                      const double *lCoef,
                      float displayGamma,
                      float displayNits)
{
    using float_v = typename KoColorTransferFunctions<Arch>::float_v;
    if (linearizePolicy == LinearizePolicy::LinearFromPQ) {
        auto v = float_v::load_unaligned(pixelValues);
        KoColorTransferFunctions<Arch>::removeSmpte2048Curve(v);
        v.store_unaligned(pixelValues);
    } else if (linearizePolicy == LinearizePolicy::LinearFromHLG) {
        auto v = float_v::load_unaligned(pixelValues);
        KoColorTransferFunctions<Arch>::removeHLGCurve(v);
        v.store_unaligned(pixelValues);
    } else if (linearizePolicy == LinearizePolicy::LinearFromSMPTE428) {
        auto v = float_v::load_unaligned(pixelValues);
        KoColorTransferFunctions<Arch>::removeSMPTE_ST_428Curve(v);
        v.store_unaligned(pixelValues);
    }

    if (linearizePolicy == LinearizePolicy::KeepTheSame) {
        qSwap(pixelValues[0], pixelValues[2]);
    } else if (linearizePolicy == LinearizePolicy::LinearFromHLG && applyOOTF) {
        applyHLGOOTF(pixelValues, lCoef, displayGamma, displayNits);
    }
}
#endif

template<typename Arch,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename std::enable_if_t<std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
inline void linearize(float *pixelValues,
                      const double *lCoef,
                      float displayGamma,
                      float displayNits)
{
    if (linearizePolicy == LinearizePolicy::KeepTheSame) {
        qSwap(pixelValues[0], pixelValues[2]);
    } else if (linearizePolicy == LinearizePolicy::LinearFromHLG && applyOOTF) {
        applyHLGOOTF(pixelValues, lCoef, displayGamma, displayNits);
    }
}

template<typename Arch,
         LinearizePolicy linearizePolicy,
         int channels,
         typename std::enable_if_t<!std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
inline float value(const uint8_t *img, int stride, int x, int y, int ch)
{
    uint8_t source = img[(y * stride) + (x * channels) + ch];
    return float(source) / 255.0f;
}

template<LinearizePolicy policy>
inline float linearizeValueAsNeeded(float value)
{
    if (policy == LinearizePolicy::LinearFromPQ) {
        return removeSmpte2048Curve(value);
    } else if (policy == LinearizePolicy::LinearFromHLG) {
        return removeHLGCurve(value);
    } else if (policy == LinearizePolicy::LinearFromSMPTE428) {
        return removeSMPTE_ST_428Curve(value);
    }
    return value;
}

template<typename Arch,
         LinearizePolicy linearizePolicy,
         int channels,
         typename std::enable_if_t<std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
inline float value(const uint8_t *img, int stride, int x, int y, int ch)
{
    uint8_t source = img[(y * stride) + (x * channels) + ch];
    return linearizeValueAsNeeded<linearizePolicy>(float(source) / 255.0f);
}

template<typename Arch,
         int channels,
         typename std::enable_if_t<std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
constexpr int bufferSize()
{
    return channels;
}

#if defined HAVE_XSIMD && !defined(XSIMD_NO_SUPPORTED_ARCHITECTURE)
template<typename Arch,
         int channels,
         typename std::enable_if_t<!std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
constexpr int bufferSize()
{
    return qMax<int>(channels, KoStreamedMath<Arch>::float_v::size);
}
#endif

template<typename Arch,
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
    QVector<float> pixelValues(bufferSize<Arch, channels>());
    float *data = pixelValues.data();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int i = 0; i < bufferSize<Arch, channels>(); i++) {
                data[i] = 0;
            }

            for (int ch = 0; ch < channels; ch++) {
                if (ch == KoBgrU8Traits::alpha_pos) {
                    data[ch] =
                        value<Arch, LinearizePolicy::KeepTheSame, channels>(
                            img,
                            stride,
                            x,
                            y,
                            ch);
                } else {
                    data[ch] = value<Arch, linearizePolicy, channels>(img,
                                                                      stride,
                                                                      x,
                                                                      y,
                                                                      ch);
                }
            }

            linearize<Arch, linearizePolicy, applyOOTF>(data,
                                                        lCoef.constData(),
                                                        displayGamma,
                                                        displayNits);

            KoBgrU8Traits::fromNormalisedChannelsValue(it->rawData(),
                                                       pixelValues);

            it->nextPixel();
        }

        it->nextRow();
    }
}

template<typename Arch,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename... Args>
inline auto readInterleavedWithAlpha(bool hasAlpha, Args &&...args)
{
    if (hasAlpha) {
        return SDR::readLayer<Arch, linearizePolicy, applyOOTF, 4>(
            std::forward<Args>(args)...);
    } else {
        return SDR::readLayer<Arch, linearizePolicy, applyOOTF, 3>(
            std::forward<Args>(args)...);
    }
}

template<typename Arch, LinearizePolicy linearizePolicy, typename... Args>
inline auto readInterleavedWithPolicy(bool applyOOTF, Args &&...args)
{
    if (applyOOTF) {
        return readInterleavedWithAlpha<Arch, linearizePolicy, true>(
            std::forward<Args>(args)...);
    } else {
        return readInterleavedWithAlpha<Arch, linearizePolicy, false>(
            std::forward<Args>(args)...);
    }
}

template<typename Arch>
void readLayerImpl::create(LinearizePolicy linearizePolicy,
                           bool applyOOTF,
                           bool hasAlpha,
                           const int width,
                           const int height,
                           const uint8_t *img,
                           const int stride,
                           KisHLineIteratorSP it,
                           float displayGamma,
                           float displayNits,
                           const KoColorSpace *colorSpace)
{
    if (linearizePolicy == LinearizePolicy::LinearFromHLG) {
        return readInterleavedWithPolicy<Arch, LinearizePolicy::LinearFromHLG>(applyOOTF,
                                                                               hasAlpha,
                                                                               width,
                                                                               height,
                                                                               img,
                                                                               stride,
                                                                               it,
                                                                               displayGamma,
                                                                               displayNits,
                                                                               colorSpace);
    } else if (linearizePolicy == LinearizePolicy::LinearFromPQ) {
        return readInterleavedWithPolicy<Arch, LinearizePolicy::LinearFromPQ>(applyOOTF,
                                                                              hasAlpha,
                                                                              width,
                                                                              height,
                                                                              img,
                                                                              stride,
                                                                              it,
                                                                              displayGamma,
                                                                              displayNits,
                                                                              colorSpace);
    } else if (linearizePolicy == LinearizePolicy::LinearFromSMPTE428) {
        return readInterleavedWithPolicy<Arch, LinearizePolicy::LinearFromSMPTE428>(applyOOTF,
                                                                                    hasAlpha,
                                                                                    width,
                                                                                    height,
                                                                                    img,
                                                                                    stride,
                                                                                    it,
                                                                                    displayGamma,
                                                                                    displayNits,
                                                                                    colorSpace);
    } else {
        return readInterleavedWithPolicy<Arch, LinearizePolicy::KeepTheSame>(applyOOTF,
                                                                             hasAlpha,
                                                                             width,
                                                                             height,
                                                                             img,
                                                                             stride,
                                                                             it,
                                                                             displayGamma,
                                                                             displayNits,
                                                                             colorSpace);
    }
}

template void
readLayerImpl::create<xsimd::current_arch>(LinearizePolicy linearizePolicy,
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
} // namespace SDR

#endif // XSIMD_UNIVERSAL_BUILD_PASS
