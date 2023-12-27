/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2020-2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_qmic_simple_convertor.h"

#include <array>
#include <cstddef>
#include <map>
#include <memory>
#include <vector>

#include <kis_debug.h>
#include <kis_random_accessor_ng.h>

#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>
#include <KoCompositeOpRegistry.h>

#define SCALE_TO_FLOAT(v) KoColorSpaceMaths<_channel_type_, float>::scaleToA(v)
#define SCALE_FROM_FLOAT(v)                                                    \
    KoColorSpaceMaths<float, _channel_type_>::scaleToA(v)

using KoColorTransformationSP = std::shared_ptr<KoColorTransformation>;

template<typename _channel_type_, typename traits>
class KisColorToFloatConvertor : public KoColorTransformation
{
    using RGBTrait = traits;
    using RGBPixel = typename RGBTrait::Pixel;

public:
    KisColorToFloatConvertor(float gmicUnitValue = 255.0f)
        : m_gmicUnitValue(gmicUnitValue)
    {
    }

    float m_gmicUnitValue;

    void
    transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        const float gmicUnitValue2KritaUnitValue =
            m_gmicUnitValue / KoColorSpaceMathsTraits<float>::unitValue;

        const auto *srcPixel = reinterpret_cast<const RGBPixel *>(src);
        auto *dstPixel = reinterpret_cast<KoRgbF32Traits::Pixel *>(dst);

        while (nPixels > 0) {
            dstPixel->red =
                SCALE_TO_FLOAT(srcPixel->red) * gmicUnitValue2KritaUnitValue;
            dstPixel->green =
                SCALE_TO_FLOAT(srcPixel->green) * gmicUnitValue2KritaUnitValue;
            dstPixel->blue =
                SCALE_TO_FLOAT(srcPixel->blue) * gmicUnitValue2KritaUnitValue;
            dstPixel->alpha =
                SCALE_TO_FLOAT(srcPixel->alpha) * gmicUnitValue2KritaUnitValue;

            --nPixels;
            ++srcPixel;
            ++dstPixel;
        }
    }
};

template<typename _channel_type_, typename traits>
class KisColorFromFloat : public KoColorTransformation
{
    using RGBTrait = traits;
    using RGBPixel = typename RGBTrait::Pixel;

public:
    KisColorFromFloat(float gmicUnitValue = 255.0f)
        : m_gmicUnitValue(gmicUnitValue)
    {
    }

    void
    transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        const auto *srcPixel =
            reinterpret_cast<const KoRgbF32Traits::Pixel *>(src);
        auto *dstPixel = reinterpret_cast<RGBPixel *>(dst);

        const float gmicUnitValue2KritaUnitValue =
            KoColorSpaceMathsTraits<float>::unitValue / m_gmicUnitValue;

        while (nPixels > 0) {
            dstPixel->red =
                SCALE_FROM_FLOAT(srcPixel->red * gmicUnitValue2KritaUnitValue);
            dstPixel->green = SCALE_FROM_FLOAT(srcPixel->green
                                               * gmicUnitValue2KritaUnitValue);
            dstPixel->blue =
                SCALE_FROM_FLOAT(srcPixel->blue * gmicUnitValue2KritaUnitValue);
            dstPixel->alpha = SCALE_FROM_FLOAT(srcPixel->alpha
                                               * gmicUnitValue2KritaUnitValue);

            --nPixels;
            ++srcPixel;
            ++dstPixel;
        }
    }

private:
    float m_gmicUnitValue;
};

template<typename _channel_type_, typename traits>
class KisColorFromGrayScaleFloat : public KoColorTransformation
{
    using RGBTrait = traits;
    using RGBPixel = typename RGBTrait::Pixel;

public:
    KisColorFromGrayScaleFloat(float gmicUnitValue = 255.0f)
        : m_gmicUnitValue(gmicUnitValue)
    {
    }

    void
    transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        const auto *srcPixel =
            reinterpret_cast<const KoRgbF32Traits::Pixel *>(src);
        auto *dstPixel = reinterpret_cast<RGBPixel *>(dst);

        const float gmicUnitValue2KritaUnitValue =
            KoColorSpaceMathsTraits<float>::unitValue / m_gmicUnitValue;
        // warning: green and blue channels on input contain random data!!! see
        // that we copy only one channel when gmic image has grayscale
        // colorspace
        while (nPixels > 0) {
            dstPixel->red = dstPixel->green = dstPixel->blue =
                SCALE_FROM_FLOAT(srcPixel->red * gmicUnitValue2KritaUnitValue);
            dstPixel->alpha = SCALE_FROM_FLOAT(srcPixel->alpha
                                               * gmicUnitValue2KritaUnitValue);

            --nPixels;
            ++srcPixel;
            ++dstPixel;
        }
    }

private:
    float m_gmicUnitValue;
};

template<typename _channel_type_, typename traits>
class KisColorFromGrayScaleAlphaFloat : public KoColorTransformation
{
    using RGBTrait = traits;
    using RGBPixel = typename RGBTrait::Pixel;

public:
    KisColorFromGrayScaleAlphaFloat(float gmicUnitValue = 255.0f)
        : m_gmicUnitValue(gmicUnitValue)
    {
    }

    void
    transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        const auto *srcPixel =
            reinterpret_cast<const KoRgbF32Traits::Pixel *>(src);
        auto *dstPixel = reinterpret_cast<RGBPixel *>(dst);

        const float gmicUnitValue2KritaUnitValue =
            KoColorSpaceMathsTraits<float>::unitValue / m_gmicUnitValue;
        // warning: green and blue channels on input contain random data!!! see
        // that we copy only one channel when gmic image has grayscale
        // colorspace
        while (nPixels > 0) {
            dstPixel->red = dstPixel->green = dstPixel->blue =
                SCALE_FROM_FLOAT(srcPixel->red * gmicUnitValue2KritaUnitValue);
            dstPixel->alpha = SCALE_FROM_FLOAT(srcPixel->green
                                               * gmicUnitValue2KritaUnitValue);

            --nPixels;
            ++srcPixel;
            ++dstPixel;
        }
    }

private:
    float m_gmicUnitValue;
};

const std::map<QString, QString> blendingModeMap = {
    {"add", COMPOSITE_ADD},
    //   {"alpha", ""}, // XXX
    {"and", COMPOSITE_AND},
    //   {"average", ""}, // XXX
    {"blue", COMPOSITE_COPY_BLUE},
    {"burn", COMPOSITE_BURN},
    {"darken", COMPOSITE_DARKEN},
    {"difference", COMPOSITE_DIFF},
    {"divide", COMPOSITE_DIVIDE},
    {"dodge", COMPOSITE_DODGE},
    //   {"edges", ""}, // XXX
    {"exclusion", COMPOSITE_EXCLUSION},
    {"freeze", COMPOSITE_FREEZE},
    {"grainextract", COMPOSITE_GRAIN_EXTRACT},
    {"grainmerge", COMPOSITE_GRAIN_MERGE},
    {"green", COMPOSITE_COPY_GREEN},
    {"hardlight", COMPOSITE_HARD_LIGHT},
    {"hardmix", COMPOSITE_HARD_MIX},
    {"hue", COMPOSITE_HUE},
    {"interpolation", COMPOSITE_INTERPOLATION},
    {"lighten", COMPOSITE_LIGHTEN},
    {"lightness", COMPOSITE_LIGHTNESS},
    {"linearburn", COMPOSITE_LINEAR_BURN},
    {"linearlight", COMPOSITE_LINEAR_LIGHT},
    {"luminance", COMPOSITE_LUMINIZE},
    {"multiply", COMPOSITE_MULT},
    {"negation", COMPOSITE_NEGATION},
    {"or", COMPOSITE_OR},
    {"overlay", COMPOSITE_OVERLAY},
    {"pinlight", COMPOSITE_PIN_LIGHT},
    {"red", COMPOSITE_COPY_RED},
    {"reflect", COMPOSITE_REFLECT},
    {"saturation", COMPOSITE_SATURATION},
    //   {"seamless", ""},       // XXX
    //   {"seamless_mixed", ""}, // XXX
    {"screen", COMPOSITE_SCREEN},
    //   {"shapeareamax", ""},                    // XXX
    //   {"shapeareamax0", ""},                   // XXX
    //   {"shapeareamin", ""},                    // XXX
    //   {"shapeareamin0", ""},                   // XXX
    //   {"shapeaverage", ""},                    // XXX
    //   {"shapeaverage0", ""},                   // XXX
    //   {"shapemedian", ""},                     // XXX
    //   {"shapemedian0", ""},                    // XXX
    //   {"shapemin", ""},                        // XXX
    //   {"shapemin0", ""},                       // XXX
    //   {"shapemax", ""},                        // XXX
    //   {"shapemax0", ""},                       // XXX
    //   {"softburn", ""},                        // XXX
    //   {"softdodge", ""},                       // XXX
    {"softlight", COMPOSITE_SOFT_LIGHT_SVG}, // XXX Is this correct?
    //   {"stamp", ""}, // XXX
    {"subtract", COMPOSITE_SUBTRACT},
    {"value", COMPOSITE_VALUE},
    {"vividlight", COMPOSITE_VIVID_LIGHT},
    {"xor", COMPOSITE_XOR}};

static KoColorTransformation *
createTransformationFromGmic(const KoColorSpace *colorSpace,
                             int gmicSpectrum,
                             float gmicUnitValue)
{
    KoColorTransformation *colorTransformation = nullptr;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        dbgKrita << "Unsupported color space for fast pixel transformation to "
                    "gmic pixel format"
                 << colorSpace->id();
        return nullptr;
    }

    if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        if (gmicSpectrum == 3 || gmicSpectrum == 4) {
            colorTransformation =
                new KisColorFromFloat<float, KoRgbTraits<float>>(gmicUnitValue);
        } else if (gmicSpectrum == 1) {
            colorTransformation =
                new KisColorFromGrayScaleFloat<float, KoRgbTraits<float>>(
                    gmicUnitValue);
        } else if (gmicSpectrum == 2) {
            colorTransformation =
                new KisColorFromGrayScaleAlphaFloat<float, KoRgbTraits<float>>(
                    gmicUnitValue);
        }
    }
#ifdef HAVE_OPENEXR
    else if (colorSpace->colorDepthId() == Float16BitsColorDepthID) {
        if (gmicSpectrum == 3 || gmicSpectrum == 4) {
            colorTransformation =
                new KisColorFromFloat<half, KoRgbTraits<half>>(gmicUnitValue);
        } else if (gmicSpectrum == 1) {
            colorTransformation =
                new KisColorFromGrayScaleFloat<half, KoRgbTraits<half>>(
                    gmicUnitValue);
        } else if (gmicSpectrum == 2) {
            colorTransformation =
                new KisColorFromGrayScaleAlphaFloat<half, KoRgbTraits<half>>(
                    gmicUnitValue);
        }
    }
#endif
    else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        if (gmicSpectrum == 3 || gmicSpectrum == 4) {
            colorTransformation =
                new KisColorFromFloat<quint16, KoBgrTraits<quint16>>(
                    gmicUnitValue);
        } else if (gmicSpectrum == 1) {
            colorTransformation =
                new KisColorFromGrayScaleFloat<quint16, KoBgrTraits<quint16>>(
                    gmicUnitValue);
        } else if (gmicSpectrum == 2) {
            colorTransformation =
                new KisColorFromGrayScaleAlphaFloat<quint16,
                                                    KoBgrTraits<quint16>>(
                    gmicUnitValue);
        }
    } else if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        if (gmicSpectrum == 3 || gmicSpectrum == 4) {
            colorTransformation =
                new KisColorFromFloat<quint8, KoBgrTraits<quint8>>(
                    gmicUnitValue);
        } else if (gmicSpectrum == 1) {
            colorTransformation =
                new KisColorFromGrayScaleFloat<quint8, KoBgrTraits<quint8>>(
                    gmicUnitValue);
        } else if (gmicSpectrum == 2) {
            colorTransformation =
                new KisColorFromGrayScaleAlphaFloat<quint8,
                                                    KoBgrTraits<quint8>>(
                    gmicUnitValue);
        }
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id()
                 << " for fast pixel transformation to gmic pixel format";
        return nullptr;
    }

    return colorTransformation;
}

static KoColorTransformation *
createTransformation(const KoColorSpace *colorSpace)
{
    KoColorTransformation *colorTransformation = nullptr;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        dbgKrita << "Unsupported color space for fast pixel transformation to "
                    "gmic pixel format"
                 << colorSpace->id();
        return nullptr;
    }

    if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        colorTransformation =
            new KisColorToFloatConvertor<float, KoRgbTraits<float>>();
    }
#ifdef HAVE_OPENEXR
    else if (colorSpace->colorDepthId() == Float16BitsColorDepthID) {
        colorTransformation =
            new KisColorToFloatConvertor<half, KoRgbTraits<half>>();
    }
#endif
    else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        colorTransformation =
            new KisColorToFloatConvertor<quint16, KoBgrTraits<quint16>>();
    } else if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        colorTransformation =
            new KisColorToFloatConvertor<quint8, KoBgrTraits<quint8>>();
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id()
                 << " for fast pixel transformation to gmic pixel format";
        return nullptr;
    }
    return colorTransformation;
}

void KisQmicSimpleConvertor::convertFromGmicFast(const KisQMicImage &gmicImage,
                                                 KisPaintDeviceSP dst,
                                                 float gmicUnitValue)
{
    dbgPlugins << "convertFromGmicFast";
    const KoColorSpace *dstColorSpace = dst->colorSpace();
    const KoColorTransformationSP gmicToDstPixelFormat(
        createTransformationFromGmic(dstColorSpace,
                                     gmicImage.m_spectrum,
                                     gmicUnitValue));
    if (!gmicToDstPixelFormat) {
        dbgPlugins << "Fall-back to slow color conversion";
        convertFromGmicImage(gmicImage, dst, gmicUnitValue);
        return;
    }

    dst->clear();
    dst->moveTo(0, 0);

    const qint32 x = 0;
    const qint32 y = 0;
    const auto width = static_cast<size_t>(gmicImage.m_width);
    const auto height = static_cast<size_t>(gmicImage.m_height);

    const auto *rgbaFloat32bitcolorSpace =
        KoColorSpaceRegistry::instance()->colorSpace(
            RGBAColorModelID.id(),
            Float32BitsColorDepthID.id(),
            KoColorSpaceRegistry::instance()->rgb8()->profile());
    // this function always convert to rgba or rgb with various color depth
    const quint32 dstNumChannels = rgbaFloat32bitcolorSpace->channelCount();

    // number of channels that we will copy
    const int numChannels = gmicImage.m_spectrum;

    // gmic image has 4, 3, 2, 1 channel
    std::vector<float *> planes(dstNumChannels);
    const size_t channelOffset = width * height;
    for (int channelIndex = 0; channelIndex < gmicImage.m_spectrum;
         channelIndex++) {
        planes[channelIndex] = gmicImage.m_data + channelOffset * channelIndex;
    }

    for (int channelIndex = gmicImage.m_spectrum;
         channelIndex < (int)dstNumChannels;
         channelIndex++) {
        planes[channelIndex] = 0; // turn off
    }

    size_t dataY = 0;
    int imageY = y;
    size_t rowsRemaining = height;

    const auto floatPixelSize = rgbaFloat32bitcolorSpace->pixelSize();

    KisRandomAccessorSP it = dst->createRandomAccessorNG();
    const auto tileWidth =
        static_cast<size_t>(it->numContiguousColumns(dst->x()));
    const auto tileHeight =
        static_cast<size_t>(it->numContiguousRows(dst->y()));
    Q_ASSERT(tileWidth == 64);
    Q_ASSERT(tileHeight == 64);
    std::vector<quint8> convertedTile(
        static_cast<size_t>(rgbaFloat32bitcolorSpace->pixelSize()) * tileWidth
        * tileHeight);

    // grayscale and rgb case does not have alpha, so let's fill 4th channel of
    // rgba tile with opacity opaque
    if (gmicImage.m_spectrum == 1 || gmicImage.m_spectrum == 3) {
        const auto nPixels = tileWidth * tileHeight;
        auto *srcPixel =
            reinterpret_cast<KoRgbF32Traits::Pixel *>(convertedTile.data());
        for (size_t pixelIndex = 0; pixelIndex < nPixels;
             pixelIndex++, srcPixel++) {
            srcPixel->alpha = gmicUnitValue;
        }
    }

    while (rowsRemaining > 0) {
        size_t dataX = 0;
        qint32 imageX = x;
        size_t columnsRemaining = width;
        const auto numContiguousImageRows =
            static_cast<size_t>(it->numContiguousRows(imageY));

        size_t rowsToWork = qMin(numContiguousImageRows, rowsRemaining);

        while (columnsRemaining > 0) {
            const auto numContiguousImageColumns =
                static_cast<size_t>(it->numContiguousColumns(imageX));
            auto columnsToWork =
                qMin(numContiguousImageColumns, columnsRemaining);

            const auto dataIdx = dataX + dataY * width;
            const auto tileRowStride =
                (tileWidth - columnsToWork) * floatPixelSize;

            auto *tileItStart = convertedTile.data();
            // copy gmic channels to float tile
            const auto channelSize = sizeof(float);
            for (int i = 0; i < numChannels; i++) {
                float *planeIt = planes[i] + dataIdx;
                const auto dataStride = width - columnsToWork;
                quint8 *tileIt = tileItStart;

                for (size_t row = 0; row < rowsToWork; row++) {
                    for (size_t col = 0; col < columnsToWork; col++) {
                        memcpy(tileIt, planeIt, channelSize);
                        tileIt += floatPixelSize;
                        planeIt += 1;
                    }

                    tileIt += tileRowStride;
                    planeIt += dataStride;
                }
                tileItStart += channelSize;
            }

            it->moveTo(imageX, imageY);
            quint8 *dstTileItStart = it->rawData();
            tileItStart =
                convertedTile.data(); // back to the start of the converted tile
            // copy float tile to dst colorspace based on input colorspace (rgb
            // or grayscale)
            for (size_t row = 0; row < rowsToWork; row++) {
                gmicToDstPixelFormat->transform(
                    tileItStart,
                    dstTileItStart,
                    static_cast<int>(columnsToWork));
                dstTileItStart += dstColorSpace->pixelSize() * tileWidth;
                tileItStart += floatPixelSize * tileWidth;
            }

            imageX += static_cast<int>(columnsToWork);
            dataX += columnsToWork;
            columnsRemaining -= columnsToWork;
        }

        imageY += static_cast<int>(rowsToWork);
        dataY += rowsToWork;
        rowsRemaining -= rowsToWork;
    }

    dst->crop(x, y, static_cast<qint32>(width), static_cast<qint32>(height));
}

void KisQmicSimpleConvertor::convertToGmicImageFast(KisPaintDeviceSP dev,
                                                    KisQMicImage &gmicImage,
                                                    QRect rc)
{
    KoColorTransformationSP pixelToGmicPixelFormat(
        createTransformation(dev->colorSpace()));
    if (!pixelToGmicPixelFormat) {
        dbgPlugins << "Fall-back to slow color conversion method";
        convertToGmicImage(dev, gmicImage, rc);
        return;
    }

    if (rc.isEmpty()) {
        dbgPlugins
            << "Image rectangle is empty! Using supplied gmic layer dimension";
        rc = QRect(0,
                   0,
                   static_cast<int>(gmicImage.m_width),
                   static_cast<int>(gmicImage.m_height));
    }

    const auto x = rc.x();
    const auto y = rc.y();
    const size_t width = rc.width() < 0 ? 0 : static_cast<size_t>(rc.width());
    const size_t height =
        rc.height() < 0 ? 0 : static_cast<size_t>(rc.height());

    const qint32 numChannels = 4;

    const size_t greenOffset =
        static_cast<size_t>(gmicImage.m_width) * gmicImage.m_height;
    const size_t blueOffset = greenOffset * 2;
    const size_t alphaOffset = greenOffset * 3;

    const std::array<float *, 4> planes = {gmicImage.m_data,
                                           gmicImage.m_data + greenOffset,
                                           gmicImage.m_data + blueOffset,
                                           gmicImage.m_data + alphaOffset};

    KisRandomConstAccessorSP it = dev->createRandomConstAccessorNG();
    const auto tileWidth = it->numContiguousColumns(dev->x());
    const auto tileHeight = it->numContiguousRows(dev->y());

    Q_ASSERT(tileWidth == 64);
    Q_ASSERT(tileHeight == 64);

    const KoColorSpace *rgbaFloat32bitcolorSpace =
        KoColorSpaceRegistry::instance()->colorSpace(
            RGBAColorModelID.id(),
            Float32BitsColorDepthID.id(),
            KoColorSpaceRegistry::instance()->rgb8()->profile());
    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);
    const auto dstPixelSize = rgbaFloat32bitcolorSpace->pixelSize();
    const auto srcPixelSize = dev->pixelSize();

    std::vector<quint8> dstTile(dstPixelSize * static_cast<size_t>(tileWidth)
                                * static_cast<size_t>(tileHeight));

    size_t dataY = 0;
    int imageY = y;
    int imageX = x;
    it->moveTo(imageX, imageY);
    size_t rowsRemaining = height;

    while (rowsRemaining > 0) {
        size_t dataX = 0;
        imageX = x;
        size_t columnsRemaining = width;
        const auto numContiguousImageRows = it->numContiguousRows(imageY);

        const auto rowsToWork =
            qMin(numContiguousImageRows, static_cast<qint32>(rowsRemaining));
        const auto convertedTileY = tileHeight - rowsToWork;
        Q_ASSERT(convertedTileY >= 0);

        while (columnsRemaining > 0) {
            const auto numContiguousImageColumns =
                static_cast<size_t>(it->numContiguousColumns(imageX));
            const auto columnsToWork =
                qMin(numContiguousImageColumns, columnsRemaining);
            const auto convertedTileX =
                tileWidth - static_cast<qint32>(columnsToWork);
            Q_ASSERT(convertedTileX >= 0);

            const auto dataIdx = dataX + dataY * width;
            const auto dstTileIndex =
                convertedTileX + convertedTileY * tileWidth;
            const auto tileRowStride =
                (static_cast<size_t>(tileWidth) - columnsToWork) * dstPixelSize;
            const auto srcTileRowStride =
                (static_cast<size_t>(tileWidth) - columnsToWork) * srcPixelSize;

            it->moveTo(imageX, imageY);
            quint8 *tileItStart = dstTile.data() + dstTileIndex * dstPixelSize;

            // transform tile row by row
            auto *dstTileIt = tileItStart;
            const auto *srcTileIt = it->rawDataConst();

            auto row = rowsToWork;
            while (row > 0) {
                pixelToGmicPixelFormat->transform(
                    srcTileIt,
                    dstTileIt,
                    static_cast<qint32>(columnsToWork));
                srcTileIt += columnsToWork * srcPixelSize;
                srcTileIt += srcTileRowStride;

                dstTileIt += columnsToWork * dstPixelSize;
                dstTileIt += tileRowStride;

                row--;
            }

            // here we want to copy floats to dstTile, so tileItStart has to
            // point to float buffer
            const auto channelSize = sizeof(float);
            for (size_t i = 0; i < numChannels; i++) {
                float *planeIt = planes.at(i) + dataIdx;
                const auto dataStride = (width - columnsToWork);
                quint8 *tileIt = tileItStart;

                for (int row = 0; row < rowsToWork; row++) {
                    for (size_t col = 0; col < columnsToWork; col++) {
                        memcpy(planeIt, tileIt, channelSize);
                        tileIt += dstPixelSize;
                        planeIt += 1;
                    }

                    tileIt += tileRowStride;
                    planeIt += dataStride;
                }
                // skip channel in tile: red, green, blue, alpha
                tileItStart += channelSize;
            }

            imageX += static_cast<int>(columnsToWork);
            dataX += columnsToWork;
            columnsRemaining -= columnsToWork;
        }

        imageY += static_cast<int>(rowsToWork);
        dataY += rowsToWork;
        rowsRemaining -= rowsToWork;
    }
}

// gmic assumes float rgba in 0.0 - 255.0
void KisQmicSimpleConvertor::convertToGmicImage(KisPaintDeviceSP dev,
                                                KisQMicImage &gmicImage,
                                                QRect rc)
{
    Q_ASSERT(!dev.isNull());
    Q_ASSERT(gmicImage.m_spectrum == 4); // rgba

    if (rc.isEmpty()) {
        rc = QRect(0,
                   0,
                   static_cast<int>(gmicImage.m_width),
                   static_cast<int>(gmicImage.m_height));
    }

    const KoColorSpace *rgbaFloat32bitcolorSpace =
        KoColorSpaceRegistry::instance()->colorSpace(
            RGBAColorModelID.id(),
            Float32BitsColorDepthID.id(),
            KoColorSpaceRegistry::instance()->rgb8()->profile());
    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);

    const KoColorTransformationSP pixelToGmicPixelFormat(
        createTransformation(rgbaFloat32bitcolorSpace));

    const size_t greenOffset =
        static_cast<size_t>(gmicImage.m_width) * gmicImage.m_height;
    const size_t blueOffset = greenOffset * 2;
    const size_t alphaOffset = greenOffset * 3;

    const auto renderingIntent =
        KoColorConversionTransformation::internalRenderingIntent();
    const auto conversionFlags =
        KoColorConversionTransformation::internalConversionFlags();

    const KoColorSpace *colorSpace = dev->colorSpace();
    KisRandomConstAccessorSP it = dev->createRandomConstAccessorNG();

    const size_t optimalBufferSize =
        64; // most common numContiguousColumns, tile size?
    std::vector<quint8> floatRGBApixelStorage(
        rgbaFloat32bitcolorSpace->pixelSize() * optimalBufferSize);
    quint8 *floatRGBApixel = floatRGBApixelStorage.data();

    const auto pixelSize = rgbaFloat32bitcolorSpace->pixelSize();
    for (int y = 0; y < rc.height(); y++) {
        int x = 0;
        while (x < rc.width()) {
            it->moveTo(rc.x() + x, rc.y() + y);
            auto numContiguousColumns =
                qMin(static_cast<size_t>(it->numContiguousColumns(rc.x() + x)),
                     optimalBufferSize);
            numContiguousColumns =
                qMin(numContiguousColumns, static_cast<size_t>(rc.width() - x));

            colorSpace->convertPixelsTo(
                it->rawDataConst(),
                floatRGBApixel,
                rgbaFloat32bitcolorSpace,
                static_cast<quint32>(numContiguousColumns),
                renderingIntent,
                conversionFlags);
            pixelToGmicPixelFormat->transform(
                floatRGBApixel,
                floatRGBApixel,
                static_cast<qint32>(numContiguousColumns));

            auto pos = static_cast<size_t>(y) * gmicImage.m_width
                + static_cast<size_t>(x);
            for (size_t bx = 0; bx < numContiguousColumns; bx++) {
                memcpy(gmicImage.m_data + pos,
                       floatRGBApixel + bx * pixelSize,
                       4);
                memcpy(gmicImage.m_data + pos + greenOffset,
                       floatRGBApixel + bx * pixelSize + 4,
                       4);
                memcpy(gmicImage.m_data + pos + blueOffset,
                       floatRGBApixel + bx * pixelSize + 8,
                       4);
                memcpy(gmicImage.m_data + pos + alphaOffset,
                       floatRGBApixel + bx * pixelSize + 12,
                       4);
                pos++;
            }

            x += static_cast<int>(numContiguousColumns);
        }
    }
}

void KisQmicSimpleConvertor::convertFromGmicImage(const KisQMicImage &gmicImage,
                                                  KisPaintDeviceSP dst,
                                                  float gmicMaxChannelValue)
{
    dbgPlugins << "convertFromGmicSlow";
    Q_ASSERT(!dst.isNull());
    const KoColorSpace *rgbaFloat32bitcolorSpace =
        KoColorSpaceRegistry::instance()->colorSpace(
            RGBAColorModelID.id(),
            Float32BitsColorDepthID.id(),
            KoColorSpaceRegistry::instance()->rgb8()->profile());
    const KoColorSpace *dstColorSpace = dst->colorSpace();

    KisPaintDeviceSP dev = dst;
    const size_t greenOffset =
        static_cast<size_t>(gmicImage.m_width) * gmicImage.m_height;
    const size_t blueOffset = greenOffset * 2;
    const size_t alphaOffset = greenOffset * 3;
    QRect rc(0,
             0,
             static_cast<int>(gmicImage.m_width),
             static_cast<int>(gmicImage.m_height));

    KisRandomAccessorSP it = dev->createRandomAccessorNG();
    float r = 0;
    float g = 0;
    float b = 0;
    float a = 1.0;

    const size_t optimalBufferSize =
        64; // most common numContiguousColumns, tile size?
    std::vector<quint8> floatRGBApixelStorage(
        rgbaFloat32bitcolorSpace->pixelSize() * optimalBufferSize);
    quint8 *floatRGBApixel = floatRGBApixelStorage.data();
    const auto pixelSize = rgbaFloat32bitcolorSpace->pixelSize();

    const auto renderingIntent =
        KoColorConversionTransformation::internalRenderingIntent();
    const auto conversionFlags =
        KoColorConversionTransformation::internalConversionFlags();

    // Krita needs rgba in 0.0...1.0
    const float multiplied =
        KoColorSpaceMathsTraits<float>::unitValue / gmicMaxChannelValue;

    switch (gmicImage.m_spectrum) {
    case 1: {
        // convert grayscale to rgba
        for (int y = 0; y < rc.height(); y++) {
            int x = 0;
            while (x < rc.width()) {
                it->moveTo(x, y);
                auto numContiguousColumns =
                    qMin(static_cast<size_t>(it->numContiguousColumns(x)),
                         optimalBufferSize);
                numContiguousColumns =
                    qMin(numContiguousColumns,
                         static_cast<size_t>(rc.width() - x));

                size_t pos = static_cast<size_t>(y) * gmicImage.m_width
                    + static_cast<size_t>(x);
                for (size_t bx = 0; bx < numContiguousColumns; bx++) {
                    r = g = b = gmicImage.m_data[pos] * multiplied;
                    a = KoColorSpaceMathsTraits<float>::unitValue;

                    memcpy(floatRGBApixel + bx * pixelSize, &r, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 4, &g, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 8, &b, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 12, &a, 4);
                    pos++;
                }
                rgbaFloat32bitcolorSpace->convertPixelsTo(
                    floatRGBApixel,
                    it->rawData(),
                    dstColorSpace,
                    static_cast<quint32>(numContiguousColumns),
                    renderingIntent,
                    conversionFlags);
                x += static_cast<int>(numContiguousColumns);
            }
        }
        break;
    }
    case 2: {
        // convert grayscale alpha to rgba
        for (int y = 0; y < rc.height(); y++) {
            int x = 0;
            while (x < rc.width()) {
                it->moveTo(x, y);
                auto numContiguousColumns =
                    qMin(static_cast<size_t>(it->numContiguousColumns(x)),
                         optimalBufferSize);
                numContiguousColumns =
                    qMin(numContiguousColumns,
                         static_cast<size_t>(rc.width() - x));

                size_t pos = static_cast<size_t>(y) * gmicImage.m_width
                    + static_cast<size_t>(x);
                for (size_t bx = 0; bx < numContiguousColumns; bx++) {
                    r = g = b = gmicImage.m_data[pos] * multiplied;
                    a = gmicImage.m_data[pos + greenOffset] * multiplied;

                    memcpy(floatRGBApixel + bx * pixelSize, &r, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 4, &g, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 8, &b, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 12, &a, 4);
                    pos++;
                }
                rgbaFloat32bitcolorSpace->convertPixelsTo(
                    floatRGBApixel,
                    it->rawData(),
                    dstColorSpace,
                    static_cast<quint32>(numContiguousColumns),
                    renderingIntent,
                    conversionFlags);
                x += static_cast<int>(numContiguousColumns);
            }
        }
        break;
    }
    case 3: {
        // convert rgb -> rgba
        for (int y = 0; y < rc.height(); y++) {
            int x = 0;
            while (x < rc.width()) {
                it->moveTo(x, y);
                auto numContiguousColumns =
                    qMin(static_cast<size_t>(it->numContiguousColumns(x)),
                         optimalBufferSize);
                numContiguousColumns =
                    qMin(numContiguousColumns,
                         static_cast<size_t>(rc.width() - x));

                size_t pos = static_cast<size_t>(y) * gmicImage.m_width
                    + static_cast<size_t>(x);
                for (size_t bx = 0; bx < numContiguousColumns; bx++) {
                    r = gmicImage.m_data[pos] * multiplied;
                    g = gmicImage.m_data[pos + greenOffset] * multiplied;
                    b = gmicImage.m_data[pos + blueOffset] * multiplied;
                    a = gmicMaxChannelValue * multiplied;

                    memcpy(floatRGBApixel + bx * pixelSize, &r, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 4, &g, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 8, &b, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 12, &a, 4);
                    pos++;
                }
                rgbaFloat32bitcolorSpace->convertPixelsTo(
                    floatRGBApixel,
                    it->rawData(),
                    dstColorSpace,
                    static_cast<quint32>(numContiguousColumns),
                    renderingIntent,
                    conversionFlags);
                x += static_cast<int>(numContiguousColumns);
            }
        }
        break;
    }
    case 4: {
        for (int y = 0; y < rc.height(); y++) {
            int x = 0;
            while (x < rc.width()) {
                it->moveTo(x, y);
                auto numContiguousColumns =
                    qMin(static_cast<size_t>(it->numContiguousColumns(x)),
                         optimalBufferSize);
                numContiguousColumns =
                    qMin(numContiguousColumns,
                         static_cast<size_t>(rc.width() - x));

                size_t pos = static_cast<size_t>(y) * gmicImage.m_width
                    + static_cast<size_t>(x);
                for (size_t bx = 0; bx < numContiguousColumns; bx++) {
                    r = gmicImage.m_data[pos] * multiplied;
                    g = gmicImage.m_data[pos + greenOffset] * multiplied;
                    b = gmicImage.m_data[pos + blueOffset] * multiplied;
                    a = gmicImage.m_data[pos + alphaOffset] * multiplied;

                    memcpy(floatRGBApixel + bx * pixelSize, &r, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 4, &g, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 8, &b, 4);
                    memcpy(floatRGBApixel + bx * pixelSize + 12, &a, 4);
                    pos++;
                }
                rgbaFloat32bitcolorSpace->convertPixelsTo(
                    floatRGBApixel,
                    it->rawData(),
                    dstColorSpace,
                    static_cast<quint32>(numContiguousColumns),
                    renderingIntent,
                    conversionFlags);
                x += static_cast<int>(numContiguousColumns);
            }
        }
        break;
    }

    default: {
        dbgPlugins << "Unsupported gmic output format : " << gmicImage.m_width
                   << gmicImage.m_height << gmicImage.m_spectrum;
    }
    }
}

QImage KisQmicSimpleConvertor::convertToQImage(const KisQMicImage &gmicImage,
                                               float gmicActualMaxChannelValue)
{
    QImage image = QImage(static_cast<int>(gmicImage.m_width),
                          static_cast<int>(gmicImage.m_height),
                          QImage::Format_ARGB32);

    dbgPlugins << image.format() << "first pixel:" << gmicImage.m_data[0]
               << gmicImage.m_width << gmicImage.m_height
               << gmicImage.m_spectrum;

    const size_t greenOffset =
        static_cast<size_t>(gmicImage.m_width) * gmicImage.m_height;
    const size_t blueOffset = greenOffset * 2;

    // always put 255 to qimage
    const float multiplied = 255.0f / gmicActualMaxChannelValue;

    for (int y = 0; y < gmicImage.m_height; y++) {
        QRgb *pixel =
            reinterpret_cast<QRgb *>(image.scanLine(static_cast<int>(y)));
        for (int x = 0; x < gmicImage.m_width; x++) {
            const auto pos = y * gmicImage.m_width + x;
            const float r = gmicImage.m_data[pos] * multiplied;
            const float g = gmicImage.m_data[pos + greenOffset] * multiplied;
            const float b = gmicImage.m_data[pos + blueOffset] * multiplied;
            pixel[x] = qRgb(int(r), int(g), int(b));
        }
    }
    return image;
}

void KisQmicSimpleConvertor::convertFromQImage(const QImage &image,
                                               KisQMicImage &gmicImage,
                                               float gmicUnitValue)
{
    const auto greenOffset =
        static_cast<size_t>(gmicImage.m_width) * gmicImage.m_height;
    const size_t blueOffset = greenOffset * 2;
    const size_t alphaOffset = greenOffset * 3;

    // QImage has 0..255
    const float qimageUnitValue = 255.0f;
    const float multiplied = gmicUnitValue / qimageUnitValue;

    Q_ASSERT(image.width() == int(gmicImage.m_width));
    Q_ASSERT(image.height() == int(gmicImage.m_height));
    Q_ASSERT(image.format() == QImage::Format_ARGB32);

    switch (gmicImage.m_spectrum) {
    case 1: {
        for (int y = 0; y < image.height(); y++) {
            const QRgb *pixel =
                reinterpret_cast<const QRgb *>(image.scanLine(y));
            for (int x = 0; x < image.width(); x++) {
                const auto pos = static_cast<size_t>(y) * gmicImage.m_width
                    + static_cast<size_t>(x);
                gmicImage.m_data[pos] =
                    static_cast<float>(qGray(pixel[x])) * multiplied;
            }
        }
        break;
    }
    case 2: {
        for (int y = 0; y < image.height(); y++) {
            const QRgb *pixel =
                reinterpret_cast<const QRgb *>(image.scanLine(y));
            for (int x = 0; x < image.width(); x++) {
                const auto pos = static_cast<size_t>(y) * gmicImage.m_width
                    + static_cast<size_t>(x);
                gmicImage.m_data[pos] =
                    static_cast<float>(qGray(pixel[x])) * multiplied;
                gmicImage.m_data[pos + greenOffset] =
                    static_cast<float>(qAlpha(pixel[x])) * multiplied;
            }
        }
        break;
    }
    case 3: {
        for (int y = 0; y < image.height(); y++) {
            const QRgb *pixel =
                reinterpret_cast<const QRgb *>(image.scanLine(y));
            for (int x = 0; x < image.width(); x++) {
                const auto pos = static_cast<size_t>(y) * gmicImage.m_width
                    + static_cast<size_t>(x);
                gmicImage.m_data[pos] =
                    static_cast<float>(qRed(pixel[x])) * multiplied;
                gmicImage.m_data[pos + greenOffset] =
                    static_cast<float>(qGreen(pixel[x])) * multiplied;
                gmicImage.m_data[pos + blueOffset] =
                    static_cast<float>(qBlue(pixel[x])) * multiplied;
            }
        }
        break;
    }
    case 4: {
        for (int y = 0; y < image.height(); y++) {
            const QRgb *pixel =
                reinterpret_cast<const QRgb *>(image.scanLine(y));
            for (int x = 0; x < image.width(); x++) {
                const auto pos = static_cast<size_t>(y) * gmicImage.m_width
                    + static_cast<size_t>(x);
                gmicImage.m_data[pos] =
                    static_cast<float>(qRed(pixel[x])) * multiplied;
                gmicImage.m_data[pos + greenOffset] =
                    static_cast<float>(qGreen(pixel[x])) * multiplied;
                gmicImage.m_data[pos + blueOffset] =
                    static_cast<float>(qBlue(pixel[x])) * multiplied;
                gmicImage.m_data[pos + alphaOffset] =
                    static_cast<float>(qAlpha(pixel[x])) * multiplied;
            }
        }
        break;
    }
    default: {
        Q_ASSERT(false);
        dbgKrita << "Unexpected gmic image format";
        break;
    }
    }
}

std::map<QString, QString> reverseMap()
{
    std::map<QString, QString> result{};
    for (const auto &pair : blendingModeMap) {
        result.emplace(pair.second, pair.first);
    }
    return result;
}

QString KisQmicSimpleConvertor::blendingModeToString(QString blendMode)
{
    static auto reverseModeMap = reverseMap();
    if (reverseModeMap.find(blendMode) != reverseModeMap.end())
        return reverseModeMap.at(blendMode);
    return "alpha";
}

QString KisQmicSimpleConvertor::stringToBlendingMode(QString blendMode)
{
    if (blendingModeMap.find(blendMode) != blendingModeMap.end())
        return blendingModeMap.at(blendMode);
    return COMPOSITE_OVER;
}
