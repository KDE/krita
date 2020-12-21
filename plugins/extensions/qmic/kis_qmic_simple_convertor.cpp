/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <map>

#include <kis_qmic_simple_convertor.h>

#include <kis_debug.h>
#include <kis_random_accessor_ng.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceTraits.h>
#include <KoCompositeOpRegistry.h>

#define SCALE_TO_FLOAT( v ) KoColorSpaceMaths< _channel_type_, float>::scaleToA( v )
#define SCALE_FROM_FLOAT( v  ) KoColorSpaceMaths< float, _channel_type_>::scaleToA( v )


template<typename _channel_type_, typename traits>
class KisColorToFloatConvertor : public KoColorTransformation
{
    typedef traits RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
    KisColorToFloatConvertor(float gmicUnitValue = 255.0f)
        : m_gmicUnitValue(gmicUnitValue)
    {}

    float m_gmicUnitValue;

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        float gmicUnitValue2KritaUnitValue = m_gmicUnitValue / KoColorSpaceMathsTraits<float>::unitValue;

        const RGBPixel* srcPixel = reinterpret_cast<const RGBPixel*>(src);
        KoRgbF32Traits::Pixel* dstPixel = reinterpret_cast<KoRgbF32Traits::Pixel*>(dst);

        while(nPixels > 0)
        {
            dstPixel->red = SCALE_TO_FLOAT(srcPixel->red) * gmicUnitValue2KritaUnitValue;
            dstPixel->green = SCALE_TO_FLOAT(srcPixel->green) * gmicUnitValue2KritaUnitValue;
            dstPixel->blue = SCALE_TO_FLOAT(srcPixel->blue) * gmicUnitValue2KritaUnitValue;
            dstPixel->alpha = SCALE_TO_FLOAT(srcPixel->alpha) * gmicUnitValue2KritaUnitValue;

            --nPixels;
            ++srcPixel;
            ++dstPixel;
        }
    }
};


template<typename _channel_type_, typename traits>
class KisColorFromFloat : public KoColorTransformation
{
    typedef traits RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
    KisColorFromFloat(float gmicUnitValue = 255.0f)
        : m_gmicUnitValue(gmicUnitValue)
    {
    }

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        const KoRgbF32Traits::Pixel* srcPixel = reinterpret_cast<const KoRgbF32Traits::Pixel*>(src);
        RGBPixel* dstPixel = reinterpret_cast<RGBPixel*>(dst);

        float gmicUnitValue2KritaUnitValue = KoColorSpaceMathsTraits<float>::unitValue / m_gmicUnitValue;

        while(nPixels > 0)
        {
            dstPixel->red = SCALE_FROM_FLOAT(srcPixel->red * gmicUnitValue2KritaUnitValue);
            dstPixel->green = SCALE_FROM_FLOAT(srcPixel->green * gmicUnitValue2KritaUnitValue);
            dstPixel->blue = SCALE_FROM_FLOAT(srcPixel->blue * gmicUnitValue2KritaUnitValue);
            dstPixel->alpha = SCALE_FROM_FLOAT(srcPixel->alpha * gmicUnitValue2KritaUnitValue);

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
    typedef traits RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
    KisColorFromGrayScaleFloat(float gmicUnitValue = 255.0f):m_gmicUnitValue(gmicUnitValue){}

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        const KoRgbF32Traits::Pixel* srcPixel = reinterpret_cast<const KoRgbF32Traits::Pixel*>(src);
        RGBPixel* dstPixel = reinterpret_cast<RGBPixel*>(dst);

        float gmicUnitValue2KritaUnitValue = KoColorSpaceMathsTraits<float>::unitValue / m_gmicUnitValue;
        // warning: green and blue channels on input contain random data!!! see that we copy only one channel
        // when gmic image has grayscale colorspace
        while(nPixels > 0)
        {
            dstPixel->red = dstPixel->green = dstPixel->blue = SCALE_FROM_FLOAT(srcPixel->red * gmicUnitValue2KritaUnitValue);
            dstPixel->alpha = SCALE_FROM_FLOAT(srcPixel->alpha * gmicUnitValue2KritaUnitValue);

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
    typedef traits RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
    KisColorFromGrayScaleAlphaFloat(float gmicUnitValue = 255.0f):m_gmicUnitValue(gmicUnitValue){}

    void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const override
    {
        const KoRgbF32Traits::Pixel* srcPixel = reinterpret_cast<const KoRgbF32Traits::Pixel*>(src);
        RGBPixel* dstPixel = reinterpret_cast<RGBPixel*>(dst);

        float gmicUnitValue2KritaUnitValue = KoColorSpaceMathsTraits<float>::unitValue / m_gmicUnitValue;
        // warning: green and blue channels on input contain random data!!! see that we copy only one channel
        // when gmic image has grayscale colorspace
        while(nPixels > 0)
        {
            dstPixel->red = dstPixel->green = dstPixel->blue = SCALE_FROM_FLOAT(srcPixel->red * gmicUnitValue2KritaUnitValue);
            dstPixel->alpha = SCALE_FROM_FLOAT(srcPixel->green * gmicUnitValue2KritaUnitValue);

            --nPixels;
            ++srcPixel;
            ++dstPixel;
        }
    }

private:
    float m_gmicUnitValue;
};

const std::map<QString, QString> blendingModeMap = {{"add", COMPOSITE_ADD},
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

static KoColorTransformation* createTransformationFromGmic(const KoColorSpace* colorSpace, quint32 gmicSpectrum,float gmicUnitValue)
{
    KoColorTransformation * colorTransformation = 0;
    if (colorSpace->colorModelId() != RGBAColorModelID)
    {
        dbgKrita << "Unsupported color space for fast pixel transformation to gmic pixel format" << colorSpace->id();
        return 0;
    }

    if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        if (gmicSpectrum == 3 || gmicSpectrum == 4) {
            colorTransformation = new KisColorFromFloat< float, KoRgbTraits < float > >(gmicUnitValue);
        }
        else if (gmicSpectrum == 1) {
            colorTransformation = new KisColorFromGrayScaleFloat<float, KoRgbTraits < float > >(gmicUnitValue);
        }
        else if (gmicSpectrum == 2) {
            colorTransformation = new KisColorFromGrayScaleAlphaFloat<float, KoRgbTraits < float > >(gmicUnitValue);
        }
    }
#ifdef HAVE_OPENEXR
    else if (colorSpace->colorDepthId() == Float16BitsColorDepthID) {
        if (gmicSpectrum == 3 || gmicSpectrum == 4) {
            colorTransformation = new KisColorFromFloat< half, KoRgbTraits < half > >(gmicUnitValue);
        }
        else if (gmicSpectrum == 1) {
            colorTransformation = new KisColorFromGrayScaleFloat< half, KoRgbTraits < half > >(gmicUnitValue);
        }
        else if (gmicSpectrum == 2) {
            colorTransformation = new KisColorFromGrayScaleAlphaFloat< half, KoRgbTraits < half > >(gmicUnitValue);
        }
    }
#endif
    else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        if (gmicSpectrum == 3 || gmicSpectrum == 4) {
            colorTransformation = new KisColorFromFloat< quint16, KoBgrTraits < quint16 > >(gmicUnitValue);
        }
        else if (gmicSpectrum == 1) {
            colorTransformation = new KisColorFromGrayScaleFloat< quint16, KoBgrTraits < quint16 > >(gmicUnitValue);
        }
        else if (gmicSpectrum == 2) {
            colorTransformation = new KisColorFromGrayScaleAlphaFloat< quint16, KoBgrTraits < quint16 > >(gmicUnitValue);
        }
    }
    else if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        if (gmicSpectrum == 3 || gmicSpectrum == 4) {
            colorTransformation = new KisColorFromFloat< quint8, KoBgrTraits < quint8 > >(gmicUnitValue);
        }
        else if (gmicSpectrum == 1) {
            colorTransformation = new KisColorFromGrayScaleFloat< quint8, KoBgrTraits < quint8 > >(gmicUnitValue);
        }
        else if (gmicSpectrum == 2) {
            colorTransformation = new KisColorFromGrayScaleAlphaFloat< quint8, KoBgrTraits < quint8 > >(gmicUnitValue);
        }
    }
    else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " for fast pixel transformation to gmic pixel format";
        return 0;
    }

    return colorTransformation;
}


static KoColorTransformation* createTransformation(const KoColorSpace* colorSpace)
{
    KoColorTransformation * colorTransformation = 0;
    if (colorSpace->colorModelId() != RGBAColorModelID)
    {
        dbgKrita << "Unsupported color space for fast pixel transformation to gmic pixel format" << colorSpace->id();
        return 0;
    }

    if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        colorTransformation = new KisColorToFloatConvertor< float, KoRgbTraits < float > >();
    }
#ifdef HAVE_OPENEXR
    else if (colorSpace->colorDepthId() == Float16BitsColorDepthID) {
        colorTransformation = new KisColorToFloatConvertor< half, KoRgbTraits < half > >();
    }
#endif
    else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        colorTransformation = new KisColorToFloatConvertor< quint16, KoBgrTraits < quint16 > >();
    } else if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        colorTransformation = new KisColorToFloatConvertor< quint8, KoBgrTraits < quint8 > >();
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " for fast pixel transformation to gmic pixel format";
        return 0;
    }
    return colorTransformation;
}


void KisQmicSimpleConvertor::convertFromGmicFast(gmic_image<float>& gmicImage, KisPaintDeviceSP dst, float gmicUnitValue)
{
    dbgPlugins << "convertFromGmicFast";
    const KoColorSpace * dstColorSpace = dst->colorSpace();
    KoColorTransformation * gmicToDstPixelFormat = createTransformationFromGmic(dstColorSpace, gmicImage._spectrum, gmicUnitValue);
    if (gmicToDstPixelFormat == 0) {
            dbgPlugins << "Fall-back to slow color conversion";
            convertFromGmicImage(gmicImage, dst, gmicUnitValue);
            return;
    }

    qint32 x = 0;
    qint32 y = 0;
    qint32 width = gmicImage._width;
    qint32 height = gmicImage._height;

    width  = width < 0  ? 0 : width;
    height = height < 0 ? 0 : height;

    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());
    // this function always convert to rgba or rgb with various color depth
    quint32 dstNumChannels = rgbaFloat32bitcolorSpace->channelCount();

    // number of channels that we will copy
    quint32 numChannels = gmicImage._spectrum;

    // gmic image has 4, 3, 2, 1 channel
    QVector<float *> planes(dstNumChannels);
    int channelOffset = gmicImage._width * gmicImage._height;
    for (unsigned int channelIndex = 0; channelIndex < gmicImage._spectrum; channelIndex++) {
        planes[channelIndex] = gmicImage._data + channelOffset * channelIndex;
    }

    for (unsigned int channelIndex = gmicImage._spectrum; channelIndex < dstNumChannels; channelIndex++) {
        planes[channelIndex] = 0; //turn off
    }

    qint32 dataY = 0;
    qint32 imageY = y;
    qint32 rowsRemaining = height;

    const qint32 floatPixelSize = rgbaFloat32bitcolorSpace->pixelSize();

    KisRandomAccessorSP it = dst->createRandomAccessorNG();
    int tileWidth = it->numContiguousColumns(dst->x());
    int tileHeight = it->numContiguousRows(dst->y());
    Q_ASSERT(tileWidth == 64);
    Q_ASSERT(tileHeight == 64);
    quint8 *convertedTile = new quint8[rgbaFloat32bitcolorSpace->pixelSize() * tileWidth * tileHeight];

    // grayscale and rgb case does not have alpha, so let's fill 4th channel of rgba tile with opacity opaque
    if (gmicImage._spectrum == 1 || gmicImage._spectrum == 3) {
        quint32 nPixels = tileWidth * tileHeight;
        quint32 pixelIndex = 0;
        KoRgbF32Traits::Pixel* srcPixel = reinterpret_cast<KoRgbF32Traits::Pixel*>(convertedTile);
        while (pixelIndex < nPixels) {
            srcPixel->alpha = gmicUnitValue;
            ++srcPixel;
            ++pixelIndex;
        }
    }

    while (rowsRemaining > 0) {

        qint32 dataX = 0;
        qint32 imageX = x;
        qint32 columnsRemaining = width;
        qint32 numContiguousImageRows = it->numContiguousRows(imageY);

        qint32 rowsToWork = qMin(numContiguousImageRows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousImageColumns = it->numContiguousColumns(imageX);
            qint32 columnsToWork = qMin(numContiguousImageColumns, columnsRemaining);

            const qint32 dataIdx = dataX + dataY * width;
            const qint32 tileRowStride = (tileWidth - columnsToWork) * floatPixelSize;

            quint8 *tileItStart = convertedTile;
            // copy gmic channels to float tile
            qint32 channelSize = sizeof(float);
            for(quint32 i=0; i<numChannels; i++)
            {
                float * planeIt = planes[i] + dataIdx;
                qint32 dataStride = (width - columnsToWork);
                quint8* tileIt = tileItStart;

                for (qint32 row = 0; row < rowsToWork; row++) {
                    for (int col = 0; col < columnsToWork; col++) {
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
            tileItStart = convertedTile;  // back to the start of the converted tile
            // copy float tile to dst colorspace based on input colorspace (rgb or grayscale)
            for (qint32 row = 0; row < rowsToWork; row++)
            {
                gmicToDstPixelFormat->transform(tileItStart, dstTileItStart, columnsToWork);
                dstTileItStart += dstColorSpace->pixelSize() * tileWidth;
                tileItStart += floatPixelSize * tileWidth;
            }

            imageX += columnsToWork;
            dataX += columnsToWork;
            columnsRemaining -= columnsToWork;
        }


        imageY += rowsToWork;
        dataY += rowsToWork;
        rowsRemaining -= rowsToWork;
    }

    delete [] convertedTile;
    delete gmicToDstPixelFormat;

}



void KisQmicSimpleConvertor::convertToGmicImageFast(KisPaintDeviceSP dev, gmic_image<float> *gmicImage, QRect rc)
{

    KoColorTransformation * pixelToGmicPixelFormat = createTransformation(dev->colorSpace());
    if (pixelToGmicPixelFormat == 0)
    {
        dbgPlugins << "Fall-back to slow color conversion method";
        convertToGmicImage(dev, gmicImage, rc);
        return;
    }

    if (rc.isEmpty())
    {
        dbgPlugins << "Image rectangle is empty! Using supplied gmic layer dimension";
        rc = QRect(0, 0, gmicImage->_width, gmicImage->_height);
    }

    qint32 x = rc.x();
    qint32 y = rc.y();
    qint32 width = rc.width();
    qint32 height = rc.height();

    width  = width < 0  ? 0 : width;
    height = height < 0 ? 0 : height;

    const qint32 numChannels = 4;

    int greenOffset = gmicImage->_width * gmicImage->_height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;

    QVector<float *> planes;
    planes.append(gmicImage->_data);
    planes.append(gmicImage->_data + greenOffset);
    planes.append(gmicImage->_data + blueOffset);
    planes.append(gmicImage->_data + alphaOffset);

    KisRandomConstAccessorSP it = dev->createRandomConstAccessorNG();
    int tileWidth = it->numContiguousColumns(dev->x());
    int tileHeight = it->numContiguousRows(dev->y());

    Q_ASSERT(tileWidth == 64);
    Q_ASSERT(tileHeight == 64);

    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());
    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);
    const qint32 dstPixelSize = rgbaFloat32bitcolorSpace->pixelSize();
    const qint32 srcPixelSize = dev->pixelSize();

    quint8 * dstTile = new quint8[dstPixelSize * tileWidth * tileHeight];

    qint32 dataY = 0;
    qint32 imageX = x;
    qint32 imageY = y;
    it->moveTo(imageX, imageY);
    qint32 rowsRemaining = height;

    while (rowsRemaining > 0) {

        qint32 dataX = 0;
        imageX = x;
        qint32 columnsRemaining = width;
        qint32 numContiguousImageRows = it->numContiguousRows(imageY);

        qint32 rowsToWork = qMin(numContiguousImageRows, rowsRemaining);
        qint32 convertedTileY = tileHeight - rowsToWork;
        Q_ASSERT(convertedTileY >= 0);

        while (columnsRemaining > 0) {

            qint32 numContiguousImageColumns = it->numContiguousColumns(imageX);
            qint32 columnsToWork = qMin(numContiguousImageColumns, columnsRemaining);
            qint32 convertedTileX = tileWidth - columnsToWork;
            Q_ASSERT(convertedTileX >= 0);

            const qint32 dataIdx = dataX + dataY * width;
            const qint32 dstTileIndex = convertedTileX + convertedTileY * tileWidth;
            const qint32 tileRowStride = (tileWidth - columnsToWork) * dstPixelSize;
            const qint32 srcTileRowStride = (tileWidth - columnsToWork) * srcPixelSize;

            it->moveTo(imageX, imageY);
            quint8 *tileItStart = dstTile + dstTileIndex * dstPixelSize;

            // transform tile row by row
            quint8 *dstTileIt = tileItStart;
            quint8 *srcTileIt = const_cast<quint8*>(it->rawDataConst());

            qint32 row = rowsToWork;
            while (row > 0) {
                pixelToGmicPixelFormat->transform(srcTileIt, dstTileIt , columnsToWork);
                srcTileIt += columnsToWork * srcPixelSize;
                srcTileIt += srcTileRowStride;

                dstTileIt += columnsToWork * dstPixelSize;
                dstTileIt += tileRowStride;

                row--;
            }

            // here we want to copy floats to dstTile, so tileItStart has to point to float buffer
            qint32 channelSize = sizeof(float);
            for(qint32 i = 0; i< numChannels; i++) {

                float * planeIt = planes[i] + dataIdx;
                qint32 dataStride = (width - columnsToWork);
                quint8* tileIt = tileItStart;

                for (qint32 row = 0; row < rowsToWork; row++) {
                    for (int col = 0; col < columnsToWork; col++) {
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

            imageX += columnsToWork;
            dataX += columnsToWork;
            columnsRemaining -= columnsToWork;
        }

        imageY += rowsToWork;
        dataY += rowsToWork;
        rowsRemaining -= rowsToWork;
    }

    delete [] dstTile;
    delete pixelToGmicPixelFormat;


}

// gmic assumes float rgba in 0.0 - 255.0
void KisQmicSimpleConvertor::convertToGmicImage(KisPaintDeviceSP dev, gmic_image<float> *gmicImage, QRect rc)
{
    Q_ASSERT(!dev.isNull());
    Q_ASSERT(gmicImage->_spectrum == 4); // rgba

    if (rc.isEmpty()) {
        rc = QRect(0, 0, gmicImage->_width, gmicImage->_height);
    }

    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());
    Q_CHECK_PTR(rgbaFloat32bitcolorSpace);

    KoColorTransformation *pixelToGmicPixelFormat = createTransformation(rgbaFloat32bitcolorSpace);

    int greenOffset = gmicImage->_width * gmicImage->_height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;

    KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent();
    KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags();

    const KoColorSpace * colorSpace = dev->colorSpace();
    KisRandomConstAccessorSP it = dev->createRandomConstAccessorNG();

    int optimalBufferSize = 64; // most common numContiguousColumns, tile size?
    QScopedArrayPointer<quint8> floatRGBApixelStorage(new quint8[rgbaFloat32bitcolorSpace->pixelSize() * optimalBufferSize]);
    quint8 *floatRGBApixel = floatRGBApixelStorage.data();

    quint32 pixelSize = rgbaFloat32bitcolorSpace->pixelSize();
    int pos = 0;
    for (int y = 0; y < rc.height(); y++)
    {
        int x = 0;
        while (x < rc.width())
        {
            it->moveTo(rc.x() + x, rc.y() + y);
            qint32 numContiguousColumns = qMin(it->numContiguousColumns(rc.x() + x), optimalBufferSize);
            numContiguousColumns = qMin(numContiguousColumns, rc.width() - x);

            colorSpace->convertPixelsTo(it->rawDataConst(), floatRGBApixel, rgbaFloat32bitcolorSpace, numContiguousColumns, renderingIntent, conversionFlags);
            pixelToGmicPixelFormat->transform(floatRGBApixel, floatRGBApixel, numContiguousColumns);

            pos = y * gmicImage->_width + x;
            for (qint32 bx = 0; bx < numContiguousColumns; bx++)
            {
                memcpy(gmicImage->_data + pos                  ,floatRGBApixel + bx * pixelSize   , 4);
                memcpy(gmicImage->_data + pos + greenOffset    ,floatRGBApixel + bx * pixelSize + 4, 4);
                memcpy(gmicImage->_data + pos + blueOffset     ,floatRGBApixel + bx * pixelSize + 8, 4);
                memcpy(gmicImage->_data + pos + alphaOffset    ,floatRGBApixel + bx * pixelSize + 12, 4);
                pos++;
            }

            x += numContiguousColumns;
        }
    }
    delete pixelToGmicPixelFormat;
}

void KisQmicSimpleConvertor::convertFromGmicImage(gmic_image<float>& gmicImage, KisPaintDeviceSP dst, float gmicMaxChannelValue)
{
    dbgPlugins << "convertFromGmicSlow";
    Q_ASSERT(!dst.isNull());
    const KoColorSpace *rgbaFloat32bitcolorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                Float32BitsColorDepthID.id(),
                                                                                                KoColorSpaceRegistry::instance()->rgb8()->profile());
    const KoColorSpace *dstColorSpace = dst->colorSpace();

    KisPaintDeviceSP dev = dst;
    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    QRect rc(0,0,gmicImage._width, gmicImage._height);

    KisRandomAccessorSP it = dev->createRandomAccessorNG();
    int pos;
    float r,g,b,a;

    int optimalBufferSize = 64; // most common numContiguousColumns, tile size?
    QScopedArrayPointer<quint8> floatRGBApixelStorage(new quint8[rgbaFloat32bitcolorSpace->pixelSize() * optimalBufferSize]);
    quint8 * floatRGBApixel = floatRGBApixelStorage.data();
    quint32 pixelSize = rgbaFloat32bitcolorSpace->pixelSize();

    KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent();
    KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags();

    // Krita needs rgba in 0.0...1.0
    float multiplied = KoColorSpaceMathsTraits<float>::unitValue / gmicMaxChannelValue;

    switch (gmicImage._spectrum)
    {
        case 1:
        {
            // convert grayscale to rgba
            for (int y = 0; y < rc.height(); y++)
            {
                int x = 0;
                while (x < rc.width())
                {
                    it->moveTo(x, y);
                    qint32 numContiguousColumns = qMin(it->numContiguousColumns(x), optimalBufferSize);
                    numContiguousColumns = qMin(numContiguousColumns, rc.width() - x);

                    pos = y * gmicImage._width + x;
                    for (qint32 bx = 0; bx < numContiguousColumns; bx++)
                    {
                            r = g = b = gmicImage._data[pos] * multiplied;
                            a = KoColorSpaceMathsTraits<float>::unitValue;

                            memcpy(floatRGBApixel + bx * pixelSize,      &r,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 4,  &g,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 8,  &b,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 12, &a,4);
                            pos++;
                    }
                    rgbaFloat32bitcolorSpace->convertPixelsTo(floatRGBApixel, it->rawData(), dstColorSpace, numContiguousColumns,renderingIntent, conversionFlags);
                    x += numContiguousColumns;
                }
            }
            break;
        }
        case 2:
        {
            // convert grayscale alpha to rgba
            for (int y = 0; y < rc.height(); y++)
            {
                int x = 0;
                while (x < rc.width())
                {
                    it->moveTo(x, y);
                    qint32 numContiguousColumns = qMin(it->numContiguousColumns(x), optimalBufferSize);
                    numContiguousColumns = qMin(numContiguousColumns, rc.width() - x);

                    pos = y * gmicImage._width + x;
                    for (qint32 bx = 0; bx < numContiguousColumns; bx++)
                    {
                            r = g = b = gmicImage._data[pos] * multiplied;
                            a = gmicImage._data[pos + greenOffset] * multiplied;

                            memcpy(floatRGBApixel + bx * pixelSize,      &r,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 4,  &g,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 8,  &b,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 12, &a,4);
                            pos++;
                    }
                    rgbaFloat32bitcolorSpace->convertPixelsTo(floatRGBApixel, it->rawData(), dstColorSpace, numContiguousColumns,renderingIntent, conversionFlags);
                    x += numContiguousColumns;
                }
            }
            break;
        }
        case 3:
        {
            // convert rgb -> rgba
            for (int y = 0; y < rc.height(); y++)
            {
                int x = 0;
                while (x < rc.width())
                {
                    it->moveTo(x, y);
                    qint32 numContiguousColumns = qMin(it->numContiguousColumns(x), optimalBufferSize);
                    numContiguousColumns = qMin(numContiguousColumns, rc.width() - x);

                    pos = y * gmicImage._width + x;
                    for (qint32 bx = 0; bx < numContiguousColumns; bx++)
                    {
                            r = gmicImage._data[pos] * multiplied;
                            g = gmicImage._data[pos + greenOffset] * multiplied;
                            b = gmicImage._data[pos + blueOffset ] * multiplied;
                            a = gmicMaxChannelValue * multiplied;

                            memcpy(floatRGBApixel + bx * pixelSize,      &r,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 4,  &g,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 8,  &b,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 12, &a,4);
                            pos++;
                    }
                    rgbaFloat32bitcolorSpace->convertPixelsTo(floatRGBApixel, it->rawData(), dstColorSpace, numContiguousColumns,renderingIntent, conversionFlags);
                    x += numContiguousColumns;
                }
            }
            break;
        }
        case 4:
        {
            for (int y = 0; y < rc.height(); y++)
            {
                int x = 0;
                while (x < rc.width())
                {
                    it->moveTo(x, y);
                    qint32 numContiguousColumns = qMin(it->numContiguousColumns(x), optimalBufferSize);
                    numContiguousColumns = qMin(numContiguousColumns, rc.width() - x);

                    pos = y * gmicImage._width + x;
                    for (qint32 bx = 0; bx < numContiguousColumns; bx++)
                    {
                            r = gmicImage._data[pos] * multiplied;
                            g = gmicImage._data[pos + greenOffset] * multiplied;
                            b = gmicImage._data[pos + blueOffset ] * multiplied;
                            a = gmicImage._data[pos + alphaOffset] * multiplied;

                            memcpy(floatRGBApixel + bx * pixelSize,      &r,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 4,  &g,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 8,  &b,4);
                            memcpy(floatRGBApixel + bx * pixelSize + 12, &a,4);
                            pos++;
                    }
                    rgbaFloat32bitcolorSpace->convertPixelsTo(floatRGBApixel, it->rawData(), dstColorSpace, numContiguousColumns,renderingIntent, conversionFlags);
                    x += numContiguousColumns;
                }
            }
            break;
        }

        default:
        {
            dbgPlugins << "Unsupported gmic output format : " <<  gmicImage._width << gmicImage._height << gmicImage._depth << gmicImage._spectrum;
        }
    }
}


QImage KisQmicSimpleConvertor::convertToQImage(gmic_image<float>& gmicImage, float gmicActualMaxChannelValue)
{

    QImage image = QImage(gmicImage._width, gmicImage._height, QImage::Format_ARGB32);

    dbgPlugins << image.format() <<"first pixel:"<< gmicImage._data[0] << gmicImage._width << gmicImage._height << gmicImage._spectrum;

    int greenOffset = gmicImage._width * gmicImage._height;
    int blueOffset = greenOffset * 2;
    int pos = 0;

    // always put 255 to qimage
    float multiplied = 255.0f / gmicActualMaxChannelValue;

    for (unsigned int y = 0; y < gmicImage._height; y++)
    {
        QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (unsigned int x = 0; x < gmicImage._width; x++)
        {
            pos = y * gmicImage._width + x;
            float r = gmicImage._data[pos] * multiplied;
            float g = gmicImage._data[pos + greenOffset]  * multiplied;
            float b = gmicImage._data[pos + blueOffset]  * multiplied;
            pixel[x] = qRgb(int(r),int(g), int(b));
        }
    }
    return image;
}


void KisQmicSimpleConvertor::convertFromQImage(const QImage &image, gmic_image<float> *gmicImage, float gmicUnitValue)
{
    int greenOffset = gmicImage->_width * gmicImage->_height;
    int blueOffset = greenOffset * 2;
    int alphaOffset = greenOffset * 3;
    int pos = 0;

    // QImage has 0..255
    float qimageUnitValue = 255.0f;
    float multiplied = gmicUnitValue / qimageUnitValue;


    Q_ASSERT(image.width() == int(gmicImage->_width));
    Q_ASSERT(image.height() == int(gmicImage->_height));
    Q_ASSERT(image.format() == QImage::Format_ARGB32);

    switch (gmicImage->_spectrum)
    {
        case 1:
        {
            for (int y = 0; y < image.height(); y++)
            {
                const QRgb *pixel = reinterpret_cast<const QRgb *>(image.scanLine(y));
                for (int x = 0; x < image.width(); x++)
                {
                    pos = y * gmicImage->_width + x;
                    gmicImage->_data[pos]                = qGray(pixel[x]) * multiplied;
                }
            }
            break;
        }
        case 2:
        {
            for (int y = 0; y < image.height(); y++)
            {
                const QRgb *pixel = reinterpret_cast<const QRgb *>(image.scanLine(y));
                for (int x = 0; x < image.width(); x++)
                {
                    pos = y * gmicImage->_width + x;
                    gmicImage->_data[pos]                = qGray(pixel[x]) * multiplied;
                    gmicImage->_data[pos + greenOffset]  = qAlpha(pixel[x]) * multiplied;
                }
            }
            break;
        }
        case 3:
        {
            for (int y = 0; y < image.height(); y++)
            {
                const QRgb *pixel = reinterpret_cast<const QRgb *>(image.scanLine(y));
                for (int x = 0; x < image.width(); x++)
                {
                    pos = y * gmicImage->_width + x;
                    gmicImage->_data[pos]                = qRed(pixel[x]) * multiplied;
                    gmicImage->_data[pos + greenOffset]  = qGreen(pixel[x]) * multiplied;
                    gmicImage->_data[pos + blueOffset]   = qBlue(pixel[x]) * multiplied;
                }
            }
            break;
        }
        case 4:
        {
            for (int y = 0; y < image.height(); y++)
            {
                const QRgb *pixel = reinterpret_cast<const QRgb *>(image.scanLine(y));
                for (int x = 0; x < image.width(); x++)
                {
                    pos = y * gmicImage->_width + x;
                    gmicImage->_data[pos]                = qRed(pixel[x]) * multiplied;
                    gmicImage->_data[pos + greenOffset]  = qGreen(pixel[x]) * multiplied;
                    gmicImage->_data[pos + blueOffset]   = qBlue(pixel[x]) * multiplied;
                    gmicImage->_data[pos + alphaOffset]  = qAlpha(pixel[x]) * multiplied;
                }
            }
            break;
        }
        default:
        {
            Q_ASSERT(false);
            dbgKrita << "Unexpected gmic image format";
            break;
        }
    }
}

std::map<QString, QString> reverseMap()
{
    std::map<QString, QString> result {};
    for (auto &pair : blendingModeMap) {
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
