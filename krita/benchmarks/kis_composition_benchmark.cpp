/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2015 Thorsten Zachmann <zachmann@kde.org>
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
#include <KoOptimizedCompositeOpOver128.h>
#include <KoOptimizedCompositeOpAlphaDarken32.h>
#endif

// for posix_memalign()
#include <stdlib.h>

#include <kis_debug.h>

const int alpha_pos = 3;

enum AlphaRange {
    ALPHA_ZERO,
    ALPHA_UNIT,
    ALPHA_RANDOM
};


template <typename channel_type, class RandomGenerator>
inline channel_type generateAlphaValue(AlphaRange range, RandomGenerator &rnd) {
    channel_type value = 0;

    switch (range) {
    case ALPHA_ZERO:
        break;
    case ALPHA_UNIT:
        value = rnd.unit();
        break;
    case ALPHA_RANDOM:
        value = rnd();
        break;
    }

    return value;
}

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/uniform_real.hpp>

template <typename channel_type>
struct RandomGenerator {
    channel_type operator() () {
        qFatal("Wrong template instantiation");
        return channel_type(0);
    }

    channel_type unit() {
        qFatal("Wrong template instantiation");
        return channel_type(0);
    }
};

template <>
struct RandomGenerator<quint8>
{
    RandomGenerator(int seed)
        : m_smallint(0,255),
          m_rnd(seed)
    {
    }

    quint8 operator() () {
        return m_smallint(m_rnd);
    }

    quint8 unit() {
        return KoColorSpaceMathsTraits<quint8>::unitValue;
    }

    boost::uniform_smallint<int> m_smallint;
    boost::mt11213b m_rnd;
};

template <>
struct RandomGenerator<float>
{
    RandomGenerator(int seed)
        : m_rnd(seed)
    {
    }

    float operator() () {
        //return float(m_rnd()) / float(m_rnd.max());
        return m_smallfloat(m_rnd);
    }

    float unit() {
        return KoColorSpaceMathsTraits<float>::unitValue;
    }

    boost::uniform_real<float> m_smallfloat;
    boost::mt11213b m_rnd;
};

template <>
struct RandomGenerator<double> : RandomGenerator<float>
{
    RandomGenerator(int seed)
        : RandomGenerator<float>(seed)
    {
    }
};


template <typename channel_type>
void generateDataLine(uint seed, int numPixels, quint8 *srcPixels, quint8 *dstPixels, quint8 *mask, AlphaRange srcAlphaRange, AlphaRange dstAlphaRange)
{
    Q_ASSERT(numPixels >= 4);

    RandomGenerator<channel_type> rnd(seed);
    RandomGenerator<quint8> maskRnd(seed + 1);

    channel_type *srcArray = reinterpret_cast<channel_type*>(srcPixels);
    channel_type *dstArray = reinterpret_cast<channel_type*>(dstPixels);

    for (int i = 0; i < numPixels; i++) {
        for (int j = 0; j < 3; j++) {
            channel_type s = rnd();
            channel_type d = rnd();
            *(srcArray++) = s;
            *(dstArray++) = d;
        }

        channel_type sa = generateAlphaValue<channel_type>(srcAlphaRange, rnd);
        channel_type da = generateAlphaValue<channel_type>(dstAlphaRange, rnd);
        *(srcArray++) = sa;
        *(dstArray++) = da;

        *(mask++) = maskRnd();
    }
}

void printData(int numPixels, quint8 *srcPixels, quint8 *dstPixels, quint8 *mask)
{
    for (int i = 0; i < numPixels; i++) {
        dbgKrita << "Src: "
                 << srcPixels[i*4] << "\t"
                 << srcPixels[i*4+1] << "\t"
                 << srcPixels[i*4+2] << "\t"
                 << srcPixels[i*4+3] << "\t"
                 << "Msk:" << mask[i];

        dbgKrita << "Dst: "
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
                            AlphaRange dstAlphaRange,
                            const quint32 pixelSize)
{
    QVector<Tile> tiles(size);

#ifdef HAVE_VC
    const int vecSize = Vc::float_v::Size;
#else
    const int vecSize = 1;
#endif

    // the 256 are used to make sure that we have a good alignment no matter what build options are used.
    const size_t pixelAlignment = qMax(size_t(vecSize * sizeof(float)), 256ul);
    const size_t maskAlignment = qMax(size_t(vecSize), 256ul);
    for (int i = 0; i < size; i++) {
        void *ptr = NULL;
        int error = posix_memalign(&ptr, pixelAlignment, numPixels * pixelSize + srcAlignmentShift);
        if (error) {
            qFatal("posix_memalign failed: %d", error);
        }
        tiles[i].src = (quint8*)ptr + srcAlignmentShift;
        error = posix_memalign(&ptr, pixelAlignment, numPixels * pixelSize + dstAlignmentShift);
        if (error) {
            qFatal("posix_memalign failed: %d", error);
        }
        tiles[i].dst = (quint8*)ptr + dstAlignmentShift;
        error = posix_memalign(&ptr, maskAlignment, numPixels);
        if (error) {
            qFatal("posix_memalign failed: %d", error);
        }
        tiles[i].mask = (quint8*)ptr;

        if (pixelSize == 4) {
            generateDataLine<quint8>(1, numPixels, tiles[i].src, tiles[i].dst, tiles[i].mask, srcAlphaRange, dstAlphaRange);
        } else if (pixelSize == 16) {
            generateDataLine<float>(1, numPixels, tiles[i].src, tiles[i].dst, tiles[i].mask, srcAlphaRange, dstAlphaRange);
        } else {
            qFatal("Pixel size %i is not implemented", pixelSize);
        }
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

template <typename channel_type>
inline bool fuzzyCompare(channel_type a, channel_type b, channel_type prec) {
    return qAbs(a - b) <= prec;
}

template <typename channel_type>
inline bool comparePixels(channel_type *p1, channel_type *p2, channel_type prec) {
    return (p1[3] == p2[3] && p1[3] == 0) ||
        (fuzzyCompare(p1[0], p2[0], prec) &&
         fuzzyCompare(p1[1], p2[1], prec) &&
         fuzzyCompare(p1[2], p2[2], prec) &&
         fuzzyCompare(p1[3], p2[3], prec));
}

template <typename channel_type>
bool compareTwoOpsPixels(QVector<Tile> &tiles, channel_type prec) {
    channel_type *dst1 = reinterpret_cast<channel_type*>(tiles[0].dst);
    channel_type *dst2 = reinterpret_cast<channel_type*>(tiles[1].dst);

    channel_type *src1 = reinterpret_cast<channel_type*>(tiles[0].src);
    channel_type *src2 = reinterpret_cast<channel_type*>(tiles[1].src);

    for (int i = 0; i < numPixels; i++) {
        if (!comparePixels<channel_type>(dst1, dst2, prec)) {
            dbgKrita << "Wrong result:" << i;
            dbgKrita << "Act: " << dst1[0] << dst1[1] << dst1[2] << dst1[3];
            dbgKrita << "Exp: " << dst2[0] << dst2[1] << dst2[2] << dst2[3];
            dbgKrita << "Dif: " << dst1[0] - dst2[0] << dst1[1] - dst2[1] << dst1[2] - dst2[2] << dst1[3] - dst2[3];

            channel_type *s1 = src1 + 4 * i;
            channel_type *s2 = src2 + 4 * i;

            dbgKrita << "SrcA:" << s1[0] << s1[1] << s1[2] << s1[3];
            dbgKrita << "SrcE:" << s2[0] << s2[1] << s2[2] << s2[3];

            dbgKrita << "MskA:" << tiles[0].mask[i];
            dbgKrita << "MskE:" << tiles[1].mask[i];

            return false;
        }
        dst1 += 4;
        dst2 += 4;
    }
    return true;
}

bool compareTwoOps(bool haveMask, const KoCompositeOp *op1, const KoCompositeOp *op2)
{
    Q_ASSERT(op1->colorSpace()->pixelSize() == op2->colorSpace()->pixelSize());
    const quint32 pixelSize = op1->colorSpace()->pixelSize();
    const int alignment = 16;
    QVector<Tile> tiles = generateTiles(2, alignment, alignment, ALPHA_RANDOM, ALPHA_RANDOM, op1->colorSpace()->pixelSize());

    KoCompositeOp::ParameterInfo params;
    params.dstRowStride  = 4 * rowStride;
    params.srcRowStride  = 4 * rowStride;
    params.maskRowStride = rowStride;
    params.rows          = processRect.height();
    params.cols          = processRect.width();
    // This is a hack as in the old version we get a rounding of opacity to this value
    params.opacity       = float(Arithmetic::scale<quint8>(0.5*1.0f))/255.0;
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

    bool compareResult = true;
    if (pixelSize == 4) {
        compareResult = compareTwoOpsPixels<quint8>(tiles, 10);
    }
    else if (pixelSize == 16) {
        compareResult = compareTwoOpsPixels<float>(tiles, 0);
    }
    else {
        qFatal("Pixel size %i is not implemented", pixelSize);
    }

    freeTiles(tiles, alignment, alignment);

    return compareResult;
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
        generateTiles(numTiles, srcAlignmentShift, dstAlignmentShift, srcAlphaRange, dstAlphaRange, op->colorSpace()->pixelSize());

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

    dbgKrita << testName << "RESULT:" << timer.elapsed() << "msec";

    freeTiles(tiles, srcAlignmentShift, dstAlignmentShift);
}

void benchmarkCompositeOp(const KoCompositeOp *op, const QString &postfix)
{
    dbgKrita << "Testing Composite Op:" << op->id() << "(" << postfix << ")";

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
void checkRounding(qreal opacity, qreal flow, qreal averageOpacity = -1, quint32 pixelSize = 4)
{
    QVector<Tile> tiles =
        generateTiles(2, 0, 0, ALPHA_RANDOM, ALPHA_RANDOM, pixelSize);

    const int vecSize = Vc::float_v::Size;

    const int numBlocks = numPixels / vecSize;

    quint8 *src1 = tiles[0].src;
    quint8 *dst1 = tiles[0].dst;
    quint8 *msk1 = tiles[0].mask;

    quint8 *src2 = tiles[1].src;
    quint8 *dst2 = tiles[1].dst;
    quint8 *msk2 = tiles[1].mask;

    KoCompositeOp::ParameterInfo params;
    params.opacity = opacity;
    params.flow = flow;

    if (averageOpacity >= 0.0) {
        params._lastOpacityData = averageOpacity;
        params.lastOpacity = &params._lastOpacityData;
    }

    params.channelFlags = QBitArray();
    typename Compositor::OptionalParams optionalParams(params);

    // The error count is needed as 38.5 gets rounded to 38 instead of 39 in the vc version.
    int errorcount = 0;
    for (int i = 0; i < numBlocks; i++) {
        Compositor::template compositeVector<true,true, VC_IMPL>(src1, dst1, msk1, params.opacity, optionalParams);
        for (int j = 0; j < vecSize; j++) {

            //if (8 * i + j == 7080) {
            //    dbgKrita << "src: " << src2[0] << src2[1] << src2[2] << src2[3];
            //    dbgKrita << "dst: " << dst2[0] << dst2[1] << dst2[2] << dst2[3];
            //    dbgKrita << "msk:" << msk2[0];
            //}

            Compositor::template compositeOnePixelScalar<true, VC_IMPL>(src2, dst2, msk2, params.opacity, optionalParams);

            bool compareResult = true;
            if (pixelSize == 4) {
                compareResult = comparePixels<quint8>(dst1, dst2, 0);
                if (!compareResult) {
                    ++errorcount;
                    compareResult = comparePixels<quint8>(dst1, dst2, 1);
                    if (!compareResult) {
                        ++errorcount;
                    }
                }
            }
            else if (pixelSize == 16) {
                compareResult = comparePixels<float>(reinterpret_cast<float*>(dst1), reinterpret_cast<float*>(dst2), 0);
            }
            else {
                qFatal("Pixel size %i is not implemented", pixelSize);
            }

            if(!compareResult || errorcount > 1) {
                dbgKrita << "Wrong rounding in pixel:" << 8 * i + j;
                dbgKrita << "Vector version: " << dst1[0] << dst1[1] << dst1[2] << dst1[3];
                dbgKrita << "Scalar version: " << dst2[0] << dst2[1] << dst2[2] << dst2[3];

                dbgKrita << "src:" << src1[0] << src1[1] << src1[2] << src1[3];
                dbgKrita << "msk:" << msk1[0];

                QFAIL("Wrong rounding");
            }

            src1 += pixelSize;
            dst1 += pixelSize;
            src2 += pixelSize;
            dst2 += pixelSize;
            msk1++;
            msk2++;
        }
    }

    freeTiles(tiles, 0, 0);
}

#endif


void KisCompositionBenchmark::checkRoundingAlphaDarken_05_03()
{
#ifdef HAVE_VC
    checkRounding<AlphaDarkenCompositor32<quint8, quint32> >(0.5,0.3);
#endif
}

void KisCompositionBenchmark::checkRoundingAlphaDarken_05_05()
{
#ifdef HAVE_VC
    checkRounding<AlphaDarkenCompositor32<quint8, quint32> >(0.5,0.5);
#endif
}

void KisCompositionBenchmark::checkRoundingAlphaDarken_05_07()
{
#ifdef HAVE_VC
    checkRounding<AlphaDarkenCompositor32<quint8, quint32> >(0.5,0.7);
#endif
}

void KisCompositionBenchmark::checkRoundingAlphaDarken_05_10()
{
#ifdef HAVE_VC
    checkRounding<AlphaDarkenCompositor32<quint8, quint32> >(0.5,1.0);
#endif
}

void KisCompositionBenchmark::checkRoundingAlphaDarken_05_10_08()
{
#ifdef HAVE_VC
    checkRounding<AlphaDarkenCompositor32<quint8, quint32> >(0.5,1.0,0.8);
#endif
}

void KisCompositionBenchmark::checkRoundingOver()
{
#ifdef HAVE_VC
    checkRounding<OverCompositor32<quint8, quint32, false, true> >(0.5, 0.3);
#endif
}

void KisCompositionBenchmark::checkRoundingOverRgbaF32()
{
#ifdef HAVE_VC
    checkRounding<OverCompositor128<float, float, false, true> >(0.5, 0.3, -1, 16);
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

void KisCompositionBenchmark::compareRgbF32OverOps()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", "F32", "");
    KoCompositeOp *opAct = KoOptimizedCompositeOpFactory::createOverOp128(cs);
    KoCompositeOp *opExp = new KoCompositeOpOver<KoRgbF32Traits>(cs);

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

void KisCompositionBenchmark::testRgbF32CompositeOverLegacy()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", "F32", "");
    KoCompositeOp *op = new KoCompositeOpOver<KoRgbF32Traits>(cs);
    benchmarkCompositeOp(op, "RGBF32 Legacy");
    delete op;
}

void KisCompositionBenchmark::testRgbF32CompositeOverOptimized()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", "F32", "");
    KoCompositeOp *op = KoOptimizedCompositeOpFactory::createOverOp128(cs);
    benchmarkCompositeOp(op, "RGBF32 Optimized");
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

void KisCompositionBenchmark::testRgb8CompositeCopyLegacy()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    const KoCompositeOp *op = cs->compositeOp(COMPOSITE_COPY);
    benchmarkCompositeOp(op, "Copy");
}

void KisCompositionBenchmark::benchmarkMemcpy()
{
    QVector<Tile> tiles =
        generateTiles(numTiles, 0, 0, ALPHA_UNIT, ALPHA_UNIT, 4);

    QBENCHMARK_ONCE {
        foreach (const Tile &tile, tiles) {
            memcpy(tile.dst, tile.src, 4 * numPixels);
        }
    }

    freeTiles(tiles, 0, 0);
}

#ifdef HAVE_VC
    const int vecSize = Vc::float_v::Size;
    const size_t uint8VecAlignment = qMax(vecSize * sizeof(quint8), sizeof(void*));
    const size_t uint32VecAlignment = qMax(vecSize * sizeof(quint32), sizeof(void*));
    const size_t floatVecAlignment = qMax(vecSize * sizeof(float), sizeof(void*));
#endif

void KisCompositionBenchmark::benchmarkUintFloat()
{
#ifdef HAVE_VC
    const int dataSize = 4096;
    void *ptr = NULL;
    int error = posix_memalign(&ptr, uint8VecAlignment, dataSize);
    if (error) {
        qFatal("posix_memalign failed: %d", error);
    }
    quint8 *iData = (quint8*)ptr;
    error = posix_memalign(&ptr, floatVecAlignment, dataSize * sizeof(float));
    if (error) {
        qFatal("posix_memalign failed: %d", error);
    }
    float *fData = (float*)ptr;

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
    const int dataSize = 4096;
    void *ptr = NULL;
    int error = posix_memalign(&ptr, uint8VecAlignment, dataSize);
    if (error) {
        qFatal("posix_memalign failed: %d", error);
    }
    quint8 *iData = (quint8*)ptr;
    error = posix_memalign(&ptr, floatVecAlignment, dataSize * sizeof(float));
    if (error) {
        qFatal("posix_memalign failed: %d", error);
    }
    float *fData = (float*)ptr;

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
    const int dataSize = 4096;
    void *ptr = NULL;
    int error = posix_memalign(&ptr, uint32VecAlignment, dataSize * sizeof(quint32));
    if (error) {
        qFatal("posix_memalign failed: %d", error);
    }
    quint32 *iData = (quint32*)ptr;
    error = posix_memalign(&ptr, floatVecAlignment, dataSize * sizeof(float));
    if (error) {
        qFatal("posix_memalign failed: %d", error);
    }
    float *fData = (float*)ptr;

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
    const int dataSize = 4096;
    void *ptr = NULL;
    int error = posix_memalign(&ptr, uint32VecAlignment, dataSize * sizeof(quint32));
    if (error) {
        qFatal("posix_memalign failed: %d", error);
    }
    quint32 *iData = (quint32*)ptr;
    error = posix_memalign(&ptr, floatVecAlignment, dataSize * sizeof(float));
    if (error) {
        qFatal("posix_memalign failed: %d", error);
    }
    float *fData = (float*)ptr;

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

