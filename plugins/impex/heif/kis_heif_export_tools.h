/**
 *  SPDX-FileCopyrightText: 2020-2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HEIF_EXPORT_TOOLS_H
#define KIS_HEIF_EXPORT_TOOLS_H

#include <cstdint>

#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoColorTransferFunctions.h>
#include <kis_iterator_ng.h>

namespace Gray
{
template<int endValue0, int endValue1, int luma>
inline void
applyValue(const quint8 *data, uint8_t *ptrG, int strideG, int x, int y)
{
    if (luma == 8) {
        ptrG[y * strideG + x] = KoGrayU8Traits::gray(data);
    } else {
        uint16_t v = qBound<uint16_t>(
            0,
            static_cast<uint16_t>(float(KoGrayU16Traits::gray(data))
                                  * multiplier16bit * max12bit),
            max12bit);
        ptrG[(x * 2) + y * strideG + endValue0] = static_cast<uint8_t>(v >> 8);
        ptrG[(x * 2) + y * strideG + endValue1] =
            static_cast<uint8_t>(v & 0xFF);
    }
}

template<int endValue0, int endValue1, int luma, bool hasAlpha>
inline void
applyAlpha(const quint8 *data, uint8_t *ptrA, const int strideA, int x, int y)
{
    if (hasAlpha) {
        if (luma == 8) {
            ptrA[y * strideA + x] = KoGrayU8Traits::opacityU8(data);
        } else {
            uint16_t vA = qBound<uint16_t>(
                0,
                static_cast<uint16_t>(KoGrayU16Traits::opacityF(data)
                                      * max12bit),
                max12bit);
            ptrA[(x * 2) + y * strideA + endValue0] = (uint8_t)(vA >> 8);
            ptrA[(x * 2) + y * strideA + endValue1] = (uint8_t)(vA & 0xFF);
        }
    }
}

template<int endValue0, int endValue1, int luma, bool hasAlpha>
inline void writeLayer(const int width,
                       const int height,
                       uint8_t *ptrG,
                       const int strideG,
                       uint8_t *ptrA,
                       const int strideA,
                       KisHLineConstIteratorSP it)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const uint8_t *data = it->rawDataConst();

            applyValue<endValue0, endValue1, luma>(data, ptrG, strideG, x, y);

            applyAlpha<endValue0, endValue1, luma, hasAlpha>(data,
                                                             ptrA,
                                                             strideA,
                                                             x,
                                                             y);

            it->nextPixel();
        }

        it->nextRow();
    }
}

template<int endValue0, int endValue1, int luma, typename... Args>
inline auto writePlanarWithAlpha(bool hasAlpha, Args &&...args)
{
    if (hasAlpha) {
        return Gray::writeLayer<endValue0, endValue1, luma, true>(
            std::forward<Args>(args)...);
    } else {
        return Gray::writeLayer<endValue0, endValue1, luma, false>(
            std::forward<Args>(args)...);
    }
}

template<int endValue0, int endValue1, typename... Args>
inline auto writePlanarWithLuma(const int luma, Args &&...args)
{
    if (luma == 8) {
        return writePlanarWithAlpha<endValue0, endValue1, 8>(
            std::forward<Args>(args)...);
    } else {
        return writePlanarWithAlpha<endValue0, endValue1, 12>(
            std::forward<Args>(args)...);
    }
}

template<typename... Args>
inline auto writePlanarLayer(QSysInfo::Endian endian, Args &&...args)
{
    if (endian == QSysInfo::LittleEndian) {
        return Gray::writePlanarWithLuma<1, 0>(std::forward<Args>(args)...);
    } else {
        return Gray::writePlanarWithLuma<0, 1>(std::forward<Args>(args)...);
    }
}
} // namespace Gray

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

namespace HDRInt
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
        return HDRInt::writeLayerImpl<endValue0, endValue1, 4>(
            std::forward<Args>(args)...);
    } else {
        return HDRInt::writeLayerImpl<endValue0, endValue1, 3>(
            std::forward<Args>(args)...);
    }
}

template<typename... Args>
inline auto writeInterleavedLayer(QSysInfo::Endian endian, Args &&...args)
{
    if (endian == QSysInfo::LittleEndian) {
        return writeInterleavedWithAlpha<1, 0>(std::forward<Args>(args)...);
    } else {
        return writeInterleavedWithAlpha<0, 1>(std::forward<Args>(args)...);
    }
}
} // namespace HDRInt

namespace HDRFloat
{

template<ConversionPolicy policy>
inline float applyCurveAsNeeded(float value)
{
    if (policy == ConversionPolicy::ApplyPQ) {
        return applySmpte2048Curve(value);
    } else if (policy == ConversionPolicy::ApplyHLG) {
        return applyHLGCurve(value);
    } else if (policy == ConversionPolicy::ApplySMPTE428) {
        return applySMPTE_ST_428Curve(value);
    }
    return value;
}

template<typename CSTrait,
         QSysInfo::Endian endianness,
         int channels,
         bool convertToRec2020,
         bool isLinear,
         ConversionPolicy conversionPolicy,
         bool removeOOTF>
inline void writeFloatLayerImpl(const int width,
                                const int height,
                                uint8_t *ptr,
                                const int stride,
                                KisHLineConstIteratorSP it,
                                float hlgGamma,
                                float hlgNominalPeak,
                                const KoColorSpace *cs)
{
    const int endValue0 = endianness == QSysInfo::LittleEndian ? 1 : 0;
    const int endValue1 = endianness == QSysInfo::LittleEndian ? 0 : 1;
    QVector<float> pixelValues(4);
    QVector<qreal> pixelValuesLinear(4);
    const KoColorProfile *profile = cs->profile();
    const QVector<qreal> lCoef{cs->lumaCoefficients()};
    double *src = pixelValuesLinear.data();
    float *dst = pixelValues.data();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            CSTrait::normalisedChannelsValue(it->rawDataConst(), pixelValues);
            if (!convertToRec2020 && !isLinear) {
                for (int i = 0; i < 4; i++) {
                    src[i] = static_cast<double>(dst[i]);
                }
                profile->linearizeFloatValue(pixelValuesLinear);
                for (int i = 0; i < 4; i++) {
                    dst[i] = static_cast<float>(src[i]);
                }
            }

            if (conversionPolicy == ConversionPolicy::ApplyHLG && removeOOTF) {
                removeHLGOOTF(dst, lCoef.constData(), hlgGamma, hlgNominalPeak);
            }

            for (int ch = 0; ch < channels; ch++) {
                uint16_t v = 0;
                if (ch == CSTrait::alpha_pos) {
                    v = qBound<uint16_t>(
                        0,
                        static_cast<uint16_t>(
                            applyCurveAsNeeded<ConversionPolicy::KeepTheSame>(
                                dst[ch])
                            * max12bit),
                        max12bit);
                } else {
                    v = qBound<uint16_t>(
                        0,
                        static_cast<uint16_t>(
                            applyCurveAsNeeded<conversionPolicy>(dst[ch])
                            * max12bit),
                        max12bit);
                }
                ptr[2 * (x * channels) + y * stride + endValue0 + (ch * 2)] =
                    (uint8_t)(v >> 8);
                ptr[2 * (x * channels) + y * stride + endValue1 + (ch * 2)] =
                    (uint8_t)(v & 0xFF);
            }

            it->nextPixel();
        }

        it->nextRow();
    }
}
template<typename CSTrait,
         QSysInfo::Endian endianness,
         int channels,
         bool convertToRec2020,
         bool isLinear,
         ConversionPolicy linearizePolicy,
         typename... Args>
inline auto writeInterleavedWithPolicy(bool removeOOTF, Args &&...args)
{
    if (removeOOTF) {
        return writeFloatLayerImpl<CSTrait,
                                   endianness,
                                   channels,
                                   convertToRec2020,
                                   isLinear,
                                   linearizePolicy,
                                   true>(std::forward<Args>(args)...);
    } else {
        return writeFloatLayerImpl<CSTrait,
                                   endianness,
                                   channels,
                                   convertToRec2020,
                                   isLinear,
                                   linearizePolicy,
                                   false>(std::forward<Args>(args)...);
    }
}

template<typename CSTrait,
         QSysInfo::Endian endianness,
         int channels,
         bool convertToRec2020,
         bool isLinear,
         typename... Args>
inline auto writeInterleavedWithLinear(ConversionPolicy linearizePolicy,
                                       Args &&...args)
{
    if (linearizePolicy == ConversionPolicy::ApplyHLG) {
        return writeInterleavedWithPolicy<CSTrait,
                                          endianness,
                                          channels,
                                          convertToRec2020,
                                          isLinear,
                                          ConversionPolicy::ApplyHLG>(std::forward<Args>(args)...);
    } else if (linearizePolicy == ConversionPolicy::ApplyPQ) {
        return writeInterleavedWithPolicy<CSTrait,
                                          endianness,
                                          channels,
                                          convertToRec2020,
                                          isLinear,
                                          ConversionPolicy::ApplyPQ>(std::forward<Args>(args)...);
    } else if (linearizePolicy == ConversionPolicy::ApplySMPTE428) {
        return writeInterleavedWithPolicy<CSTrait,
                                          endianness,
                                          channels,
                                          convertToRec2020,
                                          isLinear,
                                          ConversionPolicy::ApplySMPTE428>(std::forward<Args>(args)...);
    } else {
        return writeInterleavedWithPolicy<CSTrait,
                                          endianness,
                                          channels,
                                          convertToRec2020,
                                          isLinear,
                                          ConversionPolicy::KeepTheSame>(std::forward<Args>(args)...);
    }
}

template<typename CSTrait,
         QSysInfo::Endian endianness,
         int channels,
         bool convertToRec2020,
         typename... Args>
inline auto writeInterleavedWithRec2020(bool isLinear, Args &&...args)
{
    if (isLinear) {
        return writeInterleavedWithLinear<CSTrait,
                                          endianness,
                                          channels,
                                          convertToRec2020,
                                          true>(std::forward<Args>(args)...);
    } else {
        return writeInterleavedWithLinear<CSTrait,
                                          endianness,
                                          channels,
                                          convertToRec2020,
                                          false>(std::forward<Args>(args)...);
    }
}

template<typename CSTrait,
         QSysInfo::Endian endianness,
         int channels,
         typename... Args>
inline auto writeInterleavedWithAlpha(bool convertToRec2020, Args &&...args)
{
    if (convertToRec2020) {
        return writeInterleavedWithRec2020<CSTrait, endianness, channels, true>(
            std::forward<Args>(args)...);
    } else {
        return writeInterleavedWithRec2020<CSTrait,
                                           endianness,
                                           channels,
                                           false>(std::forward<Args>(args)...);
    }
}

template<typename CSTrait, QSysInfo::Endian endianness, typename... Args>
inline auto writeInterleavedWithEndian(bool hasAlpha, Args &&...args)
{
    if (hasAlpha) {
        return writeInterleavedWithAlpha<CSTrait, endianness, 4>(
            std::forward<Args>(args)...);
    } else {
        return writeInterleavedWithAlpha<CSTrait, endianness, 3>(
            std::forward<Args>(args)...);
    }
}

template<typename CSTrait, typename... Args>
inline auto writeInterleavedWithDepth(QSysInfo::Endian endian, Args &&...args)
{
    if (endian == QSysInfo::LittleEndian) {
        return writeInterleavedWithEndian<CSTrait, QSysInfo::LittleEndian>(
            std::forward<Args>(args)...);
    } else {
        return writeInterleavedWithEndian<CSTrait, QSysInfo::BigEndian>(
            std::forward<Args>(args)...);
    }
}

template<typename... Args>
inline auto writeInterleavedLayer(const KoID &id, Args &&...args)
{
#ifdef HAVE_OPENEXR
    if (id == Float16BitsColorDepthID) {
        return writeInterleavedWithDepth<KoBgrF16Traits>(
            std::forward<Args>(args)...);
    } else
#else
    Q_UNUSED(id);
#endif
    {
        return writeInterleavedWithDepth<KoBgrF32Traits>(
            std::forward<Args>(args)...);
    }
}
} // namespace HDRFloat

#endif // KIS_HEIF_EXPORT_TOOLS_H
