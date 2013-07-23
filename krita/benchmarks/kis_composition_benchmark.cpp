/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_composition_benchmark.h"

#include <qtest_kde.h>

#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoColorSpaceRegistry.h>

#include <KoColorSpaceTraits.h>
#include <KoCompositeOpAlphaDarken.h>
#include <KoCompositeOpOver.h>
#include "KoOptimizedCompositeOpFactory.h"



// for calculation of the needed alignment
#include <config-vc.h>
#ifdef HAVE_VC
#include <Vc/Vc>
#include <Vc/IO>

#include <KoOptimizedCompositeOpOver32.h>
#include <KoOptimizedCompositeOpAlphaDarken32.h>
#endif

// for memalign()
#if !defined(__APPLE__)
#include <malloc.h>
#endif

const int alpha_pos = 3;

enum AlphaRange {
    ALPHA_ZERO,
    ALPHA_UNIT,
    ALPHA_RANDOM
};

inline quint8 generateAlphaValue(AlphaRange range) {
    quint8 value = 0;

    switch (range) {
    case ALPHA_ZERO:
        break;
    case ALPHA_UNIT:
        value = 255;
        break;
    case ALPHA_RANDOM:
        value = qrand() % 255;
        break;
    }

    return value;
}

void generateDataLine(uint seed, int numPixels, quint8 *srcPixels, quint8 *dstPixels, quint8 *mask, AlphaRange srcAlphaRange, AlphaRange dstAlphaRange)
{
    Q_ASSERT(numPixels >= 4);

    for (int i = 0; i < 4; i++) {
        srcPixels[4*i]   = i * 10 + 30;
        srcPixels[4*i+1] = i * 10 + 30;
        srcPixels[4*i+2] = i * 10 + 30;
        srcPixels[4*i+3] = i * 10 + 35;

        dstPixels[4*i]   = i * 10 + 160;
        dstPixels[4*i+1] = i * 10 + 160;
        dstPixels[4*i+2] = i * 10 + 160;
        dstPixels[4*i+3] = i * 10 + 165;

        mask[i] = i * 10 + 225;
    }

    qsrand(seed);
    numPixels -= 4;
    srcPixels += 4 * 4;
    dstPixels += 4 * 4;
    mask += 4;

    for (int i = 0; i < numPixels; i++) {
        for (int j = 0; j < 3; j++) {
            *(srcPixels++) = qrand() % 255;
            *(dstPixels++) = qrand() % 255;
        }

        *(srcPixels++) = generateAlphaValue(srcAlphaRange);
        *(dstPixels++) = generateAlphaValue(dstAlphaRange);

        *(mask++) = qrand() % 255;
    }
}

void printData(int numPixels, quint8 *srcPixels, quint8 *dstPixels, quint8 *mask)
{
    for (int i = 0; i < numPixels; i++) {
        qDebug() << "Src: "
                 << srcPixels[i*4] << "\t"
                 << srcPixels[i*4+1] << "\t"
                 << srcPixels[i*4+2] << "\t"
                 << srcPixels[i*4+3] << "\t"
                 << "Msk:" << mask[i];

        qDebug() << "Dst: "
                 << dstPixels[i*4] << "\t"
                 << dstPixels[i*4+1] << "\t"
                 << dstPixels[i*4+2] << "\t"
                 << dstPixels[i*4+3];
    }
}

const int rowStride = 64;
const int totalRows = 64;
const QRect processRect(0,0,64,64);
const int numPixels = rowStride * totalRows;
const int numTiles = 1024;


struct Tile {
    quint8 *src;
    quint8 *dst;
    quint8 *mask;
};
#include <stdint.h>
QVector<Tile> generateTiles(int size,
                            const int srcAlignmentShift,
                            const int dstAlignmentShift,
                            AlphaRange srcAlphaRange,
                            AlphaRange dstAlphaRange)
{
    QVector<Tile> tiles(size);

#ifdef HAVE_VC
    const int vecSize = Vc::float_v::Size;
#else
    const int vecSize = 1;
#endif

    for (int i = 0; i < size; i++) {
#if !defined(__APPLE__) // In OSX it's automatically aligned by 16 bytes
        tiles[i].src = (quint8*)memalign(vecSize * 4, numPixels * 4 + srcAlignmentShift) + srcAlignmentShift;
        tiles[i].dst = (quint8*)memalign(vecSize * 4, numPixels * 4 + dstAlignmentShift) + dstAlignmentShift;
        tiles[i].mask = (quint8*)memalign(vecSize, numPixels);
#else
        tiles[i].src = (quint8*)malloc(numPixels * 4 + srcAlignmentShift) + srcAlignmentShift;
        tiles[i].dst = (quint8*)malloc(numPixels * 4 + dstAlignmentShift) + dstAlignmentShift;
        tiles[i].mask = (quint8*)malloc(numPixels);
#endif
        generateDataLine(1, numPixels, tiles[i].src, tiles[i].dst, tiles[i].mask, srcAlphaRange, dstAlphaRange);
    }

    return tiles;
}

void freeTiles(QVector<Tile> tiles,
               const int srcAlignmentShift,
               const int dstAlignmentShift)
{
    foreach (const Tile &tile, tiles) {
        free(tile.src - srcAlignmentShift);
        free(tile.dst - dstAlignmentShift);
        free(tile.mask);
    }
}

inline bool fuzzyCompare(quint8 a, quint8 b, quint8 prec) {
    return qAbs(a - b) <= prec;
}

inline bool comparePixels(quint8 *p1, quint8*p2, quint8 prec) {
    return (p1[3] == p2[3] && p1[3] == 0) ||
        (fuzzyCompare(p1[0], p2[0], prec) &&
         fuzzyCompare(p1[1], p2[1], prec) &&
         fuzzyCompare(p1[2], p2[2], prec) &&
         fuzzyCompare(p1[3], p2[3], prec));

}

bool compareTwoOps(bool haveMask, const KoCompositeOp *op1, const KoCompositeOp *op2)
{
    QVector<Tile> tiles = generateTiles(2, 16, 16, ALPHA_RANDOM, ALPHA_RANDOM);

    KoCompositeOp::ParameterInfo params;
    params.dstRowStride  = 4 * rowStride;
    params.srcRowStride  = 4 * rowStride;
    params.maskRowStride = rowStride;
    params.rows          = processRect.height();
    params.cols          = processRect.width();
    params.opacity       = 0.5*1.0f;
    params.flow          = 0.3*1.0f;
    params.channelFlags  = QBitArray();

    params.dstRowStart   = tiles[0].dst;
    params.srcRowStart   = tiles[0].src;
    params.maskRowStart  = haveMask ? tiles[0].mask : 0;
    op1->composite(params);

    params.dstRowStart   = tiles[1].dst;
    params.srcRowStart   = tiles[1].src;
    params.maskRowStart  = haveMask ? tiles[1].mask : 0;
    op2->composite(params);

    quint8 *dst1 = tiles[0].dst;
    quint8 *dst2 = tiles[1].dst;
    for (int i = 0; i < numPixels; i++) {
        if (!comparePixels(dst1, dst2, 7)) {

            qDebug() << "Wrong result:" << i;
            qDebug() << "Act: " << dst1[0] << dst1[1] << dst1[2] << dst1[3];
            qDebug() << "Exp: " << dst2[0] << dst2[1] << dst2[2] << dst2[3];

            quint8 *src1 = tiles[0].src + 4 * i;
            quint8 *src2 = tiles[1].src + 4 * i;

            qDebug() << "SrcA:" << src1[0] << src1[1] << src1[2] << src1[3];
            qDebug() << "SrcE:" << src2[0] << src2[1] << src2[2] << src2[3];

            qDebug() << "MskA:" << tiles[0].mask[i];
            qDebug() << "MskE:" << tiles[1].mask[i];

            return false;
        }
        dst1 += 4;
        dst2 += 4;
    }

    freeTiles(tiles, 16, 16);

    return true;
}

QString getTestName(bool haveMask,
                    const int srcAlignmentShift,
                    const int dstAlignmentShift,
                    AlphaRange srcAlphaRange,
                    AlphaRange dstAlphaRange)
{

    QString testName;
    testName +=
        !srcAlignmentShift && !dstAlignmentShift ? "Aligned   " :
        !srcAlignmentShift &&  dstAlignmentShift ? "SrcUnalig " :
         srcAlignmentShift && !dstAlignmentShift ? "DstUnalig " :
         srcAlignmentShift &&  dstAlignmentShift ? "Unaligned " : "###";

    testName += haveMask ? "Mask   " : "NoMask ";

    testName +=
        srcAlphaRange == ALPHA_RANDOM ? "SrcRand " :
        srcAlphaRange == ALPHA_ZERO   ? "SrcZero " :
        srcAlphaRange == ALPHA_UNIT   ? "SrcUnit " : "###";

    testName +=
        dstAlphaRange == ALPHA_RANDOM ? "DstRand" :
        dstAlphaRange == ALPHA_ZERO   ? "DstZero" :
        dstAlphaRange == ALPHA_UNIT   ? "DstUnit" : "###";

    return testName;
}

void benchmarkCompositeOp(const KoCompositeOp *op,
                          bool haveMask,
                          qreal opacity,
                          qreal flow,
                          const int srcAlignmentShift,
                          const int dstAlignmentShift,
                          AlphaRange srcAlphaRange,
                          AlphaRange dstAlphaRange)
{
    QString testName = getTestName(haveMask, srcAlignmentShift, dstAlignmentShift, srcAlphaRange, dstAlphaRange);

    QVector<Tile> tiles =
        generateTiles(numTiles, srcAlignmentShift, dstAlignmentShift, srcAlphaRange, dstAlphaRange);

//    qDebug() << "Initial values:";
//    printData(8, tiles[0].src, tiles[0].dst, tiles[0].mask);

    const int tileOffset = 4 * (processRect.y() * rowStride + processRect.x());

    KoCompositeOp::ParameterInfo params;
    params.dstRowStride  = 4 * rowStride;
    params.srcRowStride  = 4 * rowStride;
    params.maskRowStride = rowStride;
    params.rows          = processRect.height();
    params.cols          = processRect.width();
    params.opacity       = opacity;
    params.flow          = flow;
    params.channelFlags  = QBitArray();

    QTime timer;
    timer.start();

    foreach (const Tile &tile, tiles) {
        params.dstRowStart   = tile.dst + tileOffset;
        params.srcRowStart   = tile.src + tileOffset;
        params.maskRowStart  = haveMask ? tile.mask : 0;
        op->composite(params);
    }

    qDebug() << testName << "RESULT:" << timer.elapsed() << "msec";

//    qDebug() << "Final values:";
//    printData(8, tiles[0].src, tiles[0].dst, tiles[0].mask);

    freeTiles(tiles, srcAlignmentShift, dstAlignmentShift);
}

void benchmarkCompositeOp(const KoCompositeOp *op, const QString &postfix)
{
    qDebug() << "Testing Composite Op:" << op->id() << "(" << postfix << ")";

    benchmarkCompositeOp(op, true, 0.5, 0.3, 0, 0, ALPHA_RANDOM, ALPHA_RANDOM);
    benchmarkCompositeOp(op, true, 0.5, 0.3, 8, 0, ALPHA_RANDOM, ALPHA_RANDOM);
    benchmarkCompositeOp(op, true, 0.5, 0.3, 0, 8, ALPHA_RANDOM, ALPHA_RANDOM);
    benchmarkCompositeOp(op, true, 0.5, 0.3, 4, 8, ALPHA_RANDOM, ALPHA_RANDOM);

/// --- Vary the content of the source and destination

    benchmarkCompositeOp(op, false, 1.0, 1.0, 0, 0, ALPHA_RANDOM, ALPHA_RANDOM);
    benchmarkCompositeOp(op, false, 1.0, 1.0, 0, 0, ALPHA_ZERO, ALPHA_RANDOM);
    benchmarkCompositeOp(op, false, 1.0, 1.0, 0, 0, ALPHA_UNIT, ALPHA_RANDOM);

/// ---

    benchmarkCompositeOp(op, false, 1.0, 1.0, 0, 0, ALPHA_RANDOM, ALPHA_ZERO);
    benchmarkCompositeOp(op, false, 1.0, 1.0, 0, 0, ALPHA_ZERO, ALPHA_ZERO);
    benchmarkCompositeOp(op, false, 1.0, 1.0, 0, 0, ALPHA_UNIT, ALPHA_ZERO);

/// ---

    benchmarkCompositeOp(op, false, 1.0, 1.0, 0, 0, ALPHA_RANDOM, ALPHA_UNIT);
    benchmarkCompositeOp(op, false, 1.0, 1.0, 0, 0, ALPHA_ZERO, ALPHA_UNIT);
    benchmarkCompositeOp(op, false, 1.0, 1.0, 0, 0, ALPHA_UNIT, ALPHA_UNIT);
}

#ifdef HAVE_VC

template<class Compositor>
void checkRounding()
{
    QVector<Tile> tiles =
        generateTiles(2, 0, 0, ALPHA_RANDOM, ALPHA_RANDOM);

    const int vecSize = Vc::float_v::Size;

    const int numBlocks = numPixels / vecSize;

    quint8 *src1 = tiles[0].src;
    quint8 *dst1 = tiles[0].dst;
    quint8 *msk1 = tiles[0].mask;

    quint8 *src2 = tiles[1].src;
    quint8 *dst2 = tiles[1].dst;
    quint8 *msk2 = tiles[1].mask;

    for (int i = 0; i < numBlocks; i++) {
        Compositor::template compositeVector<true,true, VC_IMPL>(src1, dst1, msk1, 0.5, 0.3);
        for (int j = 0; j < vecSize; j++) {

            Compositor::template compositeOnePixelScalar<true, VC_IMPL>(src2, dst2, msk2, 0.5, 0.3, QBitArray());

            if(!comparePixels(dst1, dst2, 0)) {
                qDebug() << "Wrong rounding in pixel:" << 8 * i + j;
                qDebug() << "Vector version: " << dst1[0] << dst1[1] << dst1[2] << dst1[3];
                qDebug() << "Scalar version: " << dst2[0] << dst2[1] << dst2[2] << dst2[3];

                qDebug() << "src:" << src1[0] << src1[1] << src1[2] << src1[3];
                qDebug() << "msk:" << msk1[0];

                QFAIL("Wrong rounding");
            }

            src1 += 4;
            dst1 += 4;
            src2 += 4;
            dst2 += 4;
            msk1++;
            msk2++;
        }
    }

    freeTiles(tiles, 0, 0);
}

#endif


void KisCompositionBenchmark::checkRoundingAlphaDarken()
{
#ifdef HAVE_VC
    checkRounding<AlphaDarkenCompositor32<quint8, quint32> >();
#endif
}

void KisCompositionBenchmark::checkRoundingOver()
{
#ifdef HAVE_VC
    checkRounding<OverCompositor32<quint8, quint32, false, true> >();
#endif
}

void KisCompositionBenchmark::compareAlphaDarkenOps()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoCompositeOp *opAct = KoOptimizedCompositeOpFactory::createAlphaDarkenOp32(cs);
    KoCompositeOp *opExp = new KoCompositeOpAlphaDarken<KoBgrU8Traits>(cs);

    QVERIFY(compareTwoOps(true, opAct, opExp));

    delete opExp;
    delete opAct;
}

void KisCompositionBenchmark::compareAlphaDarkenOpsNoMask()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoCompositeOp *opAct = KoOptimizedCompositeOpFactory::createAlphaDarkenOp32(cs);
    KoCompositeOp *opExp = new KoCompositeOpAlphaDarken<KoBgrU8Traits>(cs);

    QVERIFY(compareTwoOps(false, opAct, opExp));

    delete opExp;
    delete opAct;
}

void KisCompositionBenchmark::compareOverOps()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoCompositeOp *opAct = KoOptimizedCompositeOpFactory::createOverOp32(cs);
    KoCompositeOp *opExp = new KoCompositeOpOver<KoBgrU8Traits>(cs);

    QVERIFY(compareTwoOps(true, opAct, opExp));

    delete opExp;
    delete opAct;
}

void KisCompositionBenchmark::compareOverOpsNoMask()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoCompositeOp *opAct = KoOptimizedCompositeOpFactory::createOverOp32(cs);
    KoCompositeOp *opExp = new KoCompositeOpOver<KoBgrU8Traits>(cs);

    QVERIFY(compareTwoOps(false, opAct, opExp));

    delete opExp;
    delete opAct;
}

void KisCompositionBenchmark::testRgb8CompositeAlphaDarkenLegacy()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoCompositeOp *op = new KoCompositeOpAlphaDarken<KoBgrU8Traits>(cs);
    benchmarkCompositeOp(op, "Legacy");
    delete op;
}

void KisCompositionBenchmark::testRgb8CompositeAlphaDarkenOptimized()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoCompositeOp *op = KoOptimizedCompositeOpFactory::createAlphaDarkenOp32(cs);
    benchmarkCompositeOp(op, "Optimized");
    delete op;
}

void KisCompositionBenchmark::testRgb8CompositeOverLegacy()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoCompositeOp *op = new KoCompositeOpOver<KoBgrU8Traits>(cs);
    benchmarkCompositeOp(op, "Legacy");
    delete op;
}

void KisCompositionBenchmark::testRgb8CompositeOverOptimized()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoCompositeOp *op = KoOptimizedCompositeOpFactory::createOverOp32(cs);
    benchmarkCompositeOp(op, "Optimized");
    delete op;
}

void KisCompositionBenchmark::testRgb8CompositeAlphaDarkenReal_Aligned()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    const KoCompositeOp *op = cs->compositeOp(COMPOSITE_ALPHA_DARKEN);
    benchmarkCompositeOp(op, true, 0.5, 0.3, 0, 0, ALPHA_RANDOM, ALPHA_RANDOM);
}

void KisCompositionBenchmark::testRgb8CompositeOverReal_Aligned()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    const KoCompositeOp *op = cs->compositeOp(COMPOSITE_OVER);
    benchmarkCompositeOp(op, true, 0.5, 0.3, 0, 0, ALPHA_RANDOM, ALPHA_RANDOM);
}

void KisCompositionBenchmark::benchmarkMemcpy()
{
    QVector<Tile> tiles =
        generateTiles(numTiles, 0, 0, ALPHA_UNIT, ALPHA_UNIT);

    QBENCHMARK_ONCE {
        foreach (const Tile &tile, tiles) {
            memcpy(tile.dst, tile.src, 4 * numPixels);
        }
    }

    freeTiles(tiles, 0, 0);
}

void KisCompositionBenchmark::benchmarkUintFloat()
{
#ifdef HAVE_VC
    const int vecSize = Vc::float_v::Size;

    const int dataSize = 4096;
    quint8 *iData = (quint8*) memalign(vecSize, dataSize);
    float *fData = (float*) memalign(vecSize * 4, dataSize * 4);

    QBENCHMARK {
        for (int i = 0; i < dataSize; i += Vc::float_v::Size) {
            // convert uint -> float directly, this causes
            // static_cast helper be called
            Vc::float_v b(Vc::uint_v(iData + i));
            b.store(fData + i);
        }
    }

    free(iData);
    free(fData);
#endif
}

void KisCompositionBenchmark::benchmarkUintIntFloat()
{
#ifdef HAVE_VC
    const int vecSize = Vc::float_v::Size;

    const int dataSize = 4096;
    quint8 *iData = (quint8*) memalign(vecSize, dataSize);
    float *fData = (float*) memalign(vecSize * 4, dataSize * 4);

    QBENCHMARK {
        for (int i = 0; i < dataSize; i += Vc::float_v::Size) {
            // convert uint->int->float, that avoids special sign
            // treating, and gives 2.6 times speedup
            Vc::float_v b(Vc::int_v(Vc::uint_v(iData + i)));
            b.store(fData + i);
        }
    }

    free(iData);
    free(fData);
#endif
}

void KisCompositionBenchmark::benchmarkFloatUint()
{
#ifdef HAVE_VC
    const int vecSize = Vc::float_v::Size;

    const int dataSize = 4096;
    quint32 *iData = (quint32*) memalign(vecSize * 4, dataSize * 4);
    float *fData = (float*) memalign(vecSize * 4, dataSize * 4);

    QBENCHMARK {
        for (int i = 0; i < dataSize; i += Vc::float_v::Size) {
            // conversion float -> uint
            Vc::uint_v b(Vc::float_v(fData + i));

            b.store(iData + i);
        }
    }

    free(iData);
    free(fData);
#endif
}

void KisCompositionBenchmark::benchmarkFloatIntUint()
{
#ifdef HAVE_VC
    const int vecSize = Vc::float_v::Size;

    const int dataSize = 4096;
    quint32 *iData = (quint32*) memalign(vecSize * 4, dataSize * 4);
    float *fData = (float*) memalign(vecSize * 4, dataSize * 4);

    QBENCHMARK {
        for (int i = 0; i < dataSize; i += Vc::float_v::Size) {
            // conversion float -> int -> uint
            Vc::uint_v b(Vc::int_v(Vc::float_v(fData + i)));

            b.store(iData + i);
        }
    }

    free(iData);
    free(fData);
#endif
}

QTEST_KDEMAIN(KisCompositionBenchmark, GUI)

