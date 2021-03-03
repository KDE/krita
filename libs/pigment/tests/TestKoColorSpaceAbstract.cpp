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


QTEST_GUILESS_MAIN(TestKoColorSpaceAbstract)
