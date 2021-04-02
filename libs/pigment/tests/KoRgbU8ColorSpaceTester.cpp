/*
 *  SPDX-FileCopyrightText: 2005 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2008 Bart Coppens <kde@bartcoppens.be>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoRgbU8ColorSpaceTester.h"

#include "KoColorModelStandardIds.h"

#include <QTest>
#include <DebugPigment.h>
#include <string.h>

#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoCompositeOp.h"
#include "KoMixColorsOp.h"
#include <KoCompositeOpRegistry.h>


#define NUM_CHANNELS 4

#define RED_CHANNEL 0
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL 2
#define ALPHA_CHANNEL 3
#include <KoColorProfile.h>

void KoRgbU8ColorSpaceTester::testBasics()
{
}

#define PIXEL_RED 0
#define PIXEL_GREEN 1
#define PIXEL_BLUE 2
#define PIXEL_ALPHA 3

void KoRgbU8ColorSpaceTester::testMixColors()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KoMixColorsOp * mixOp = cs->mixColorsOp();

    // Test mixColors.
    quint8 pixel1[4];
    quint8 pixel2[4];
    quint8 outputPixel[4];

    pixel1[PIXEL_RED] = 255;
    pixel1[PIXEL_GREEN] = 255;
    pixel1[PIXEL_BLUE] = 255;
    pixel1[PIXEL_ALPHA] = 255;

    pixel2[PIXEL_RED] = 0;
    pixel2[PIXEL_GREEN] = 0;
    pixel2[PIXEL_BLUE] = 0;
    pixel2[PIXEL_ALPHA] = 0;

    const quint8 *pixelPtrs[2];
    qint16 weights[2];

    pixelPtrs[0] = pixel1;
    pixelPtrs[1] = pixel2;

    weights[0] = 255;
    weights[1] = 0;

    mixOp->mixColors(pixelPtrs, weights, 2, outputPixel);

    QVERIFY((int)outputPixel[PIXEL_RED] == 255);
    QVERIFY((int)outputPixel[PIXEL_GREEN] == 255);
    QVERIFY((int)outputPixel[PIXEL_BLUE] == 255);
    QVERIFY((int)outputPixel[PIXEL_ALPHA] == 255);

    weights[0] = 0;
    weights[1] = 255;

    mixOp->mixColors(pixelPtrs, weights, 2, outputPixel);

    QVERIFY((int)outputPixel[PIXEL_RED] == 0);
    QVERIFY((int)outputPixel[PIXEL_GREEN] == 0);
    QVERIFY((int)outputPixel[PIXEL_BLUE] == 0);
    QVERIFY((int)outputPixel[PIXEL_ALPHA] == 0);

    weights[0] = 128;
    weights[1] = 127;

    mixOp->mixColors(pixelPtrs, weights, 2, outputPixel);

    QVERIFY((int)outputPixel[PIXEL_RED] == 255);
    QVERIFY((int)outputPixel[PIXEL_GREEN] == 255);
    QVERIFY((int)outputPixel[PIXEL_BLUE] == 255);
    QVERIFY((int)outputPixel[PIXEL_ALPHA] == 128);

    pixel1[PIXEL_RED] = 200;
    pixel1[PIXEL_GREEN] = 100;
    pixel1[PIXEL_BLUE] = 50;
    pixel1[PIXEL_ALPHA] = 255;

    pixel2[PIXEL_RED] = 100;
    pixel2[PIXEL_GREEN] = 200;
    pixel2[PIXEL_BLUE] = 20;
    pixel2[PIXEL_ALPHA] = 255;

    mixOp->mixColors(pixelPtrs, weights, 2, outputPixel);

    QVERIFY((int)outputPixel[PIXEL_RED] == 150);
    QCOMPARE((int)outputPixel[PIXEL_GREEN], 150);
    QVERIFY((int)outputPixel[PIXEL_BLUE] == 35);
    QVERIFY((int)outputPixel[PIXEL_ALPHA] == 255);

    pixel1[PIXEL_RED] = 0;
    pixel1[PIXEL_GREEN] = 0;
    pixel1[PIXEL_BLUE] = 0;
    pixel1[PIXEL_ALPHA] = 0;

    pixel2[PIXEL_RED] = 255;
    pixel2[PIXEL_GREEN] = 255;
    pixel2[PIXEL_BLUE] = 255;
    pixel2[PIXEL_ALPHA] = 254;

    weights[0] = 89;
    weights[1] = 166;

    mixOp->mixColors(pixelPtrs, weights, 2, outputPixel);

    QVERIFY((int)outputPixel[PIXEL_RED] == 255);
    QVERIFY((int)outputPixel[PIXEL_GREEN] == 255);
    QVERIFY((int)outputPixel[PIXEL_BLUE] == 255);
    QVERIFY((int)outputPixel[PIXEL_ALPHA] == 165);
}
void KoRgbU8ColorSpaceTester::testMixColorsAverage()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KoMixColorsOp * mixOp = cs->mixColorsOp();

    // Test mixColors.
    quint8 pixel1[4];
    quint8 pixel2[4];
    quint8 outputPixel[4];

    pixel1[PIXEL_RED] = 255;
    pixel1[PIXEL_GREEN] = 255;
    pixel1[PIXEL_BLUE] = 255;
    pixel1[PIXEL_ALPHA] = 255;

    pixel2[PIXEL_RED] = 0;
    pixel2[PIXEL_GREEN] = 0;
    pixel2[PIXEL_BLUE] = 0;
    pixel2[PIXEL_ALPHA] = 0;

    const quint8 *pixelPtrs[2];

    pixelPtrs[0] = pixel1;
    pixelPtrs[1] = pixel2;

    mixOp->mixColors(pixelPtrs, 2, outputPixel);

    QCOMPARE((int)outputPixel[PIXEL_RED], 255);
    QCOMPARE((int)outputPixel[PIXEL_GREEN], 255);
    QCOMPARE((int)outputPixel[PIXEL_BLUE], 255);
    QCOMPARE((int)outputPixel[PIXEL_ALPHA], 128);

    pixel2[PIXEL_ALPHA] = 255;
    mixOp->mixColors(pixelPtrs, 2, outputPixel);

    QCOMPARE((int)outputPixel[PIXEL_RED], 128);
    QCOMPARE((int)outputPixel[PIXEL_GREEN], 128);
    QCOMPARE((int)outputPixel[PIXEL_BLUE], 128);
    QCOMPARE((int)outputPixel[PIXEL_ALPHA], 255);
}

void KoRgbU8ColorSpaceTester::testCompositeOps()
{
    // Just COMPOSITE_COPY for now

    QList<KoID> depthIDs = KoColorSpaceRegistry::instance()->colorDepthList(RGBAColorModelID.id(),
                           KoColorSpaceRegistry::AllColorSpaces);

    Q_FOREACH (const KoID& depthId, depthIDs) {

        if (depthId.id().contains("Float")) continue;

        dbgPigment << depthId.id();
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(
                                     RGBAColorModelID.id(), depthId.id(), "");
        
        const KoCompositeOp* copyOp = cs->compositeOp(COMPOSITE_COPY);
        KoColor src(cs), dst(cs);

        QColor red(255, 0, 0);
        QColor blue(0, 0, 255);
        QColor transparentRed(255, 0, 0, 0);

        // Copying a color over another color should replace the original color
        src.fromQColor(red);
        dst.fromQColor(blue);
        
        dbgPigment << src.toQColor() << dst.toQColor();

        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) != 0);

        copyOp->composite(dst.data(), cs->pixelSize(), src.data(), cs->pixelSize(),
                          0, 0, 1, 1, OPACITY_OPAQUE_U8);

        src.fromQColor(red);
        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) == 0);

        // Copying something transparent over something non-transparent should, of course, make the dst transparent
        src.fromQColor(transparentRed);
        dst.fromQColor(blue);

        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) != 0);

        copyOp->composite(dst.data(), cs->pixelSize(), src.data(), cs->pixelSize(),
                          0, 0, 1, 1, OPACITY_OPAQUE_U8);

        src.fromQColor(transparentRed);
        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) == 0);

        // Copying something solid over something transparent
        src.fromQColor(blue);
        dst.fromQColor(transparentRed);

        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) != 0);

        copyOp->composite(dst.data(), cs->pixelSize(), src.data(), cs->pixelSize(),
                          0, 0, 1, 1, OPACITY_OPAQUE_U8);

        src.fromQColor(blue);
        QVERIFY(memcmp(dst.data(), src.data(), cs->pixelSize()) == 0);

    }
}

void KoRgbU8ColorSpaceTester::testCompositeOpsWithChannelFlags()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    QList<KoCompositeOp*> ops = cs->compositeOps();

    Q_FOREACH (const KoCompositeOp *op, ops) {
        /**
         * ALPHA_DARKEN composite op doesn't take channel
         * flags into account, so just skip it
         */
        if (op->id() == COMPOSITE_ALPHA_DARKEN) continue;
        if (op->id() == COMPOSITE_DISSOLVE) continue;

        quint8 src[] = {128,128,128,129};
        quint8 goodDst[] = {10,10,10,11};
        quint8 badDst[] = {12,12,12,0};

        KoCompositeOp::ParameterInfo params;
        params.maskRowStart  = 0;
        params.dstRowStride  = 0;
        params.srcRowStride  = 0;
        params.maskRowStride = 0;
        params.rows          = 1;
        params.cols          = 1;
        params.opacity       = 1.0f;
        params.flow          = 1.0f;

        QBitArray channelFlags(4, true);
        channelFlags[2] = false;
        params.channelFlags  = channelFlags;

        params.srcRowStart   = src;

        params.dstRowStart   = goodDst;
        op->composite(params);

        params.dstRowStart   = badDst;
        op->composite(params);

        /**
         * The badDst has zero alpha, so the channels should be zeroed
         * before increasing alpha of the pixel
         */
        if (badDst[3] != 0 && badDst[2] != 0) {
            dbgPigment << op->id()
                     << "easy case:" << goodDst[2]
                     << "difficult case:" << badDst[2];

            dbgPigment << "The composite op has failed to erase the color "
                "channel which was hidden by zero alpha.";
            dbgPigment << "Expected Blue channel:" << 0;
            dbgPigment << "Actual Blue channel:  " << badDst[2];

            QFAIL("Failed to erase color channel");
        }
    }
}

#include <QScopedPointer>
#include <KoOptimizedRgbPixelDataScalerU8ToU16Factory.h>
#include <QByteArray>

void KoRgbU8ColorSpaceTester::testScaler()
{
    QScopedPointer<KoOptimizedRgbPixelDataScalerU8ToU16Base> scaler(KoOptimizedRgbPixelDataScalerU8ToU16Factory::create());

    const int numPixels = 31;
    QByteArray srcBytes(1 + numPixels * 4 * sizeof(quint8), 0);
    QByteArray dstBytes(1 + numPixels * 4 * sizeof(quint16), 0);

    const quint8 pattern[] = {0, 128, 192, 255};
    const quint16 expectedDstPattern[] = {0, 32896, 49344, 65535};

    quint8 *unalignedSrcPtr = reinterpret_cast<quint8*>(srcBytes.data() + 1);
    quint8 *unalignedDstPtr = reinterpret_cast<quint8*>(dstBytes.data() + 1);


    for (int i = 0; i < numPixels * 4; i++) {
        unalignedSrcPtr[i] = pattern[i % 4];
    }

    scaler->convertU8ToU16(unalignedSrcPtr, 1, unalignedDstPtr, 1, 1, numPixels);

    const quint8 *srcPtr = reinterpret_cast<const quint8*>(unalignedSrcPtr);
    const quint16 *dstPtr = reinterpret_cast<const quint16*>(unalignedDstPtr);

    for (int i = 0; i < numPixels; i++) {
//        qDebug() << i
//                 << "S" << srcPtr[i * 4 + 0] << srcPtr[i * 4 + 1] << srcPtr[i * 4 + 2] << srcPtr[i * 4 + 3]
//                 << "D" << dstPtr[i * 4 + 0] << dstPtr[i * 4 + 1] << dstPtr[i * 4 + 2] << dstPtr[i * 4 + 3];

        QCOMPARE(dstPtr[i * 4 + 0], expectedDstPattern[0]);
        QCOMPARE(dstPtr[i * 4 + 1], expectedDstPattern[1]);
        QCOMPARE(dstPtr[i * 4 + 2], expectedDstPattern[2]);
        QCOMPARE(dstPtr[i * 4 + 3], expectedDstPattern[3]);
    }

    scaler->convertU16ToU8(unalignedDstPtr, 1, unalignedSrcPtr, 1, 1, numPixels);

    for (int i = 0; i < numPixels; i++) {
//        qDebug() << i
//                 << "S" << srcPtr[i * 4 + 0] << srcPtr[i * 4 + 1] << srcPtr[i * 4 + 2] << srcPtr[i * 4 + 3]
//                 << "D" << dstPtr[i * 4 + 0] << dstPtr[i * 4 + 1] << dstPtr[i * 4 + 2] << dstPtr[i * 4 + 3];

        QCOMPARE(srcPtr[i * 4 + 0], pattern[0]);
        QCOMPARE(srcPtr[i * 4 + 1], pattern[1]);
        QCOMPARE(srcPtr[i * 4 + 2], pattern[2]);
        QCOMPARE(srcPtr[i * 4 + 3], pattern[3]);
    }


}

QTEST_GUILESS_MAIN(KoRgbU8ColorSpaceTester)
