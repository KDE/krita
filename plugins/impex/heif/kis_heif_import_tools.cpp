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
template<typename Arch,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename std::enable_if_t<!std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
inline void linearize(QVector<float> &pixelValues,
                      const QVector<double> &lCoef,
                      float displayGamma,
                      float displayNits)
{
    using float_v = typename KoColorTransferFunctions<Arch>::float_v;
    if (linearizePolicy == LinearFromPQ) {
        auto v = float_v::load_unaligned(pixelValues.constData());
        KoColorTransferFunctions<Arch>::removeSmpte2048Curve(v);
        v.store_unaligned(pixelValues.data());
    } else if (linearizePolicy == LinearFromHLG) {
        auto v = float_v::load_unaligned(pixelValues.constData());
        KoColorTransferFunctions<Arch>::removeHLGCurve(v);
        v.store_unaligned(pixelValues.data());
    } else if (linearizePolicy == LinearFromSMPTE428) {
        auto v = float_v::load_unaligned(pixelValues.constData());
        KoColorTransferFunctions<Arch>::removeSMPTE_ST_428Curve(v);
        v.store_unaligned(pixelValues.data());
    }

    if (linearizePolicy == KeepTheSame) {
        qSwap(pixelValues[0], pixelValues[2]);
    } else if (linearizePolicy == LinearFromHLG && applyOOTF) {
        applyHLGOOTF(pixelValues, lCoef, displayGamma, displayNits);
    }
}

template<typename Arch,
         LinearizePolicy linearizePolicy,
         bool applyOOTF,
         typename std::enable_if_t<std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
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
    if (policy == LinearFromPQ) {
        return removeSmpte2048Curve(value);
    } else if (policy == LinearFromHLG) {
        return removeHLGCurve(value);
    } else if (policy == LinearFromSMPTE428) {
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

template<typename Arch,
         typename std::enable_if_t<!std::is_same<Arch, xsimd::generic>::value,
                                   int> = 0>
constexpr int bufferSize()
{
    return qMax<int>(4, KoStreamedMath<Arch>::float_v::size);
}

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

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            std::fill(pixelValues.begin(), pixelValues.end(), 1.0f);

            pixelValues[0] =
                value<Arch, luma, linearizePolicy>(imgR, strideR, x, y);
            pixelValues[1] =
                value<Arch, luma, linearizePolicy>(imgG, strideG, x, y);
            pixelValues[2] =
                value<Arch, luma, linearizePolicy>(imgB, strideB, x, y);

            if (hasAlpha) {
                pixelValues[3] =
                    value<Arch, luma, linearizePolicy>(imgA, strideA, x, y);
            }

            linearize<Arch, linearizePolicy, applyOOTF>(pixelValues,
                                                        lCoef,
                                                        displayGamma,
                                                        displayNits);

            colorSpace->fromNormalisedChannelsValue(it->rawData(), pixelValues);

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
    if (linearizePolicy == LinearFromHLG) {
        return readPlanarLayerWithPolicy<Arch, luma, LinearFromHLG>(
            std::forward<Args>(args)...);
    } else if (linearizePolicy == LinearFromPQ) {
        return readPlanarLayerWithPolicy<Arch, luma, LinearFromPQ>(
            std::forward<Args>(args)...);
    } else if (linearizePolicy == LinearFromSMPTE428) {
        return readPlanarLayerWithPolicy<Arch, luma, LinearFromSMPTE428>(
            std::forward<Args>(args)...);
    } else {
        return readPlanarLayerWithPolicy<Arch, luma, KeepTheSame>(
            std::forward<Args>(args)...);
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

#endif // XSIMD_UNIVERSAL_BUILD_PASS
