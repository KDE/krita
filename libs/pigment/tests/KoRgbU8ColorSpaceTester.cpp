/*
 *  SPDX-FileCopyrightText: 2005 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2008 Bart Coppens <kde@bartcoppens.be>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoRgbU8ColorSpaceTester.h"

#include "KoColorModelStandardIds.h"

#include <simpletest.h>
#include <DebugPigment.h>
#include <string.h>

#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoCompositeOp.h"
#include "KoMixColorsOp.h"
#include <KoCompositeOpRegistry.h>
#include "sdk/tests/kistest.h"


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

        qDebug() << depthId.id();
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
        
        qDebug() << src.toQColor() << dst.toQColor();

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
            qDebug() << op->id()
                     << "easy case:" << goodDst[2]
                     << "difficult case:" << badDst[2];

            qDebug() << "The composite op has failed to erase the color "
                "channel which was hidden by zero alpha.";
            qDebug() << "Expected Blue channel:" << 0;
            qDebug() << "Actual Blue channel:  " << badDst[2];

            QFAIL("Failed to erase color channel");
        }
    }
}

// for posix_memalign()
#include <stdlib.h>

#if defined Q_OS_WIN
#define MEMALIGN_ALLOC(p, a, s) ((*(p)) = _aligned_malloc((s), (a)), *(p) ? 0 : errno)
#define MEMALIGN_FREE(p) _aligned_free((p))
#else
#define MEMALIGN_ALLOC(p, a, s) posix_memalign((p), (a), (s))
#define MEMALIGN_FREE(p) free((p))
#endif

void KoRgbU8ColorSpaceTester::testCompositeCopyDivisionByZero()
{
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    const KoCompositeOp *op = cs->compositeOp(COMPOSITE_COPY);

    const int pixelAlignment = sizeof(float) * 16; // 512-bit alignment should be enough for everyone! (c)
    const int numPixels = 256;
    void *src = 0;
    void *dst = 0;

    int result = 0;

    result = MEMALIGN_ALLOC(&src, pixelAlignment, cs->pixelSize() * numPixels);
    KIS_ASSERT(!result);

    result = MEMALIGN_ALLOC(&dst, pixelAlignment, cs->pixelSize() * numPixels);
    KIS_ASSERT(!result);

    // generate buffers in misaligned manner
    int numTestablePixels = numPixels - 1;
    quint8 * srcPtr = reinterpret_cast<quint8*>(src) + cs->pixelSize();
    quint8 * dstPtr = reinterpret_cast<quint8*>(dst) + cs->pixelSize();

    quint8 goodSrc[] = {128,128,128,129};
    quint8 goodDst[] = {10,10,10,11};
    quint8 badSrc[] = {128,128,128,0};
    quint8 badDst[] = {12,12,12,0};


    auto testBadPixel = [cs, op, srcPtr, dstPtr, numTestablePixels] (int badPixelPos,
            const quint8 *goodSrc,
            const quint8 *goodDst,
            const quint8 *badSrc,
            const quint8 *badDst,
            quint8 opacity,
            quint8 *expectedDst) {

        quint8 *badPixelDstPtr = dstPtr + badPixelPos * cs->pixelSize();

        for (int i = 0; i < numTestablePixels; i++) {
            if (i != badPixelPos) {
                memcpy(srcPtr + cs->pixelSize() * i, goodSrc, cs->pixelSize());
                memcpy(dstPtr + cs->pixelSize() * i, goodDst, cs->pixelSize());
            } else {
                memcpy(srcPtr + cs->pixelSize() * i, badSrc, cs->pixelSize());
                memcpy(dstPtr + cs->pixelSize() * i, badDst, cs->pixelSize());
            }
        }

        op->composite(dstPtr, numTestablePixels * cs->pixelSize(),
                      srcPtr, numTestablePixels * cs->pixelSize(),
                      0, 0,
                      1, numTestablePixels, opacity);

        if (memcmp(badPixelDstPtr, expectedDst, cs->pixelSize()) != 0) {
            qDebug() << "badPixelPos" << badPixelPos;
            qDebug() << "opacity" << opacity;
            qDebug() << "oriS" << badSrc[0] << badSrc[1] << badSrc[2] << badSrc[3];
            qDebug() << "oriS" << badDst[0] << badDst[1] << badDst[2] << badDst[3];
            qDebug() << "expD" << expectedDst[0] << expectedDst[1] << expectedDst[2] << expectedDst[3];
            qDebug() << "dst1" << badPixelDstPtr[0] << badPixelDstPtr[1] << badPixelDstPtr[2] << badPixelDstPtr[3];
            QFAIL("Failed to compose pixels");
        }
    };

    /**
     * This test is supposed to catch irregularities between vector and scalar versions
     * of the composite op. In vector version we handle division be zero in a relaxed
     * way, so it some cases the content of the color channels may be different from
     * the one of the scalar versions of the algorithm. It is only allowed when the
     * pixel's alpha is null. In such case Krita considers pixel state as "undefined",
     * so any value is considered okay.
     */

    {
        quint8 goodSrc[] = {128,128,128,129};
        quint8 goodDst[] = {10,10,10,11};
        quint8 badSrc[] = {128,128,128,129};
        quint8 badDst[] = {12,12,12,0};
        quint8 *expectedDst = badSrc;
        testBadPixel(1, goodSrc, goodDst, badSrc, badDst, 255, expectedDst);
    }

    {
        quint8 goodSrc[] = {128,128,128,129};
        quint8 goodDst[] = {10,10,10,11};
        quint8 badSrc[] = {128,128,128,129};
        quint8 badDst[] = {12,12,12,0};
        quint8 *expectedDst = badSrc;
        testBadPixel(10, goodSrc, goodDst, badSrc, badDst, 255, expectedDst);
    }


    {
        quint8 goodSrc[] = {128,128,128,129};
        quint8 goodDst[] = {10,10,10,11};
        quint8 badSrc[] = {128,128,128,129};
        quint8 badDst[] = {12,12,12,0};
        quint8 expectedDst[] = {128,128,128,65};
        testBadPixel(1, goodSrc, goodDst, badSrc, badDst, 128, expectedDst);
    }

    {
        quint8 goodSrc[] = {128,128,128,129};
        quint8 goodDst[] = {10,10,10,11};
        quint8 badSrc[] = {128,128,128,129};
        quint8 badDst[] = {12,12,12,0};
        quint8 expectedDst[] = {128,128,128,65};
        testBadPixel(10, goodSrc, goodDst, badSrc, badDst, 128, expectedDst);
    }

    {
        quint8 goodSrc[] = {128,128,128,129};
        quint8 goodDst[] = {10,10,10,11};
        quint8 badSrc[] = {128,128,128,0};
        quint8 badDst[] = {12,12,12,0};
        quint8 expectedDst[] = {0,0,0,0}; // NOTE: the pixel has been changed, even though visualy it hasn't (due to zero alpha)
        testBadPixel(1, goodSrc, goodDst, badSrc, badDst, 128, expectedDst);
    }

    {
        quint8 goodSrc[] = {128,128,128,129};
        quint8 goodDst[] = {10,10,10,11};
        quint8 badSrc[] = {128,128,128,0};
        quint8 badDst[] = {12,12,12,0};
        quint8 expectedDst[] = {255,255,255,0}; // NOTE: the result is different from the scalar version, but we don't care, since alpha is still zero
        testBadPixel(10, goodSrc, goodDst, badSrc, badDst, 128, expectedDst);
    }

    {
        quint8 goodSrc[] = {128,128,128,129};
        quint8 goodDst[] = {10,10,10,11};
        quint8 badSrc[] = {128,128,128,0};
        quint8 badDst[] = {12,12,12,127};
        quint8 expectedDst[] = {12,12,12,63};
        testBadPixel(1, goodSrc, goodDst, badSrc, badDst, 128, expectedDst);
    }

    {
        quint8 goodSrc[] = {128,128,128,129};
        quint8 goodDst[] = {10,10,10,11};
        quint8 badSrc[] = {128,128,128,0};
        quint8 badDst[] = {12,12,12,127};
        quint8 expectedDst[] = {12,12,12,63};
        testBadPixel(10, goodSrc, goodDst, badSrc, badDst, 128, expectedDst);
    }
}

KISTEST_MAIN(KoRgbU8ColorSpaceTester)
