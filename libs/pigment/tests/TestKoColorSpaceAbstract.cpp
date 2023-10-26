/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestKoColorSpaceAbstract.h"

#include "KoColorSpaceAbstract.h"
#include "KoColorSpaceTraits.h"

#include <cfloat>

#include <simpletest.h>

template <class T>
T mixOpExpectedAlpha(T alpha1, T alpha2, const qint16 *weights)
{
    using compositetype = typename KoColorSpaceMathsTraits<T>::compositetype;

    const int sumOfWeights = 255;
    return safeDivideWithRound<compositetype>(weights[0] * alpha1 + weights[1] * alpha2, sumOfWeights);
}

template <class T>
T mixOpExpectedColor(T color1, T alpha1, T color2, T alpha2, const qint16 *weights)
{
    using compositetype = typename KoColorSpaceMathsTraits<T>::compositetype;

    const int sumOfWeights = 255;
    const T newAlpha = mixOpExpectedAlpha(alpha1, alpha2, weights);

    if (newAlpha > 0) {
        return safeDivideWithRound<compositetype>(
                safeDivideWithRound<compositetype>(
                    (weights[0] * color1 * alpha1) + (weights[1] * color2 * alpha2), sumOfWeights),
                newAlpha);
    } else {
        return 0;
    }
}

template <class T>
T mixOpNoAlphaExpectedColor(T color1, T color2, const qint16 *weights)
{
    using compositetype = typename KoColorSpaceMathsTraits<T>::compositetype;

    const int sumOfWeights = 255;
    return safeDivideWithRound<compositetype>((weights[0] * color1) + (weights[1] * color2), sumOfWeights);
}

void TestKoColorSpaceAbstract::testMixColorsOpU8()
{
    typedef KoColorSpaceTrait<quint8, 3, 2> U8ColorSpace;
    KoMixColorsOpImpl<U8ColorSpace> *op = new KoMixColorsOpImpl<U8ColorSpace>;

    quint8 pixel1[U8ColorSpace::channels_nb];
    quint8 pixel2[U8ColorSpace::channels_nb];
    quint8 outputPixel[U8ColorSpace::channels_nb];

    const int COLOR_CHANNEL_1 = 0;
    const int COLOR_CHANNEL_2 = 1;
    const int ALPHA_CHANNEL = 2;

    pixel1[COLOR_CHANNEL_1] = 255;
    pixel1[COLOR_CHANNEL_2] = 255;
    pixel1[ALPHA_CHANNEL] = 255;

    pixel2[COLOR_CHANNEL_1] = 0;
    pixel2[COLOR_CHANNEL_2] = 0;
    pixel2[ALPHA_CHANNEL] = 0;

    const quint8 *pixelPtrs[2];
    qint16 weights[2];

    pixelPtrs[0] = pixel1;
    pixelPtrs[1] = pixel2;

    weights[0] = 255;
    weights[1] = 0;

    op->mixColors(pixelPtrs, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpExpectedColor(pixel1[COLOR_CHANNEL_1], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_1], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpExpectedColor(pixel1[COLOR_CHANNEL_2], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_2], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[ALPHA_CHANNEL], mixOpExpectedAlpha(pixel1[ALPHA_CHANNEL], pixel2[ALPHA_CHANNEL], weights));

    weights[0] = 0;
    weights[1] = 255;

    op->mixColors(pixelPtrs, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpExpectedColor(pixel1[COLOR_CHANNEL_1], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_1], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpExpectedColor(pixel1[COLOR_CHANNEL_2], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_2], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[ALPHA_CHANNEL], mixOpExpectedAlpha(pixel1[ALPHA_CHANNEL], pixel2[ALPHA_CHANNEL], weights));

    weights[0] = 128;
    weights[1] = 127;

    op->mixColors(pixelPtrs, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpExpectedColor(pixel1[COLOR_CHANNEL_1], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_1], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpExpectedColor(pixel1[COLOR_CHANNEL_2], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_2], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[ALPHA_CHANNEL], mixOpExpectedAlpha(pixel1[ALPHA_CHANNEL], pixel2[ALPHA_CHANNEL], weights));

    pixel1[COLOR_CHANNEL_1] = 200;
    pixel1[COLOR_CHANNEL_2] = 100;
    pixel1[ALPHA_CHANNEL] = 255;

    pixel2[COLOR_CHANNEL_1] = 100;
    pixel2[COLOR_CHANNEL_2] = 200;
    pixel2[ALPHA_CHANNEL] = 255;

    op->mixColors(pixelPtrs, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpExpectedColor(pixel1[COLOR_CHANNEL_1], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_1], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpExpectedColor(pixel1[COLOR_CHANNEL_2], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_2], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[ALPHA_CHANNEL], mixOpExpectedAlpha(pixel1[ALPHA_CHANNEL], pixel2[ALPHA_CHANNEL], weights));

    pixel1[COLOR_CHANNEL_1] = 0;
    pixel1[COLOR_CHANNEL_2] = 0;
    pixel1[ALPHA_CHANNEL] = 0;

    pixel2[COLOR_CHANNEL_1] = 255;
    pixel2[COLOR_CHANNEL_2] = 255;
    pixel2[ALPHA_CHANNEL] = 255;

    weights[0] = 89;
    weights[1] = 166;

    op->mixColors(pixelPtrs, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpExpectedColor(pixel1[COLOR_CHANNEL_1], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_1], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpExpectedColor(pixel1[COLOR_CHANNEL_2], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_2], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[ALPHA_CHANNEL], mixOpExpectedAlpha(pixel1[ALPHA_CHANNEL], pixel2[ALPHA_CHANNEL], weights));
}

void TestKoColorSpaceAbstract::testMixColorsOpF32()
{
    typedef KoColorSpaceTrait<float, 3, 2> F32ColorSpace;
    KoMixColorsOpImpl<F32ColorSpace> *op = new KoMixColorsOpImpl<F32ColorSpace>;

    float pixel1[F32ColorSpace::channels_nb];
    float pixel2[F32ColorSpace::channels_nb];
    float outputPixel[F32ColorSpace::channels_nb];

    const int COLOR_CHANNEL_1 = 0;
    const int COLOR_CHANNEL_2 = 1;
    const int ALPHA_CHANNEL = 2;

    pixel1[COLOR_CHANNEL_1] = 1.0;
    pixel1[COLOR_CHANNEL_2] = 1.0;
    pixel1[ALPHA_CHANNEL] = 1.0;

    pixel2[COLOR_CHANNEL_1] = 0.0;
    pixel2[COLOR_CHANNEL_2] = 0.0;
    pixel2[ALPHA_CHANNEL] = 0.0;

    const quint8 *pixelPtrs[2];
    qint16 weights[2];

    pixelPtrs[0] = reinterpret_cast<const quint8 *>(pixel1);
    pixelPtrs[1] = reinterpret_cast<const quint8 *>(pixel2);

    weights[0] = 255;
    weights[1] = 0;

    op->mixColors(pixelPtrs, weights, 2, reinterpret_cast<quint8 *>(outputPixel));

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpExpectedColor(pixel1[COLOR_CHANNEL_1], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_1], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpExpectedColor(pixel1[COLOR_CHANNEL_2], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_2], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[ALPHA_CHANNEL], mixOpExpectedAlpha(pixel1[ALPHA_CHANNEL], pixel2[ALPHA_CHANNEL], weights));

    weights[0] = 0;
    weights[1] = 255;

    op->mixColors(pixelPtrs, weights, 2, reinterpret_cast<quint8 *>(outputPixel));

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpExpectedColor(pixel1[COLOR_CHANNEL_1], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_1], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpExpectedColor(pixel1[COLOR_CHANNEL_2], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_2], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[ALPHA_CHANNEL], mixOpExpectedAlpha(pixel1[ALPHA_CHANNEL], pixel2[ALPHA_CHANNEL], weights));

    weights[0] = 128;
    weights[1] = 127;

    op->mixColors(pixelPtrs, weights, 2, reinterpret_cast<quint8 *>(outputPixel));

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpExpectedColor(pixel1[COLOR_CHANNEL_1], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_1], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpExpectedColor(pixel1[COLOR_CHANNEL_2], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_2], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[ALPHA_CHANNEL], mixOpExpectedAlpha(pixel1[ALPHA_CHANNEL], pixel2[ALPHA_CHANNEL], weights));

    pixel1[COLOR_CHANNEL_1] = 200.0f / 255.0f;
    pixel1[COLOR_CHANNEL_2] = 100.0f / 255.0f;
    pixel1[ALPHA_CHANNEL] = 1.0;

    pixel2[COLOR_CHANNEL_1] = 100.0f / 255.0f;
    pixel2[COLOR_CHANNEL_2] = 200.0f / 255.0f;
    pixel2[ALPHA_CHANNEL] = 1.0;

    op->mixColors(pixelPtrs, weights, 2, reinterpret_cast<quint8 *>(outputPixel));

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpExpectedColor(pixel1[COLOR_CHANNEL_1], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_1], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpExpectedColor(pixel1[COLOR_CHANNEL_2], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_2], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[ALPHA_CHANNEL], mixOpExpectedAlpha(pixel1[ALPHA_CHANNEL], pixel2[ALPHA_CHANNEL], weights));

    pixel1[COLOR_CHANNEL_1] = 0;
    pixel1[COLOR_CHANNEL_2] = 0;
    pixel1[ALPHA_CHANNEL] = 0;

    pixel2[COLOR_CHANNEL_1] = 1.0;
    pixel2[COLOR_CHANNEL_2] = 1.0;
    pixel2[ALPHA_CHANNEL] = 1.0;

    weights[0] = 89;
    weights[1] = 166;

    op->mixColors(pixelPtrs, weights, 2, reinterpret_cast<quint8 *>(outputPixel));

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpExpectedColor(pixel1[COLOR_CHANNEL_1], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_1], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpExpectedColor(pixel1[COLOR_CHANNEL_2], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_2], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[ALPHA_CHANNEL], mixOpExpectedAlpha(pixel1[ALPHA_CHANNEL], pixel2[ALPHA_CHANNEL], weights));

    pixel1[COLOR_CHANNEL_1] = -667788;
    pixel1[COLOR_CHANNEL_2] = 0.123f;
    pixel1[ALPHA_CHANNEL] = 0.3f;

    pixel2[COLOR_CHANNEL_1] = 5;
    pixel2[COLOR_CHANNEL_2] = 100000;
    pixel2[ALPHA_CHANNEL] = 1.0;

    weights[0] = 89;
    weights[1] = 166;

    op->mixColors(pixelPtrs, weights, 2, reinterpret_cast<quint8 *>(outputPixel));

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpExpectedColor(pixel1[COLOR_CHANNEL_1], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_1], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpExpectedColor(pixel1[COLOR_CHANNEL_2], pixel1[ALPHA_CHANNEL],
             pixel2[COLOR_CHANNEL_2], pixel2[ALPHA_CHANNEL],
             weights));
    QCOMPARE(outputPixel[ALPHA_CHANNEL], mixOpExpectedAlpha(pixel1[ALPHA_CHANNEL], pixel2[ALPHA_CHANNEL], weights));
}

void TestKoColorSpaceAbstract::testMixColorsOpU8NoAlpha()
{
    typedef KoColorSpaceTrait < quint8, 2, -1 > U8NoAlphaColorSpace;
    KoMixColorsOpImpl<U8NoAlphaColorSpace> *op = new KoMixColorsOpImpl<U8NoAlphaColorSpace>;

    quint8 pixel1[U8NoAlphaColorSpace::channels_nb];
    quint8 pixel2[U8NoAlphaColorSpace::channels_nb];
    quint8 outputPixel[U8NoAlphaColorSpace::channels_nb];

    const int COLOR_CHANNEL_1 = 0;
    const int COLOR_CHANNEL_2 = 1;

    pixel1[COLOR_CHANNEL_1] = 255;
    pixel1[COLOR_CHANNEL_2] = 255;

    pixel2[COLOR_CHANNEL_1] = 0;
    pixel2[COLOR_CHANNEL_2] = 0;

    const quint8 *pixelPtrs[2];
    qint16 weights[2];

    pixelPtrs[0] = pixel1;
    pixelPtrs[1] = pixel2;

    weights[0] = 255;
    weights[1] = 0;

    op->mixColors(pixelPtrs, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_1], pixel2[COLOR_CHANNEL_1], weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_2], pixel2[COLOR_CHANNEL_2], weights));

    weights[0] = 0;
    weights[1] = 255;

    op->mixColors(pixelPtrs, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_1], pixel2[COLOR_CHANNEL_1], weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_2], pixel2[COLOR_CHANNEL_2], weights));

    weights[0] = 128;
    weights[1] = 127;

    op->mixColors(pixelPtrs, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_1], pixel2[COLOR_CHANNEL_1], weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_2], pixel2[COLOR_CHANNEL_2], weights));

    pixel1[COLOR_CHANNEL_1] = 200;
    pixel1[COLOR_CHANNEL_2] = 100;

    pixel2[COLOR_CHANNEL_1] = 100;
    pixel2[COLOR_CHANNEL_2] = 200;

    op->mixColors(pixelPtrs, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_1], pixel2[COLOR_CHANNEL_1], weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_2], pixel2[COLOR_CHANNEL_2], weights));
}

void TestKoColorSpaceAbstract::testMixColorsOpU8NoAlphaLinear()
{
    typedef KoColorSpaceTrait < quint8, 2, -1 > U8NoAlphaColorSpace;
    KoMixColorsOpImpl<U8NoAlphaColorSpace> *op = new KoMixColorsOpImpl<U8NoAlphaColorSpace>;

    quint8 pixels[2 * U8NoAlphaColorSpace::channels_nb];
    quint8 outputPixel[U8NoAlphaColorSpace::channels_nb];

    const int COLOR_CHANNEL_1 = 0;
    const int COLOR_CHANNEL_2 = 1;

    quint8 *pixel1 = pixels;
    quint8 *pixel2 = pixels + U8NoAlphaColorSpace::channels_nb;

    pixel1[COLOR_CHANNEL_1] = 255;
    pixel1[COLOR_CHANNEL_2] = 255;

    pixel2[COLOR_CHANNEL_1] = 0;
    pixel2[COLOR_CHANNEL_2] = 0;

    qint16 weights[2];

    weights[0] = 255;
    weights[1] = 0;

    op->mixColors(pixels, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_1], pixel2[COLOR_CHANNEL_1], weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_2], pixel2[COLOR_CHANNEL_2], weights));

    weights[0] = 0;
    weights[1] = 255;

    op->mixColors(pixels, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_1], pixel2[COLOR_CHANNEL_1], weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_2], pixel2[COLOR_CHANNEL_2], weights));

    weights[0] = 128;
    weights[1] = 127;

    op->mixColors(pixels, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_1], pixel2[COLOR_CHANNEL_1], weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_2], pixel2[COLOR_CHANNEL_2], weights));

    pixel1[COLOR_CHANNEL_1] = 200;
    pixel1[COLOR_CHANNEL_2] = 100;

    pixel2[COLOR_CHANNEL_1] = 100;
    pixel2[COLOR_CHANNEL_2] = 200;

    op->mixColors(pixels, weights, 2, outputPixel);

    QCOMPARE(outputPixel[COLOR_CHANNEL_1], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_1], pixel2[COLOR_CHANNEL_1], weights));
    QCOMPARE(outputPixel[COLOR_CHANNEL_2], mixOpNoAlphaExpectedColor(pixel1[COLOR_CHANNEL_2], pixel2[COLOR_CHANNEL_2], weights));
}

#include <KoColorSpaceRegistry.h>
#include <QByteArray>
#include <KoColor.h>
#include <KoCompositeOpRegistry.h>

KoColor makeColor(std::initializer_list<quint8> data, const KoColorSpace *cs)
{
    std::vector<quint8> buffer(data);
    return KoColor(buffer.data(), cs);
}

QBitArray makeChannelFlags(std::initializer_list<bool> flags)
{
    QBitArray result(flags.size());

    for (auto it = flags.begin(); it != flags.end(); ++it) {
        result.setBit(std::distance(flags.begin(), it), *it);
    }

    return result;
}

void TestKoColorSpaceAbstract::testBitBltCrossColorSpaceWithChannelFlags_data()
{
    QTest::addColumn<KoColor>("srcColor");
    QTest::addColumn<KoColor>("dstColor");
    QTest::addColumn<QBitArray>("channelFlags");
    QTest::addColumn<KoColor>("expectedColor");

    const KoColorSpace *alphaSpace = KoColorSpaceRegistry::instance()->alpha8();
    const KoColorSpace *rgbSpace = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace *cmykSpace = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id());

    KIS_ASSERT(alphaSpace);
    KIS_ASSERT(rgbSpace);
    KIS_ASSERT(cmykSpace);

    QTest::newRow("rgb->cmyk (full)")
        << makeColor({160,160,160,255}, rgbSpace)
        << makeColor({0,0,255,0,255}, cmykSpace)
        << QBitArray()
        << makeColor({0,0,0,108,255}, cmykSpace);

    QTest::newRow("rgb->cmyk (full, explicit)")
        << makeColor({160,160,160,255}, rgbSpace)
        << makeColor({0,0,255,0,255}, cmykSpace)
        << makeChannelFlags({true, true, true, true})
        << makeColor({0,0,0,108,255}, cmykSpace);

    QTest::newRow("rgb->cmyk (blue)")
        << makeColor({160,160,160,255}, rgbSpace)
        << makeColor({0,0,255,0,255}, cmykSpace)
        << makeChannelFlags({true, false, false, true})
        << makeColor({252,231,0,39,255}, cmykSpace);

    QTest::newRow("rgb->cmyk (green)")
        << makeColor({160,160,160,255}, rgbSpace)
        << makeColor({0,0,255,0,255}, cmykSpace)
        << makeChannelFlags({false, true, false, true})
        << makeColor({173,0,252,35,255}, cmykSpace);

    QTest::newRow("rgb->cmyk (red)")
        << makeColor({160,160,160,255}, rgbSpace)
        << makeColor({0,0,255,0,255}, cmykSpace)
        << makeChannelFlags({false, false, true, true})
        << makeColor({0,227,245,78,255}, cmykSpace);

    QTest::newRow("cmyk->rgb (full)")
        << makeColor({160,160,160,160,255}, cmykSpace)
        << makeColor({0,255,0,255}, rgbSpace)
        << QBitArray()
        << makeColor({51,46,56,255}, rgbSpace);

    QTest::newRow("cmyk->rgb (full, explicit)")
        << makeColor({160,160,160,160,255}, cmykSpace)
        << makeColor({0,255,0,255}, rgbSpace)
        << makeChannelFlags({true, true, true, true, true})
        << makeColor({51,46,56,255}, rgbSpace);

    QTest::newRow("cmyk->rgb (cyan)")
        << makeColor({160,160,160,160,255}, cmykSpace)
        << makeColor({0,255,0,255}, rgbSpace)
        << makeChannelFlags({true, false, false, false, true})
        << makeColor({234,181,22,255}, rgbSpace);

    QTest::newRow("cmyk->rgb (magenta)")
        << makeColor({160,160,160,160,255}, cmykSpace)
        << makeColor({0,255,0,255}, rgbSpace)
        << makeChannelFlags({false, true, false, false, true})
        << makeColor({166,111,241,255}, rgbSpace);

    QTest::newRow("cmyk->rgb (yellow)")
        << makeColor({160,160,160,160,255}, cmykSpace)
        << makeColor({0,255,0,255}, rgbSpace)
        << makeChannelFlags({false, false, true, false, true})
        << makeColor({102,244,255,255}, rgbSpace);

    QTest::newRow("cmyk->rgb (black)")
        << makeColor({160,160,160,160,255}, cmykSpace)
        << makeColor({0,255,0,255}, rgbSpace)
        << makeChannelFlags({false, false, false, true, true})
        << makeColor({108,107,109,255}, rgbSpace);

    QTest::newRow("cmyk->rgb (black, no-alpha)")
        << makeColor({160,160,160,160,255}, cmykSpace)
        << makeColor({0,255,0,180}, rgbSpace)
        << makeChannelFlags({false, false, false, true, false})
        << makeColor({108,107,109,180}, rgbSpace);

    QTest::newRow("rgb->cmyk (blue, no-alpha)")
        << makeColor({160,160,160,255}, rgbSpace)
        << makeColor({0,0,255,0,180}, cmykSpace)
        << makeChannelFlags({true, false, false, false})
        << makeColor({252,231,0,39,180}, cmykSpace);

    QTest::newRow("rgb->alpha (full)")
        << makeColor({120,120,120,255}, rgbSpace)
        << makeColor({188}, alphaSpace)
        << QBitArray()
        << makeColor({120}, alphaSpace);

    QTest::newRow("rgb->alpha (full, explicit)")
        << makeColor({120,120,120,255}, rgbSpace)
        << makeColor({188}, alphaSpace)
        << makeChannelFlags({true, true, true, true})
        << makeColor({120}, alphaSpace);

    QTest::newRow("rgb->alpha (transp source)")
        << makeColor({120,120,120,128}, rgbSpace)
        << makeColor({188}, alphaSpace)
        << makeChannelFlags({true, true, true, true})
        << makeColor({154}, alphaSpace);

    // dst color is always fully opaque in alpha channel
    QTest::newRow("rgb->alpha (alpha locked)")
        << makeColor({120,120,120,128}, rgbSpace)
        << makeColor({188}, alphaSpace)
        << makeChannelFlags({true, true, true, false})
        << makeColor({154}, alphaSpace);

    QTest::newRow("rgb->alpha (colors locked)")
        << makeColor({120,120,120,128}, rgbSpace)
        << makeColor({188}, alphaSpace)
        << makeChannelFlags({false, false, false, true})
        << makeColor({188}, alphaSpace);
}

void TestKoColorSpaceAbstract::testBitBltCrossColorSpaceWithChannelFlags()
{
    QFETCH(const KoColor, srcColor);
    QFETCH(const KoColor, dstColor);
    QFETCH(const KoColor, expectedColor);
    QFETCH(const QBitArray, channelFlags);

    const KoColorSpace *srcSpace = srcColor.colorSpace();
    const KoColorSpace *dstSpace = dstColor.colorSpace();

    const int numColumns = 17;
    const int numRows = 23;
    const int numPixels = numColumns * numRows;
    const int srcPixelSize = srcSpace->pixelSize();
    const int dstPixelSize = dstSpace->pixelSize();

    QByteArray srcData(numColumns * numRows * srcPixelSize, Qt::Uninitialized);
    QByteArray dstData(numColumns * numRows * dstPixelSize, Qt::Uninitialized);

    quint8 *srcPtr = reinterpret_cast<quint8*>(srcData.data());
    quint8 *dstPtr = reinterpret_cast<quint8*>(dstData.data());

    for (int i = 0; i < numPixels; i++) {
        memcpy(srcPtr + i * srcPixelSize, srcColor.data(), srcPixelSize);
        memcpy(dstPtr + i * dstPixelSize, dstColor.data(), dstPixelSize);
    }

    const KoCompositeOp *op = dstSpace->compositeOp(COMPOSITE_OVER);

    KoCompositeOp::ParameterInfo params;
    params.rows = numRows;
    params.cols = numColumns;
    params.srcRowStart = srcPtr;
    params.srcRowStride = numColumns * srcPixelSize;
    params.dstRowStart = dstPtr;
    params.dstRowStride = numColumns * dstPixelSize;
    params.channelFlags = channelFlags;

    dstSpace->bitBlt(srcSpace, params, op,
                     KoColorConversionTransformation::internalRenderingIntent(),
                     KoColorConversionTransformation::internalConversionFlags());

    for (int i = 0; i < numPixels; i++) {
        KoColor srcPixel(srcPtr + i * srcPixelSize, srcSpace);
        KoColor dstPixel(dstPtr + i * dstPixelSize, dstSpace);

//        if (i == 0) {
//            qDebug() << ppVar(i) << ppVar(srcPixel) << ppVar(dstPixel);
//        }
        QCOMPARE(dstPixel, expectedColor);
    }
}


SIMPLE_TEST_MAIN(TestKoColorSpaceAbstract)
